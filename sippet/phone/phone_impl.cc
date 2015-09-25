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
// Phone::PasswordHandler::Factory implementation
//
PhoneImpl::PasswordHandler::Factory::Factory(Settings *settings) :
  settings_(settings) {
}

PhoneImpl::PasswordHandler::Factory::~Factory() {
}

scoped_ptr<PasswordHandler>
      PhoneImpl::PasswordHandler::Factory::CreatePasswordHandler() {
  scoped_ptr<PasswordHandler> password_handler(
    new PasswordHandler(this));
  return password_handler.Pass();
}

//
// Phone::PasswordHandler implementation
//
PhoneImpl::PasswordHandler::PasswordHandler(Factory *factory) :
  factory_(factory) {
}

PhoneImpl::PasswordHandler::~PasswordHandler() {
}

int PhoneImpl::PasswordHandler::GetCredentials(
      const net::AuthChallengeInfo* auth_info,
      base::string16 *username,
      base::string16 *password,
      const net::CompletionCallback& callback) {
  std::string authorization_user;
  if (factory_->settings()->authorization_user().empty()) {
    SipURI uri(factory_->settings()->uri().spec());
    authorization_user = uri.username();
  }
  else
    authorization_user = factory_->settings()->authorization_user();
  *username = base::UTF8ToUTF16(authorization_user);
  *password = base::UTF8ToUTF16(factory_->settings()->password());
  return net::OK;
}

//
// Phone implementation
//
PhoneImpl::PhoneImpl(Phone::Delegate *delegate)
  : state_(PHONE_STATE_OFFLINE),
    last_state_(PHONE_STATE_OFFLINE),
    delegate_(delegate),
    signalling_thread_("PhoneSignalling"),
    signalling_thread_event_(false, false) {
  DCHECK(delegate);
}

PhoneImpl::~PhoneImpl() {
  signalling_thread_.message_loop()->PostTask(FROM_HERE,
    base::Bind(&PhoneImpl::OnDestroy, base::Unretained(this))
  );
  signalling_thread_event_.Wait();
}

PhoneState PhoneImpl::state() const {
  return state_;
}

bool PhoneImpl::Init(const Settings& settings) {
  if (settings_.is_valid()) {
    DVLOG(1) << "Already initialized";
    return false;
  }
  if (!settings.uri().SchemeIs("sip")
      && !settings.uri().SchemeIs("sips")) {
    DVLOG(1) << "Unknown scheme '" << settings.uri().scheme()
             << "' in uri attribute";
    return false;
  }
  if (!settings.registrar_server().is_empty()
      && !settings.registrar_server().SchemeIs("sip")
      && !settings.registrar_server().SchemeIs("sips")) {
    DVLOG(1) << "Unknown scheme '" << settings.registrar_server().scheme()
             << "' in registrar_server attribute";
    return false;
  }
  base::Thread::Options options;
  settings_ = settings;
  password_handler_factory_.reset(new PasswordHandler::Factory(&settings_));
  options.message_loop_type = base::MessageLoop::TYPE_IO;
  if (!signalling_thread_.StartWithOptions(options)) {
    return false;
  }
  state_ = PHONE_STATE_READY;
  signalling_thread_.message_loop()->PostTask(FROM_HERE,
    base::Bind(&PhoneImpl::OnInit, base::Unretained(this), settings)
  );
  return true;
}

bool PhoneImpl::Register() {
  if (PHONE_STATE_READY != state_) {
    DVLOG(1) << "Not ready";
    return false;
  }
  base::AutoLock lock(lock_);
  state_ = PHONE_STATE_REGISTERING;
  signalling_thread_.message_loop()->PostTask(FROM_HERE,
    base::Bind(&PhoneImpl::OnRegister, base::Unretained(this))
  );
  return true;
}

bool PhoneImpl::Unregister() {
  if (PHONE_STATE_REGISTERED != state_) {
    DVLOG(1) << "Not ready";
    return false;
  }
  base::AutoLock lock(lock_);
  last_state_ = state_;
  state_ = PHONE_STATE_UNREGISTERING;
  signalling_thread_.message_loop()->PostTask(FROM_HERE,
    base::Bind(&PhoneImpl::OnUnregister, base::Unretained(this))
  );
  return true;
}

bool PhoneImpl::UnregisterAll() {
  if (PHONE_STATE_READY != state_
      && PHONE_STATE_REGISTERED != state_) {
    DVLOG(1) << "Not ready";
    return false;
  }
  base::AutoLock lock(lock_);
  last_state_ = state_;
  state_ = PHONE_STATE_UNREGISTERING;
  signalling_thread_.message_loop()->PostTask(FROM_HERE,
    base::Bind(&PhoneImpl::OnUnregisterAll, base::Unretained(this))
  );
  return true;
}

scoped_refptr<Call> PhoneImpl::MakeCall(const std::string& destination) {
  if (destination.empty()) {
    DVLOG(1) << "Empty destination";
    return nullptr;
  }
  if (PHONE_STATE_READY != state_
      && PHONE_STATE_REGISTERED != state_) {
    DVLOG(1) << "Not ready";
    return nullptr;
  }
  base::AutoLock lock(lock_);
  SipURI destination_uri(GetToUri(destination));
  if (!destination_uri.is_valid()) {
    DVLOG(1) << "Invalid destination";
    return nullptr;
  }
  scoped_refptr<CallImpl> call(new CallImpl(destination_uri, this));
  calls_.push_back(call);
  signalling_thread_.message_loop()->PostTask(FROM_HERE,
    base::Bind(&CallImpl::OnMakeCall, base::Unretained(call.get()),
      base::Unretained(peer_connection_factory_.get()), ice_servers_)
  );
  return call;
}

void PhoneImpl::HangUpAll() {
  base::AutoLock lock(lock_);
  for (CallsVector::iterator i = calls_.begin(); i != calls_.end(); ++i) {
    i->get()->HangUp();
  }
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

bool PhoneImpl::InitializePeerConnectionFactory(const Settings& settings) {
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
  options.disable_encryption = settings.disable_encryption();
  options.disable_sctp_data_channels = settings.disable_sctp_data_channels();
  peer_connection_factory_->SetOptions(options);
  return true;
}

void PhoneImpl::DeletePeerConnectionFactory() {
  peer_connection_factory_ = nullptr;
}

void PhoneImpl::OnInit(const Settings& settings) {
  base::MessageLoop *message_loop = signalling_thread_.message_loop();
  refresh_timer_.reset(new base::OneShotTimer<PhoneImpl>);

  request_context_getter_ =
    new URLRequestContextGetter(message_loop->task_runner());

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

  InitializePeerConnectionFactory(settings);

  // Initialize the route-set, if available
  if (settings.route_set().size() > 0) {
    user_agent_->set_route_set(settings.route_set());
  }

  // Save the ICE server list for later, while initializing
  // a PeerConnection instance.
  ice_servers_ = settings.ice_servers();
}

void PhoneImpl::OnDestroy() {
  for (CallsVector::iterator i = calls_.begin(), ie = calls_.end();
       i != ie; i++) {
    (*i)->OnDestroy();
  }
  calls_.clear();

  channel_factory_.reset();
  auth_handler_factory_.reset();
  host_resolver_.reset();
  user_agent_ = nullptr;
  network_layer_ = nullptr;
  request_context_getter_ = nullptr;
  DeletePeerConnectionFactory();
  refresh_timer_->Stop();
  refresh_timer_.reset();

  signalling_thread_event_.Signal();
}

void PhoneImpl::OnRegister() {
  std::string registrar_uri(GetRegistrarUri());
  std::string address_of_record(GetFromUri());

  last_request_ =
    user_agent_->CreateRequest(
        Method::REGISTER,
        GURL(registrar_uri),
        GURL(address_of_record),
        GURL(address_of_record));

  // Indicate the desired expiration for the address-of-record binding
  scoped_ptr<Expires> expires(new Expires(settings_.register_expires()));
  last_request_->push_back(expires.Pass());

  int rv = user_agent_->Send(last_request_,
      base::Bind(&PhoneImpl::OnRequestSent, base::Unretained(this)));
  if (net::OK != rv && net::ERR_IO_PENDING != rv) {
    delegate_->OnNetworkError(rv);
    return;
  }

  // Wait for SIP response now
}

void PhoneImpl::OnUnregister() {
  std::string registrar_uri(GetRegistrarUri());
  std::string address_of_record(GetFromUri());

  last_request_ =
    user_agent_->CreateRequest(
        Method::REGISTER,
        GURL(registrar_uri),
        GURL(address_of_record),
        GURL(address_of_record));

  // Modifies the Contact header to include an expires=0 at the end
  // of existing parameters
  Contact* contact = last_request_->get<Contact>();
  contact->front().set_expires(0);

  int rv = user_agent_->Send(last_request_,
      base::Bind(&PhoneImpl::OnRequestSent, base::Unretained(this)));
  if (net::OK != rv && net::ERR_IO_PENDING != rv) {
    delegate_->OnNetworkError(rv);
    return;
  }

  // Wait for SIP response now
}

void PhoneImpl::OnUnregisterAll() {
  std::string registrar_uri(GetRegistrarUri());
  std::string address_of_record(GetFromUri());

  last_request_ =
    user_agent_->CreateRequest(
        Method::REGISTER,
        GURL(registrar_uri),
        GURL(address_of_record),
        GURL(address_of_record));

  // Uses a "*" as the contact header
  Contact* contact = last_request_->get<Contact>();
  contact->set_all(true);

  // Set expiration to 0 for all contacts
  scoped_ptr<Expires> expires(new Expires(0));
  last_request_->push_back(expires.Pass());

  int rv = user_agent_->Send(last_request_,
      base::Bind(&PhoneImpl::OnRequestSent, base::Unretained(this)));
  if (net::OK != rv && net::ERR_IO_PENDING != rv) {
    delegate_->OnNetworkError(rv);
    return;
  }

  // Wait for SIP response now
}

void PhoneImpl::OnRequestSent(int rv) {
  if (net::OK != rv) {
    delegate_->OnNetworkError(rv);
  }
}

void PhoneImpl::OnChannelConnected(const EndPoint &destination, int err) {
  if (net::OK != err) {
    delegate_->OnNetworkError(err);
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
    delegate_->OnIncomingCall(call);
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
      // Discard, as it was unrelated to current REGISTER request
      return;
    } else if (PHONE_STATE_REGISTERING == state_) { // result of Register
      int response_code = incoming_response->response_code();
      if (response_code / 100 == 1) {
        // Do nothing, wait for a final response
        return;
      } else if (response_code / 100 == 2) {
        // Start a timer to refresh login
        unsigned int expiration = GetContactExpiration(incoming_response);
        refresh_timer_->Start(FROM_HERE, base::TimeDelta::FromSeconds(expiration),
          base::Bind(&PhoneImpl::OnRefreshLogin, base::Unretained(this)));
        base::AutoLock lock(lock_);
        state_ = PHONE_STATE_REGISTERED;
      } else {
        base::AutoLock lock(lock_);
        state_ = PHONE_STATE_READY;
      }
      // Notify completion
      delegate_->OnRegisterCompleted(incoming_response->response_code(),
        incoming_response->reason_phrase());
    } else if (PHONE_STATE_REGISTERED == state_) { // after refresh register
      int response_code = incoming_response->response_code();
      if (response_code / 100 == 1) {
        // Do nothing, wait for a final response
        return;
      } else if (response_code / 100 == 2) {
        // Start the timer to refresh login again
        unsigned int expiration = GetContactExpiration(incoming_response);
        refresh_timer_->Start(FROM_HERE, base::TimeDelta::FromSeconds(expiration),
          base::Bind(&PhoneImpl::OnRefreshLogin, base::Unretained(this)));
      } else {
        {
          base::AutoLock lock(lock_);
          state_ = PHONE_STATE_READY;
        }
        // Notify the problem upwards
        delegate_->OnRefreshError(incoming_response->response_code(),
          incoming_response->reason_phrase());
      }
    } else if (PHONE_STATE_UNREGISTERING == state_) { // result of Unregister
      int response_code = incoming_response->response_code();
      if (response_code / 100 == 1) {
        // Do nothing, wait for a final response
        return;
      } else if (response_code / 100 == 2) {
        base::AutoLock lock(lock_);
        state_ = PHONE_STATE_READY;
      } else {
        // If registration fails, recall last state
        base::AutoLock lock(lock_);
        state_ = last_state_;
      }
      // Notify completion
      delegate_->OnUnregisterCompleted(incoming_response->response_code(),
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
  // Just send another REGISTER
  OnRegister(); 
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
       ie = incoming_response->end(); i != ie;
       i = incoming_response->find_next<Contact>(i)) {
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

std::string PhoneImpl::GetRegistrarUri() const {
  if (!settings_.registrar_server().is_empty())
    return settings_.registrar_server().spec();
  else {
    SipURI uri(SipURI(settings_.uri().spec()));
    std::string result(uri.scheme() + ":" + uri.host());
    if (uri.has_port())
      result += ":" + uri.port();
    if (uri.has_parameters())
      result += uri.parameters();
    return result;
  }
}

std::string PhoneImpl::GetFromUri() const {
  return settings_.uri().spec();
}

SipURI PhoneImpl::GetToUri(const std::string& destination) const {
  SipURI destination_uri;
  if (destination.find('@') == std::string::npos) {
    SipURI uri(GetRegistrarUri());
    if (uri.is_valid()) {
      std::string result(uri.scheme() + ":" + destination + "@" + uri.host());
      if (uri.has_port())
        result += ":" + uri.port();
      if (uri.has_parameters())
        result += uri.parameters();
      destination_uri = SipURI(result);
    }
  } else {
    destination_uri = SipURI(destination);
  }
  return destination_uri;
}

void Phone::Initialize() {
  // Initialize the SSL libraries
  rtc::InitializeSSL();
}

scoped_refptr<Phone> Phone::Create(Delegate *delegate) {
  return new PhoneImpl(delegate);
}

} // namespace sippet
} // namespace phone
