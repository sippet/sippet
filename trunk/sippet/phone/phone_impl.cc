// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/phone_impl.h"
#include "sippet/phone/call_impl.h"

#include "base/bind.h"
#include "sippet/base/casting.h"
#include "base/strings/utf_string_conversions.h"
#include "net/base/net_errors.h"
#include "net/socket/client_socket_factory.h"
#include "net/proxy/proxy_config_service.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_builder.h"
#include "sippet/ua/dialog_controller.h"
#include "talk/media/devices/devicemanager.h"
#include "jingle/glue/thread_wrapper.h"
#include "webrtc/base/ssladapter.h"

namespace sippet {

namespace {

// Config getter that always returns direct settings.
class ProxyConfigServiceDirect : public net::ProxyConfigService {
public:
  // Overridden from ProxyConfigService:
  void AddObserver(Observer* observer) override {}
  void RemoveObserver(Observer* observer) override {}
  ConfigAvailability GetLatestProxyConfig(
    net::ProxyConfig* config) override {
    *config = net::ProxyConfig::CreateDirect();
    return CONFIG_VALID;
  }
};

class URLRequestContextGetter : public net::URLRequestContextGetter {
 public:
  explicit URLRequestContextGetter(
    scoped_refptr<base::SingleThreadTaskRunner> network_task_runner)
      : network_task_runner_(network_task_runner) {
  }

  // Overridden from net::URLRequestContextGetter:
  net::URLRequestContext* GetURLRequestContext() override {
    CHECK(network_task_runner_->BelongsToCurrentThread());
    if (!url_request_context_) {
      net::URLRequestContextBuilder builder;
      // net::HttpServer fails to parse headers if user-agent header is blank.
      builder.set_user_agent("Sippet");
      builder.DisableHttpCache();
#if defined(OS_LINUX) || defined(OS_ANDROID)
      builder.set_proxy_config_service(new ProxyConfigServiceDirect());
#endif
      url_request_context_.reset(builder.Build());
    }
    return url_request_context_.get();
  }

  scoped_refptr<base::SingleThreadTaskRunner>
    GetNetworkTaskRunner() const override {
    return network_task_runner_;
  }

 private:
  ~URLRequestContextGetter() override {}

  scoped_refptr<base::SingleThreadTaskRunner> network_task_runner_;

  // Only accessed on the IO thread.
  scoped_ptr<net::URLRequestContext> url_request_context_;

  DISALLOW_COPY_AND_ASSIGN(URLRequestContextGetter);
};

} // empty namespace

namespace phone {

//
// Phone::AccountPasswordHandler::Factory implementation
//
PhoneImpl::AccountPasswordHandler::Factory::Factory() {
}

PhoneImpl::AccountPasswordHandler::Factory::~Factory() {
}

const Account &PhoneImpl::AccountPasswordHandler::Factory::account() const {
  return account_;
}

void PhoneImpl::AccountPasswordHandler::Factory::set_account(const Account &account) {
  account_ = account;
}

scoped_ptr<PasswordHandler>
      PhoneImpl::AccountPasswordHandler::Factory::CreatePasswordHandler() {
  scoped_ptr<PasswordHandler> password_handler(
    new AccountPasswordHandler(this));
  return password_handler.Pass();
}

//
// Phone::AccountPasswordHandler implementation
//
PhoneImpl::AccountPasswordHandler::AccountPasswordHandler(Factory *factory) :
  factory_(factory) {
}

PhoneImpl::AccountPasswordHandler::~AccountPasswordHandler() {
}

int PhoneImpl::AccountPasswordHandler::GetCredentials(
      const net::AuthChallengeInfo* auth_info,
      base::string16 *username,
      base::string16 *password,
      const net::CompletionCallback& callback) {
  *username = base::UTF8ToUTF16(factory_->account().username());
  *password = base::UTF8ToUTF16(factory_->account().password());
  return net::OK;
}

//
// Phone implementation
//
PhoneImpl::PhoneImpl(PhoneObserver *phone_observer)
  : state_(kStateOffline),
    phone_observer_(phone_observer),
    signalling_thread_("PhoneSignalling"),
    signalling_thread_event_(false, false),
    password_handler_factory_(new AccountPasswordHandler::Factory) {
  DCHECK(phone_observer);
}

PhoneImpl::~PhoneImpl() {
  signalling_thread_.message_loop()->PostTask(FROM_HERE,
    base::Bind(&PhoneImpl::OnDestroy, base::Unretained(this))
  );
  signalling_thread_event_.Wait();
}

bool PhoneImpl::Init(const Settings& settings) {
  if (!signalling_thread_.Start()) {
    return false;
  }
  signalling_thread_.message_loop()->PostTask(FROM_HERE,
    base::Bind(&PhoneImpl::OnInit, base::Unretained(this))
  );
  return true;
}

bool PhoneImpl::Login(const Account &account) {
  // Evaluate the account host attribute first
  GURL url(account.host());
  if (!url.SchemeIs("sip")
      && !url.SchemeIs("sips")) {
    DVLOG(1) << "Unknown scheme '" << url.scheme()
             << "' in account host attribute";
    return false;
  }
  SipURI uri(url.spec());
  scheme_ = uri.scheme();
  host_ = uri.host();
  if (uri.has_port())
    host_ += ":" + uri.port();
  if (uri.has_parameters())
    host_ += uri.parameters();
  username_ = account.username();
  password_handler_factory_->set_account(account);
  signalling_thread_.message_loop()->PostTask(FROM_HERE,
    base::Bind(&PhoneImpl::OnLogin, base::Unretained(this), account)
  );
  return true;
}

scoped_refptr<Call> PhoneImpl::MakeCall(const std::string& destination) {
  if (destination.empty()) {
    DVLOG(1) << "Empty destination";
    return false;
  }
  base::AutoLock lock(lock_);
  scoped_refptr<CallImpl> call(new CallImpl(
      SipURI(scheme_ + ":" + destination + "@" + host_), this));
  calls_.push_back(call);
  signalling_thread_.message_loop()->PostTask(FROM_HERE,
    base::Bind(&CallImpl::OnMakeCall, base::Unretained(call.get()),
      base::Unretained(peer_connection_factory_.get()))
  );
  return call;
}

void PhoneImpl::HangUpAll() {
  base::AutoLock lock(lock_);
  for (CallsVector::iterator i = calls_.begin(); i != calls_.end(); ++i) {
    i->get()->HangUp();
  }
}

void PhoneImpl::Logout() {
  signalling_thread_.message_loop()->PostTask(FROM_HERE,
    base::Bind(&PhoneImpl::OnLogout, base::Unretained(this))
  );
}

void PhoneImpl::RemoveCall(const scoped_refptr<Call>& call) {
  base::AutoLock lock(lock_);
  CallsVector::iterator i;
  for (i = calls_.begin(); i != calls_.end(); ++i) {
    if (*i == call.get())
      break;
  }
  if (calls_.end() != i)
    calls_.erase(i);
}

bool PhoneImpl::InitializePeerConnectionFactory() {
  DCHECK(peer_connection_factory_.get() == nullptr);

  // To allow sending to the signaling/worker threads.
  jingle_glue::JingleThreadWrapper::EnsureForCurrentMessageLoop();
  jingle_glue::JingleThreadWrapper::current()->set_send_allowed(true);

  peer_connection_factory_ = webrtc::CreatePeerConnectionFactory();
  if (!peer_connection_factory_.get()) {
    DeletePeerConnectionFactory();
    return false;
  }

  webrtc::PeerConnectionFactoryInterface::Options options;
  options.disable_encryption = true;
  options.disable_sctp_data_channels = true;
  peer_connection_factory_->SetOptions(options);
  return true;
}

void PhoneImpl::DeletePeerConnectionFactory() {
  peer_connection_factory_ = nullptr;
}

void PhoneImpl::OnInit() {
  base::MessageLoop *message_loop = base::MessageLoop::current();
  request_context_getter_ =
    new URLRequestContextGetter(message_loop->message_loop_proxy());

  net::ClientSocketFactory *client_socket_factory =
    net::ClientSocketFactory::GetDefaultFactory();
  host_resolver_ = net::HostResolver::CreateDefaultResolver(nullptr);
  scoped_ptr<AuthHandlerRegistryFactory> auth_handler_factory(
    AuthHandlerFactory::CreateDefault(host_resolver_.get()));
  auth_handler_factory_ =
    auth_handler_factory.Pass();

  user_agent_.reset(new ua::UserAgent(auth_handler_factory_.get(),
    password_handler_factory_.get(),
    DialogController::GetDefaultDialogController(),
    net_log_));

  network_layer_.reset(new NetworkLayer(user_agent_.get()));

  // Register the channel factory
  net::SSLConfig ssl_config;
  ssl_config.version_min = net::SSL_PROTOCOL_VERSION_TLS1;
  channel_factory_.reset(
    new ChromeChannelFactory(client_socket_factory,
    request_context_getter_, ssl_config));
  network_layer_->RegisterChannelFactory(Protocol::UDP,
    channel_factory_.get());
  network_layer_->RegisterChannelFactory(Protocol::TCP,
    channel_factory_.get());
  network_layer_->RegisterChannelFactory(Protocol::TLS,
    channel_factory_.get());
  user_agent_->SetNetworkLayer(network_layer_.get());
  user_agent_->AppendHandler(this);

  InitializePeerConnectionFactory();
}

void PhoneImpl::OnDestroy() {
  channel_factory_.reset();
  auth_handler_factory_.reset();
  host_resolver_.reset();
  user_agent_ = nullptr;
  network_layer_ = nullptr;
  request_context_getter_ = nullptr;
  DeletePeerConnectionFactory();

  signalling_thread_event_.Signal();
}

void PhoneImpl::OnLogin(const Account &account) {
  std::string registrar_uri("sip:" + host_);
  std::string from("sip:" + username_ + "@" + host_);
  std::string to("sip:" + username_ + "@" + host_);

  last_request_ =
    user_agent_->CreateRequest(
        Method::REGISTER,
        GURL(registrar_uri),
        GURL(from),
        GURL(to));

  state_ = kStateConnecting;
  int rv = user_agent_->Send(last_request_,
      base::Bind(&PhoneImpl::OnRequestSent, base::Unretained(this)));
  if (net::OK != rv && net::ERR_IO_PENDING != rv) {
    phone_observer_->OnNetworkError(rv);
    return;
  }

  // Wait for SIP response now
}

void PhoneImpl::OnLogout() {
  std::string registrar_uri("sip:" + host_);
  std::string from("sip:" + username_ + "@" + host_);
  std::string to("sip:" + username_ + "@" + host_);

  last_request_ =
    user_agent_->CreateRequest(
        Method::REGISTER,
        GURL(registrar_uri),
        GURL(from),
        GURL(to));

  // Modifies the Contact header to include an expires=0 at the end
  // of existing parameters
  Contact* contact = last_request_->get<Contact>();
  contact->front().set_expires(0);

  state_ = kStateOffline;
  int rv = user_agent_->Send(last_request_,
      base::Bind(&PhoneImpl::OnRequestSent, base::Unretained(this)));
  if (net::OK != rv && net::ERR_IO_PENDING != rv) {
    phone_observer_->OnNetworkError(rv);
    return;
  }

  // Wait for SIP response now
}

void PhoneImpl::OnRequestSent(int rv) {
  if (net::OK != rv) {
    phone_observer_->OnNetworkError(rv);
  }
}

void PhoneImpl::OnChannelConnected(const EndPoint &destination, int err) {
  if (net::OK != err) {
    phone_observer_->OnNetworkError(err);
  }
}

void PhoneImpl::OnChannelClosed(const EndPoint &destination) {
  // Nothing to do
}

void PhoneImpl::OnIncomingRequest(
    const scoped_refptr<Request> &incoming_request,
    const scoped_refptr<Dialog> &dialog) {
  if (Method::INVITE == incoming_request->method()) {
    scoped_refptr<CallImpl> call(new CallImpl(incoming_request, this));
    {
      base::AutoLock lock(lock_);
      calls_.push_back(call);
    }
    phone_observer_->OnIncomingCall(call);
  } else {
    scoped_refptr<CallImpl> call = RouteToCall(dialog);
    if (call)
      call->OnIncomingRequest(incoming_request, dialog);
  }
}

void PhoneImpl::OnIncomingResponse(
    const scoped_refptr<Response> &incoming_response,
    const scoped_refptr<Dialog> &dialog) {
  if (Method::REGISTER == incoming_response->refer_to()->method()) {
    if (last_request_->id() != incoming_response->refer_to()->id()) {
      // Discard, as it was related to current REGISTER request
      return;
    } else if (kStateConnecting == state_) {
      int response_code = incoming_response->response_code();
      if (response_code / 100 == 1) {
        // Do nothing, wait for a final response
        return;
      } else if (response_code / 100 == 2) {
        // Start a timer to refresh login
        unsigned int expiration = GetContactExpiration(incoming_response);
        refresh_timer_.Start(FROM_HERE, base::TimeDelta::FromSeconds(expiration),
          base::Bind(&PhoneImpl::OnRefreshLogin, base::Unretained(this)));
        state_ = kStateOnline;
      } else {
        state_ = kStateOffline;
      }

      // Notify completion
      phone_observer_->OnLoginCompleted(incoming_response->response_code(),
        incoming_response->reason_phrase());
    }
  } else if (Method::INVITE == incoming_response->refer_to()->method()
             || Method::BYE == incoming_response->refer_to()->method()) {
    CallImpl *call = RouteToCall(incoming_response->refer_to());
    if (call)
      call->OnIncomingResponse(incoming_response, dialog);
  }
}

void PhoneImpl::OnTimedOut(
    const scoped_refptr<Request> &request,
    const scoped_refptr<Dialog> &dialog) {
  if (Method::INVITE == request->method()
      || Method::BYE == request->method()) {
    CallImpl *call = RouteToCall(request);
    if (call)
      call->OnTimedOut(request, dialog);
  }
}

void PhoneImpl::OnTransportError(
    const scoped_refptr<Request> &request, int error,
    const scoped_refptr<Dialog> &dialog) {
  if (Method::INVITE == request->method()
      || Method::BYE == request->method()) {
    CallImpl *call = RouteToCall(request);
    if (call)
      call->OnTransportError(request, error, dialog);
  }
}

void PhoneImpl::OnRefreshLogin() {
  // TODO
}

CallImpl *PhoneImpl::RouteToCall(const scoped_refptr<Request>& request) {
  base::AutoLock lock(lock_);
  CallsVector::iterator i;
  // A simple linear search will do it for a small amount of
  // simultaneous calls... But you can improve it if you want
  for (i = calls_.begin(); i != calls_.end(); ++i) {
    if (i->get()->last_request()->id() == request->id())
      break;
  }
  return calls_.end() != i ? i->get() : nullptr;
}

CallImpl *PhoneImpl::RouteToCall(const scoped_refptr<Dialog>& dialog) {
  base::AutoLock lock(lock_);
  CallsVector::iterator i;
  for (i = calls_.begin(); i != calls_.end(); ++i) {
    if (i->get()->dialog()->id() == dialog->id())
      break;
  }
  return calls_.end() != i ? i->get() : nullptr;
}

unsigned int PhoneImpl::GetContactExpiration(
      const scoped_refptr<Response>& incoming_response) {
  unsigned int header_expires = 0;
  
  Expires *expires = incoming_response->get<Expires>();
  if (expires)
    header_expires = expires->value();
  
  Contact *request_contact = incoming_response->refer_to()->get<Contact>();
  DCHECK(request_contact) << "REGISTER request without Contact?";
  
  GURL local_uri = request_contact->front().address();
  ContactInfo *local_contact = nullptr;
  for (Message::iterator i = incoming_response->find_first<Contact>(),
    ie = incoming_response->end(); i != ie; ++i) {
    Contact *contact = dyn_cast<Contact>(i);
    for (Contact::iterator j = contact->begin(), je = contact->end();
      j != je; ++j) {
      if (j->address() == local_uri) {
        if (j->HasExpires()) {
          return j->expires();
        } else if (0 != header_expires) {
          return header_expires;
        } else {
          LOG(WARNING) << "Malformed response, assuming expiration = 3600";
          return 3600;
        }
      }
    }
  }
  LOG(WARNING) << "Response doesn't contain a Contact for our local URI";
  return 3600;
}

void Phone::Initialize() {
  // Initialize the SSL libraries
  rtc::InitializeSSL();
}

scoped_refptr<Phone> Phone::Create(PhoneObserver *phone_observer) {
  return new PhoneImpl(phone_observer);
}

} // namespace sippet
} // namespace phone