// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/test/embedded_test_server/embedded_test_server.h"

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/stl_util.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/threading/thread_restrictions.h"
#include "net/base/ip_endpoint.h"
#include "net/base/net_errors.h"

#include <pjsip.h>
#include <pjlib-util.h>
#include <pjlib.h>

namespace sippet {

namespace {
// There can exist just one Embedded server at a time
EmbeddedTestServer *g_server = NULL;
}

struct EmbeddedTestServerCallbacks {
  static pj_bool_t on_rx_request(pjsip_rx_data *rdata) {
    g_server->OnReceiveRequest(rdata);
    return PJ_TRUE;
  }
  static pj_bool_t on_rx_response(pjsip_rx_data *rdata) {
    g_server->OnReceiveResponse(rdata);
    return PJ_TRUE;
  }
};

namespace {

pjsip_module mod_app =
{
    NULL, NULL,
    { "mod-embed-srv", 13 },
    -1,
    PJSIP_MOD_PRIORITY_APPLICATION,
    NULL, // load()
    NULL,	// start()
    NULL, // stop()
    NULL, // unload()
    &EmbeddedTestServerCallbacks::on_rx_request,	// on_rx_request()
    &EmbeddedTestServerCallbacks::on_rx_response, // on_rx_response()
    NULL, // on_tx_request()
    NULL,	// on_tx_response()
    NULL,	// on_tsx_state()
};

pj_status_t LookupCredentials(pj_pool_t *pool,
    const pj_str_t *realm, const pj_str_t *acc_name,
    pjsip_cred_info *cred_info) {
  if (pj_strcmp2(realm, "no-biloxi.com")
      && pj_strcmp2(realm, "test")
      && pj_strcmp2(&cred_info->scheme, "digest")) {
    cred_info->realm = pj_str("no-biloxi.com");
    cred_info->username = pj_str("test");
    cred_info->data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
    cred_info->data = pj_str("1234");
  }
  return PJ_ENOTFOUND;
}


} // empty namespace

struct EmbeddedTestServer::ControlStruct {
  pj_caching_pool caching_pool_;
  pjsip_endpoint* endpoint_;
  pj_pool_t* pool_;
  pj_thread_t* thread_;
  pjsip_auth_srv auth_srv_;

  ControlStruct()
    : endpoint_(NULL),
      pool_(NULL),
      thread_(NULL) {
    memset(&caching_pool_, 0, sizeof(caching_pool_));
    memset(&auth_srv_, 0, sizeof(auth_srv_));
  }

  ~ControlStruct() {
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

    status = pjlib_util_init();
    if (status != PJ_SUCCESS)
      return false;

    pj_caching_pool_init(&caching_pool_,
      &pj_pool_factory_default_policy, 0);

    status = pjsip_endpt_create(&caching_pool_.factory, NULL, &endpoint_);
    if (status != PJ_SUCCESS)
      return false;

    status = pjsip_tsx_layer_init_module(endpoint_);
    if (status != PJ_SUCCESS)
      return false;

    pool_ = pj_pool_create(&caching_pool_.factory, "embedsrv",
			4000, 4000, NULL);
    return true;
  }

  bool ListenTo(const Protocol& protocol, int port) {
    pj_status_t status;

    pj_sockaddr_in addr;
    pj_str_t localhost = pj_str("127.0.0.1");
    addr.sin_family = pj_AF_INET();
    pj_inet_pton(PJ_AF_INET, &localhost, &addr.sin_addr.s_addr);
    addr.sin_port = pj_htons((pj_uint16_t)port);

    if (Protocol::UDP == protocol) {
      status = pjsip_udp_transport_start(endpoint_, &addr, NULL, 1, NULL);
    } else if (Protocol::TCP == protocol) {
      status = pjsip_tcp_transport_start(endpoint_, &addr, 1, NULL);
    } else if (Protocol::TLS == protocol) {
      pjsip_tls_setting opt;
      pjsip_tls_setting_default(&opt);
      // TODO: add other TLS settings, like self-signed certificate here
      status = pjsip_tls_transport_start(endpoint_, &opt, &addr,
        NULL, 1, NULL);
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
};

EmbeddedTestServer::EmbeddedTestServer(const Protocol &protocol)
  : port_(-1) {
  DCHECK(!g_server);
  DCHECK(thread_checker_.CalledOnValidThread());
  std::ostringstream uri;
  if (Protocol::UDP == protocol) {
    uri << "sip:127.0.0.1";
    port_ = 5060;
  } else if (Protocol::TCP == protocol) {
    uri << "sip:127.0.0.1;transport=TCP";
    port_ = 5060;
  } else if (Protocol::TLS == protocol) {
    uri << "sips:127.0.0.1";
    port_ = 5061;
  }
  base_uri_ = GURL(uri.str());
  g_server = this;
}

EmbeddedTestServer::~EmbeddedTestServer() {
  DCHECK(thread_checker_.CalledOnValidThread());

  if (Started() && !ShutdownAndWaitUntilComplete()) {
    LOG(ERROR) << "EmbeddedTestServer failed to shut down.";
  }
  
  g_server = NULL;
}

bool EmbeddedTestServer::InitializeAndWaitUntilReady() {
  base::Thread::Options thread_options;
  thread_options.message_loop_type = base::MessageLoop::TYPE_IO;
  io_thread_.reset(new base::Thread("EmbeddedTestServer io thread"));
  CHECK(io_thread_->StartWithOptions(thread_options));

  DCHECK(thread_checker_.CalledOnValidThread());

  if (!PostTaskToIOThreadAndWait(base::Bind(
          &EmbeddedTestServer::InitializeOnIOThread, base::Unretained(this)))) {
    return false;
  }

  return Started();
}

bool EmbeddedTestServer::ShutdownAndWaitUntilComplete() {
  DCHECK(thread_checker_.CalledOnValidThread());

  return PostTaskToIOThreadAndWait(base::Bind(
      &EmbeddedTestServer::ShutdownOnIOThread, base::Unretained(this)));
}

bool EmbeddedTestServer::Started() const {
  return control_struct_.get() != NULL;
}

void EmbeddedTestServer::OnReceiveRequest(pjsip_rx_data *rdata) {
  if (rdata->msg_info.msg->line.req.method.id == PJSIP_CANCEL_METHOD)
    return;

  if (!VerifyRequest(rdata))
    return;

  if (rdata->msg_info.msg->line.req.method.id == PJSIP_REGISTER_METHOD) {
    // Authentication was OK, so just return success
    pj_status_t status;
    pjsip_tx_data *tdata = NULL;
    do {
      status = pjsip_endpt_create_response(control_struct_->endpoint_,
          rdata, PJSIP_SC_OK, NULL, &tdata);
      if (status != PJ_SUCCESS)
        break;
      pjsip_transaction *uas_tsx;
      status = pjsip_tsx_create_uas(NULL, rdata, &uas_tsx);
      if (status != PJ_SUCCESS)
        break;
      pjsip_tsx_recv_msg(uas_tsx, rdata);
      pjsip_tsx_send_msg(uas_tsx, tdata);
      return;
    } while (0);
    if (tdata)
      pjsip_tx_data_dec_ref(tdata);
	  pjsip_endpt_respond_stateless(control_struct_->endpoint_, rdata,
					PJSIP_SC_INTERNAL_SERVER_ERROR, 
					NULL, NULL, NULL);
  }
}

bool EmbeddedTestServer::VerifyRequest(pjsip_rx_data *rdata) {
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
        PJSIP_SC_UNSUPPORTED_URI_SCHEME, NULL, NULL, NULL);
    return false;
  }

  // 3. Require
  if (pjsip_msg_find_hdr_by_name(rdata->msg_info.msg, &STR_REQUIRE,
      NULL) != NULL) {
    pjsip_endpt_respond_stateless(control_struct_->endpoint_, rdata,
        PJSIP_SC_BAD_EXTENSION, NULL, NULL, NULL);
    return false;
  }

  // 4. Authorization
  int status_code;
  status = pjsip_auth_srv_verify(&control_struct_->auth_srv_, rdata,
      &status_code);
  if (status == PJSIP_EAUTHNOAUTH) {
    pjsip_tx_data *tdata = NULL;
    do {
      status = pjsip_endpt_create_response(control_struct_->endpoint_,
          rdata, status_code, NULL, &tdata);
      if (status != PJ_SUCCESS)
        break;
      status = pjsip_auth_srv_challenge(&control_struct_->auth_srv_,
          NULL, NULL, NULL, PJ_FALSE, tdata);
      if (status != PJ_SUCCESS)
        break;
      pjsip_transaction *uas_tsx;
      status = pjsip_tsx_create_uas(NULL, rdata, &uas_tsx);
      if (status != PJ_SUCCESS)
        break;
      pjsip_tsx_recv_msg(uas_tsx, rdata);
      pjsip_tsx_send_msg(uas_tsx, tdata);
      return false;
    } while (0);
    if (tdata)
      pjsip_tx_data_dec_ref(tdata);
	  pjsip_endpt_respond_stateless(control_struct_->endpoint_, rdata,
					PJSIP_SC_INTERNAL_SERVER_ERROR, 
					NULL, NULL, NULL);
    return false;
  } else if (status != PJ_SUCCESS) {
    pjsip_endpt_respond_stateless(control_struct_->endpoint_, rdata,
        PJSIP_SC_FORBIDDEN, NULL, NULL, NULL);
    return false;
  }

  return true;
}

void EmbeddedTestServer::OnReceiveResponse(pjsip_rx_data *rdata) {
}

void EmbeddedTestServer::InitializeOnIOThread() {
  DCHECK(io_thread_->message_loop_proxy()->BelongsToCurrentThread());
  DCHECK(!Started());

  control_struct_.reset(new ControlStruct);
  if (!control_struct_->Init()
      || !control_struct_->ListenTo(protocol_, port_)
      || !control_struct_->RegisterModule()
      || !control_struct_->StartAuthentication())
    ShutdownOnIOThread();
}

void EmbeddedTestServer::ShutdownOnIOThread() {
  DCHECK(io_thread_->message_loop_proxy()->BelongsToCurrentThread());

  control_struct_.reset(NULL);
  pj_shutdown();
}

bool EmbeddedTestServer::PostTaskToIOThreadAndWait(
    const base::Closure& closure) {
  // Note that PostTaskAndReply below requires base::MessageLoopProxy::current()
  // to return a loop for posting the reply task. However, in order to make
  // EmbeddedTestServer universally usable, it needs to cope with the situation
  // where it's running on a thread on which a message loop is not (yet)
  // available or as has been destroyed already.
  //
  // To handle this situation, create temporary message loop to support the
  // PostTaskAndReply operation if the current thread as no message loop.
  scoped_ptr<base::MessageLoop> temporary_loop;
  if (!base::MessageLoop::current())
    temporary_loop.reset(new base::MessageLoop());

  base::RunLoop run_loop;
  if (!io_thread_->message_loop_proxy()->PostTaskAndReply(
          FROM_HERE, closure, run_loop.QuitClosure())) {
    return false;
  }
  run_loop.Run();

  return true;
}

} // namespace sippet
