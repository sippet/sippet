// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/test/standalone_test_server/standalone_test_server.h"

#include <pjsip.h>
#include <pjlib-util.h>
#include <pjlib.h>

#include <string>

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/stl_util.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/threading/thread_restrictions.h"
#include "net/base/ip_endpoint.h"
#include "net/base/net_errors.h"

#define THIS_FILE "standalone_test_server.cc"

namespace sippet {

namespace {
// There can exist just one Standalone server at a time
StandaloneTestServer *g_server = nullptr;
}

struct StandaloneTestServerCallbacks {
  static pj_bool_t on_rx_request(pjsip_rx_data *rdata) {
    g_server->OnReceiveRequest(rdata);
    return PJ_TRUE;
  }
  static pj_bool_t on_rx_response(pjsip_rx_data *rdata) {
    g_server->OnReceiveResponse(rdata);
    return PJ_TRUE;
  }
  static pj_bool_t logging_on_rx_msg(pjsip_rx_data *rdata) {
    PJ_LOG(4, (THIS_FILE, "RX %d bytes %s from %s %s:%d:\n"
        "%.*s\n"
        "--end msg--",
        rdata->msg_info.len,
        pjsip_rx_data_get_info(rdata),
        rdata->tp_info.transport->type_name,
        rdata->pkt_info.src_name,
        rdata->pkt_info.src_port,
        static_cast<int>(rdata->msg_info.len),
        rdata->msg_info.msg_buf));
    return PJ_FALSE;
  }
  static pj_status_t logging_on_tx_msg(pjsip_tx_data *tdata) {
    PJ_LOG(4, (THIS_FILE, "TX %d bytes %s to %s %s:%d:\n"
        "%.*s\n"
        "--end msg--",
        (tdata->buf.cur - tdata->buf.start),
        pjsip_tx_data_get_info(tdata),
        tdata->tp_info.transport->type_name,
        tdata->tp_info.dst_name,
        tdata->tp_info.dst_port,
        static_cast<int>(tdata->buf.cur - tdata->buf.start),
        tdata->buf.start));
    return PJ_SUCCESS;
  }
};

namespace {

pjsip_module mod_app = {
    nullptr, nullptr,
    { "mod-embed-srv", 13 },
    -1,
    PJSIP_MOD_PRIORITY_APPLICATION,
    nullptr,  // load()
    nullptr,  // start()
    nullptr,  // stop()
    nullptr,  // unload()
    &StandaloneTestServerCallbacks::on_rx_request,   // on_rx_request()
    &StandaloneTestServerCallbacks::on_rx_response,  // on_rx_response()
    nullptr,  // on_tx_request()
    nullptr,  // on_tx_response()
    nullptr,  // on_tsx_state()
};

pjsip_module msg_logger = {
    nullptr, nullptr,
    { "mod-msg-log", 11 },
    -1,
    PJSIP_MOD_PRIORITY_TRANSPORT_LAYER-1,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    &StandaloneTestServerCallbacks::logging_on_rx_msg,
    &StandaloneTestServerCallbacks::logging_on_rx_msg,
    &StandaloneTestServerCallbacks::logging_on_tx_msg,
    &StandaloneTestServerCallbacks::logging_on_tx_msg,
    nullptr,
};

pj_status_t LookupCredentials(pj_pool_t *pool,
    const pj_str_t *realm, const pj_str_t *acc_name,
    pjsip_cred_info *cred_info) {
  if (pj_strcmp2(realm, "no-biloxi.com") == 0
      || pj_strcmp2(realm, "test") == 0
      || pj_strcmp2(&cred_info->scheme, "digest") == 0) {
    cred_info->realm = pj_str("no-biloxi.com");
    cred_info->username = pj_str("test");
    cred_info->data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
    cred_info->data = pj_str("1234");
    return PJ_SUCCESS;
  }
  return PJ_ENOTFOUND;
}

}  // namespace

struct StandaloneTestServer::ControlStruct {
  pj_caching_pool caching_pool_;
  pjsip_endpoint* endpoint_;
  pj_pool_t* pool_;
  pj_thread_t* thread_;
  pjsip_auth_srv auth_srv_;

  ControlStruct()
    : endpoint_(nullptr),
      pool_(nullptr),
      thread_(nullptr) {
    memset(&caching_pool_, 0, sizeof(caching_pool_));
    memset(&auth_srv_, 0, sizeof(auth_srv_));
  }

  ~ControlStruct() {
    if (thread_)
      StopWorkerThread();
    if (endpoint_)
      pjsip_endpt_destroy(endpoint_);
    if (pool_)
      pj_pool_release(pool_);
    if (caching_pool_.max_capacity != 0)
      pj_caching_pool_destroy(&caching_pool_);
  }

  bool Init() {
    pj_status_t status;

    status = pj_init();
    if (status != PJ_SUCCESS)
      return false;

    pj_log_set_level(PJ_LOG_MAX_LEVEL);

    status = pjlib_util_init();
    if (status != PJ_SUCCESS)
      return false;

    pj_caching_pool_init(&caching_pool_,
      &pj_pool_factory_default_policy, 0);

    status = pjsip_endpt_create(&caching_pool_.factory, nullptr, &endpoint_);
    if (status != PJ_SUCCESS)
      return false;

    status = pjsip_tsx_layer_init_module(endpoint_);
    if (status != PJ_SUCCESS)
      return false;

    pjsip_endpt_register_module(endpoint_, &msg_logger);

    pool_ = pj_pool_create(&caching_pool_.factory, "embedsrv",
        4000, 4000, nullptr);
    return true;
  }

  bool ListenTo(const Protocol& protocol, int* port,
                const StandaloneTestServer::SSLOptions& options) {
    pj_status_t status;

    pj_sockaddr_in addr;
    pj_str_t localhost = pj_str("127.0.0.1");
    addr.sin_family = pj_AF_INET();
    pj_inet_pton(PJ_AF_INET, &localhost, &addr.sin_addr.s_addr);
    addr.sin_port = pj_htons(static_cast<pj_uint16_t>(*port));

    if (Protocol::UDP == protocol) {
      pjsip_transport *tp = nullptr;
      status = pjsip_udp_transport_start(endpoint_, &addr, nullptr, 1, &tp);
      if (status == PJ_SUCCESS)
        *port = tp->local_name.port;
    } else {
      pjsip_tpfactory *tf = nullptr;
      if (Protocol::TCP == protocol) {
        status = pjsip_tcp_transport_start(endpoint_, &addr, 1, &tf);
      } else if (Protocol::TLS == protocol) {
        pjsip_tls_setting opt;
        pjsip_tls_setting_default(&opt);
        opt.method = PJSIP_TLSV1_METHOD;
        opt.require_client_cert =
          options.request_client_certificate ? PJ_TRUE : PJ_FALSE;
        std::string certificate_file(options.certificate_file.AsUTF8Unsafe());
        if (!certificate_file.empty())
          opt.cert_file = pj_str(const_cast<char*>(certificate_file.c_str()));
        std::string privatekey_file(options.privatekey_file.AsUTF8Unsafe());
        if (!privatekey_file.empty())
          opt.privkey_file = pj_str(const_cast<char*>(privatekey_file.c_str()));
        if (!options.password.empty())
          opt.password = pj_str(const_cast<char*>(options.password.c_str()));
        status = pjsip_tls_transport_start(endpoint_, &opt, &addr,
          nullptr, 1, &tf);
      } else {
        NOTREACHED() << "Unknown protocol";
        return false;
      }
      if (status == PJ_SUCCESS)
        *port = tf->addr_name.port;
    }

    return status == PJ_SUCCESS;
  }

  bool RegisterModule() {
    pj_status_t status;
    status = pjsip_endpt_register_module(endpoint_, &mod_app);
    return status == PJ_SUCCESS;
  }

  bool StartAuthentication() {
    pj_status_t status;
    pj_str_t realm = pj_str("no-biloxi.com");
    status = pjsip_auth_srv_init(pool_, &auth_srv_,
      &realm, &LookupCredentials, 0);
    return status == PJ_SUCCESS;
  }

  bool StartWorkerThread() {
    pj_status_t status;
    quit_flag_ = false;
    status = pj_thread_create(pool_, "embedsrv", &WorkerThread,
        this, 0, 0, &thread_);
    return status == PJ_SUCCESS;
  }

  void StopWorkerThread() {
    quit_flag_ = true;
    pj_thread_join(thread_);
    thread_ = nullptr;
  }

  static bool quit_flag_;

  static int WorkerThread(void *p) {
    pj_time_val delay = {0, 10};
    ControlStruct *self = reinterpret_cast<ControlStruct *>(p);
    while (!quit_flag_)
      pjsip_endpt_handle_events(self->endpoint_, &delay);
    return 0;
  }
};

bool StandaloneTestServer::ControlStruct::quit_flag_;

StandaloneTestServer::SSLOptions::SSLOptions()
    : request_client_certificate(false) {
}

StandaloneTestServer::SSLOptions::~SSLOptions() {
}

StandaloneTestServer::StandaloneTestServer(const Protocol &protocol,
    int port) {
  Init(protocol, port, nullptr);
}

StandaloneTestServer::StandaloneTestServer(const Protocol &protocol,
    const SSLOptions &ssl_options, int port) {
  Init(protocol, port, &ssl_options);
}

StandaloneTestServer::~StandaloneTestServer() {
  DCHECK(thread_checker_.CalledOnValidThread());

  if (Started() && !ShutdownAndWaitUntilComplete()) {
    LOG(ERROR) << "StandaloneTestServer failed to shut down.";
  }

  g_server = nullptr;
}

void StandaloneTestServer::Init(const Protocol &protocol, int port,
    const SSLOptions *ssl_options) {
  DCHECK_GE(port, 0);
  DCHECK(!g_server);
  DCHECK(thread_checker_.CalledOnValidThread());
  port_ = port;
  protocol_ = protocol;
  if (ssl_options)
    ssl_options_ = *ssl_options;
  g_server = this;
}

bool StandaloneTestServer::InitializeAndWaitUntilReady() {
  base::Thread::Options thread_options;
  thread_options.message_loop_type = base::MessageLoop::TYPE_IO;
  io_thread_.reset(new base::Thread("StandaloneTestServer io thread"));
  CHECK(io_thread_->StartWithOptions(thread_options));

  DCHECK(thread_checker_.CalledOnValidThread());

  bool result = PostTaskToIOThreadAndWait(base::Bind(
      &StandaloneTestServer::InitializeOnIOThread,
          base::Unretained(this)));
  if (!result)
    return false;

  return Started();
}

bool StandaloneTestServer::ShutdownAndWaitUntilComplete() {
  DCHECK(thread_checker_.CalledOnValidThread());

  return PostTaskToIOThreadAndWait(base::Bind(
      &StandaloneTestServer::ShutdownOnIOThread, base::Unretained(this)));
}

bool StandaloneTestServer::Started() const {
  return control_struct_.get() != nullptr;
}

void StandaloneTestServer::OnReceiveRequest(pjsip_rx_data *rdata) {
  if (rdata->msg_info.msg->line.req.method.id == PJSIP_CANCEL_METHOD)
    return;

  if (!VerifyRequest(rdata))
    return;

  if (rdata->msg_info.msg->line.req.method.id == PJSIP_REGISTER_METHOD) {
    // Authentication was OK, so just return success
    pj_status_t status;
    pjsip_tx_data *tdata = nullptr;
    do {
      status = pjsip_endpt_create_response(control_struct_->endpoint_,
          rdata, PJSIP_SC_OK, nullptr, &tdata);
      if (status != PJ_SUCCESS)
        break;
      pjsip_transaction *uas_tsx;
      status = pjsip_tsx_create_uas(nullptr, rdata, &uas_tsx);
      if (status != PJ_SUCCESS)
        break;
      pjsip_tsx_recv_msg(uas_tsx, rdata);
      pjsip_tsx_send_msg(uas_tsx, tdata);
      return;
    } while (0);
    if (tdata)
      pjsip_tx_data_dec_ref(tdata);
    pjsip_endpt_respond_stateless(control_struct_->endpoint_, rdata,
        PJSIP_SC_INTERNAL_SERVER_ERROR, nullptr, nullptr, nullptr);
  }
}

bool StandaloneTestServer::VerifyRequest(pjsip_rx_data *rdata) {
  pj_status_t status;
  const pj_str_t STR_REQUIRE = {"Require", 7};

  // A valid message must pass the following checks:
  // 1. Reasonable Syntax
  // 2. URI scheme
  // 3. Require
  // 4. Authorization

  // 1. Reasonable Syntax.
  // This would have been checked by transport layer.

  // 2. URI scheme.
  // We only want to support "sip:"/"sips:" URI scheme for this simple server.
  if (!PJSIP_URI_SCHEME_IS_SIP(rdata->msg_info.msg->line.req.uri) &&
      !PJSIP_URI_SCHEME_IS_SIPS(rdata->msg_info.msg->line.req.uri)) {
    pjsip_endpt_respond_stateless(control_struct_->endpoint_, rdata,
        PJSIP_SC_UNSUPPORTED_URI_SCHEME, nullptr, nullptr, nullptr);
    return false;
  }

  // 3. Require
  if (pjsip_msg_find_hdr_by_name(rdata->msg_info.msg, &STR_REQUIRE,
      nullptr) != nullptr) {
    pjsip_endpt_respond_stateless(control_struct_->endpoint_, rdata,
        PJSIP_SC_BAD_EXTENSION, nullptr, nullptr, nullptr);
    return false;
  }

  // 4. Authorization
  int status_code;
  status = pjsip_auth_srv_verify(&control_struct_->auth_srv_, rdata,
      &status_code);
  if (status == PJSIP_EAUTHNOAUTH) {
    pjsip_tx_data *tdata = nullptr;
    do {
      status = pjsip_endpt_create_response(control_struct_->endpoint_,
          rdata, status_code, nullptr, &tdata);
      if (status != PJ_SUCCESS)
        break;
      status = pjsip_auth_srv_challenge(&control_struct_->auth_srv_,
          nullptr, nullptr, nullptr, PJ_FALSE, tdata);
      if (status != PJ_SUCCESS)
        break;
      pjsip_transaction *uas_tsx;
      status = pjsip_tsx_create_uas(nullptr, rdata, &uas_tsx);
      if (status != PJ_SUCCESS)
        break;
      pjsip_tsx_recv_msg(uas_tsx, rdata);
      pjsip_tsx_send_msg(uas_tsx, tdata);
      return false;
    } while (0);
    if (tdata)
      pjsip_tx_data_dec_ref(tdata);
    pjsip_endpt_respond_stateless(control_struct_->endpoint_, rdata,
        PJSIP_SC_INTERNAL_SERVER_ERROR, nullptr, nullptr, nullptr);
    return false;
  } else if (status != PJ_SUCCESS) {
    pjsip_endpt_respond_stateless(control_struct_->endpoint_, rdata,
        PJSIP_SC_FORBIDDEN, nullptr, nullptr, nullptr);
    return false;
  }

  return true;
}

void StandaloneTestServer::OnReceiveResponse(pjsip_rx_data *rdata) {
}

void StandaloneTestServer::InitializeOnIOThread() {
  DCHECK(io_thread_->task_runner()->BelongsToCurrentThread());
  DCHECK(!Started());

  control_struct_.reset(new ControlStruct);
  if (!control_struct_->Init()
      || !control_struct_->ListenTo(protocol_, &port_, ssl_options_)
      || !control_struct_->RegisterModule()
      || !control_struct_->StartAuthentication()
      || !control_struct_->StartWorkerThread()) {
    ShutdownOnIOThread();
  } else {
    std::ostringstream uri;
    if (Protocol::UDP == protocol_) {
      uri << "sip:127.0.0.1:" << port_;
    } else if (Protocol::TCP == protocol_) {
      uri << "sip:127.0.0.1:" << port_ << ";transport=TCP";
    } else if (Protocol::TLS == protocol_) {
      uri << "sips:127.0.0.1:" << port_;
    } else {
      NOTREACHED() << "Unknown protocol";
    }
    base_uri_ = GURL(uri.str());
  }
}

void StandaloneTestServer::ShutdownOnIOThread() {
  DCHECK(io_thread_->task_runner()->BelongsToCurrentThread());

  control_struct_.reset(nullptr);
  pj_shutdown();
}

bool StandaloneTestServer::PostTaskToIOThreadAndWait(
    const base::Closure& closure) {
  // Note that PostTaskAndReply below requires
  // base::MessageLoopProxy::current() to return a loop for posting the reply
  // task. However, in order to make StandaloneTestServer universally usable,
  // it needs to cope with the situation where it's running on a thread on
  // which a message loop is not (yet) available or as has been destroyed
  // already.
  //
  // To handle this situation, create temporary message loop to support the
  // PostTaskAndReply operation if the current thread as no message loop.
  scoped_ptr<base::MessageLoop> temporary_loop;
  if (!base::MessageLoop::current())
    temporary_loop.reset(new base::MessageLoop());

  base::RunLoop run_loop;
  if (!io_thread_->task_runner()->PostTaskAndReply(
          FROM_HERE, closure, run_loop.QuitClosure())) {
    return false;
  }
  run_loop.Run();

  return true;
}

}  // namespace sippet
