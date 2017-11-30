// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/phone_impl.h"

#include <string>

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
#include "sippet/phone/completion_status.h"
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
      std::unique_ptr<ProxyConfigServiceDirect> proxy_config(
          new ProxyConfigServiceDirect);
      builder.set_proxy_config_service(std::move(proxy_config));
#endif
      url_request_context_ = builder.Build();
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
  std::unique_ptr<net::URLRequestContext> url_request_context_;

  DISALLOW_COPY_AND_ASSIGN(URLRequestContextGetter);
};

void RunIfNotOk(const net::CompletionCallback& c, int rv) {
  if (net::OK != rv) {
    c.Run(rv);
  }
}

}  // empty namespace

namespace phone {

//
// Phone::PasswordHandler::Factory implementation
//
PhoneImpl::PasswordHandler::Factory::Factory(Settings *settings) :
  settings_(settings) {
}

PhoneImpl::PasswordHandler::Factory::~Factory() {
}

std::unique_ptr<PasswordHandler>
      PhoneImpl::PasswordHandler::Factory::CreatePasswordHandler() {
  std::unique_ptr<PasswordHandler> password_handler(
    new PasswordHandler(this));
  return std::move(password_handler);
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
  } else {
    authorization_user = factory_->settings()->authorization_user();
  }
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
    network_thread_("PhoneSignalling"),
    network_thread_event_(
      base::WaitableEvent::ResetPolicy::AUTOMATIC,
      base::WaitableEvent::InitialState::NOT_SIGNALED) {
  DCHECK(delegate);
}

PhoneImpl::~PhoneImpl() {
  network_thread_.message_loop()->PostTask(FROM_HERE,
      base::Bind(&PhoneImpl::OnDestroy, base::Unretained(this)));
  network_thread_event_.Wait();
}

PhoneState PhoneImpl::state() const {
  return state_;
}

bool PhoneImpl::Init(const Settings& settings) {
  if (settings_.is_valid()) {
    DVLOG(1) << "Already initialized";
    return false;
  }
  if (!settings.is_valid()) {
    DVLOG(1) << "Invalid settings";
    return false;
  }
  base::Thread::Options options;
  settings_ = settings;
  password_handler_factory_.reset(new PasswordHandler::Factory(&settings_));
  options.message_loop_type = base::MessageLoop::TYPE_IO;
  if (!network_thread_.StartWithOptions(options)) {
    return false;
  }
  state_ = PHONE_STATE_READY;
  network_thread_.message_loop()->PostTask(FROM_HERE,
      base::Bind(&PhoneImpl::OnInit, base::Unretained(this)));
  return true;
}

void PhoneImpl::Register(const net::CompletionCallback& on_completed) {
  if (PHONE_STATE_READY != state_) {
    DVLOG(1) << "Not ready";
    return;
  }
  base::AutoLock lock(lock_);
  state_ = PHONE_STATE_REGISTERING;
  on_register_completed_ = on_completed;
  network_thread_.message_loop()->PostTask(FROM_HERE,
      base::Bind(&PhoneImpl::OnRegister, base::Unretained(this)));
}

void PhoneImpl::StartRefreshRegister(
    const net::CompletionCallback& on_completed) {
  if (PHONE_STATE_REGISTERED != state_) {
    DVLOG(1) << "Not registered";
    return;
  }
  base::AutoLock lock(lock_);
  on_refresh_completed_ = on_completed;
  network_thread_.message_loop()->PostTask(FROM_HERE,
      base::Bind(&PhoneImpl::OnStartRefreshRegister, base::Unretained(this)));
}

void PhoneImpl::StopRefreshRegister() {
  if (PHONE_STATE_REGISTERED != state_) {
    DVLOG(1) << "Not registered";
    return;
  }
  base::AutoLock lock(lock_);
  network_thread_.message_loop()->PostTask(FROM_HERE,
      base::Bind(&PhoneImpl::OnStopRefreshRegister, base::Unretained(this)));
}

void PhoneImpl::Unregister(const net::CompletionCallback& on_completed) {
  if (PHONE_STATE_REGISTERED != state_) {
    DVLOG(1) << "Not ready";
    return;
  }
  base::AutoLock lock(lock_);
  last_state_ = state_;
  state_ = PHONE_STATE_UNREGISTERING;
  on_unregister_completed_ = on_completed;
  network_thread_.message_loop()->PostTask(FROM_HERE,
      base::Bind(&PhoneImpl::OnUnregister, base::Unretained(this), false));
}

void PhoneImpl::UnregisterAll(const net::CompletionCallback& on_completed) {
  if (PHONE_STATE_READY != state_
      && PHONE_STATE_REGISTERED != state_) {
    DVLOG(1) << "Not ready";
    return;
  }
  base::AutoLock lock(lock_);
  last_state_ = state_;
  state_ = PHONE_STATE_UNREGISTERING;
  on_unregister_completed_ = on_completed;
  network_thread_.message_loop()->PostTask(FROM_HERE,
      base::Bind(&PhoneImpl::OnUnregister, base::Unretained(this), true));
}

scoped_refptr<Call> PhoneImpl::MakeCall(const std::string& destination,
    const net::CompletionCallback& on_completed) {
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
  scoped_refptr<CallImpl> call(
      new CallImpl(destination_uri, this, on_completed));
  calls_.push_back(call);
  network_thread_.message_loop()->PostTask(FROM_HERE,
      base::Bind(&CallImpl::OnMakeCall, base::Unretained(call.get()),
          base::Unretained(peer_connection_factory_.get())));
  return call;
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
  options.disable_encryption = settings_.disable_encryption();
  options.disable_sctp_data_channels = settings_.disable_sctp_data_channels();
  peer_connection_factory_->SetOptions(options);
  return true;
}

void PhoneImpl::DeletePeerConnectionFactory() {
  peer_connection_factory_ = nullptr;
}

void PhoneImpl::OnInit() {
  DCHECK(GetNetworkTaskRunner()->BelongsToCurrentThread());

  base::MessageLoop *message_loop = network_thread_.message_loop();
  refresh_timer_.reset(new base::OneShotTimer);

  request_context_getter_ =
      new URLRequestContextGetter(message_loop->task_runner());

  net::ClientSocketFactory *client_socket_factory =
      net::ClientSocketFactory::GetDefaultFactory();
  host_resolver_ = net::HostResolver::CreateDefaultResolver(nullptr);
  std::unique_ptr<AuthHandlerRegistryFactory> auth_handler_factory(
      AuthHandlerFactory::CreateDefault(host_resolver_.get()));
  auth_handler_factory_ = std::move(auth_handler_factory);

  user_agent_.reset(new ua::UserAgent(auth_handler_factory_.get(),
      password_handler_factory_.get(),
      DialogController::GetDefaultDialogController(), net_log_));

  network_layer_.reset(new NetworkLayer(user_agent_.get()));

  // Register the channel factory
  net::SSLConfig ssl_config;
  ssl_config.version_min = net::SSL_PROTOCOL_VERSION_TLS1;
  channel_factory_.reset(new ChromeChannelFactory(client_socket_factory,
      request_context_getter_, ssl_config));
  network_layer_->RegisterChannelFactory(Protocol::UDP,
      channel_factory_.get());
  network_layer_->RegisterChannelFactory(Protocol::TCP,
      channel_factory_.get());
  network_layer_->RegisterChannelFactory(Protocol::TLS,
      channel_factory_.get());
  user_agent_->SetNetworkLayer(network_layer_.get());
  user_agent_->AddObserver(this);

  InitializePeerConnectionFactory();

  // Initialize the route-set, if available
  if (settings_.route_set().size() > 0) {
    user_agent_->set_route_set(settings_.route_set());
  }
}

void PhoneImpl::OnDestroy() {
  DCHECK(GetNetworkTaskRunner()->BelongsToCurrentThread());

  for (CallsVector::iterator i = calls_.begin(), ie = calls_.end();
       i != ie; i++) {
    (*i)->OnDestroy();
  }
  calls_.clear();

  user_agent_->RemoveObserver(this);

  channel_factory_.reset();
  auth_handler_factory_.reset();
  host_resolver_.reset();
  user_agent_ = nullptr;
  network_layer_ = nullptr;
  request_context_getter_ = nullptr;
  DeletePeerConnectionFactory();
  refresh_timer_->Stop();
  refresh_timer_.reset();

  network_thread_event_.Signal();
}

void PhoneImpl::OnRegister() {
  DCHECK(GetNetworkTaskRunner()->BelongsToCurrentThread());

  std::string registrar_uri(GetRegistrarUri());
  std::string address_of_record(GetFromUri());

  last_request_ =
      user_agent_->CreateRequest(
          Method::REGISTER,
          GURL(registrar_uri),
          GURL(address_of_record),
          GURL(address_of_record));

  // Indicate the desired expiration for the address-of-record binding
  std::unique_ptr<Expires> expires(new Expires(settings_.register_expires()));
  last_request_->push_back(std::move(expires));

  int rv = user_agent_->Send(last_request_,
      base::Bind(&RunIfNotOk, on_register_completed_));
  if (net::OK != rv && net::ERR_IO_PENDING != rv) {
    on_register_completed_.Run(rv);
    return;
  }

  // Wait for SIP response now
}

void PhoneImpl::OnStartRefreshRegister() {
  DCHECK(GetNetworkTaskRunner()->BelongsToCurrentThread());

  // Refresh the registration 25 seconds before expiration.
  base::TimeDelta expiration = register_expires_ - base::Time::Now()
      - base::TimeDelta::FromSeconds(25);
  if (expiration < base::TimeDelta::FromSeconds(0))
    expiration = base::TimeDelta::FromSeconds(0);
  refresh_timer_->Start(FROM_HERE, expiration,
      base::Bind(&PhoneImpl::OnRefreshRegister, base::Unretained(this)));
}

void PhoneImpl::OnStopRefreshRegister() {
  DCHECK(GetNetworkTaskRunner()->BelongsToCurrentThread());

  refresh_timer_->Stop();
}

void PhoneImpl::OnUnregister(bool all) {
  DCHECK(GetNetworkTaskRunner()->BelongsToCurrentThread());

  std::string registrar_uri(GetRegistrarUri());
  std::string address_of_record(GetFromUri());

  last_request_ =
      user_agent_->CreateRequest(
          Method::REGISTER,
          GURL(registrar_uri),
          GURL(address_of_record),
          GURL(address_of_record));

  if (all) {
    // Uses a "*" as the contact he ader
    Contact* contact = last_request_->get<Contact>();
    contact->set_all(true);

    // Set expiration to 0 for all contacts
    std::unique_ptr<Expires> expires(new Expires(0));
    last_request_->push_back(std::move(expires));
  } else {
    // Modifies the Contact header to include an expires=0 at the end
    // of existing parameters
    Contact* contact = last_request_->get<Contact>();
    contact->front().set_expires(0);
  }

  int rv = user_agent_->Send(last_request_,
      base::Bind(&RunIfNotOk, on_unregister_completed_));
  if (net::OK != rv && net::ERR_IO_PENDING != rv) {
    on_unregister_completed_.Run(rv);
    return;
  }

  // Wait for SIP response now
}

void PhoneImpl::OnChannelConnected(const EndPoint &destination, int err) {
  // Nothing to do
}

void PhoneImpl::OnChannelClosed(const EndPoint &destination) {
  // Nothing to do
}

void PhoneImpl::OnIncomingRequest(
    const scoped_refptr<Request> &incoming_request,
    const scoped_refptr<Dialog> &dialog) {
  DCHECK(GetNetworkTaskRunner()->BelongsToCurrentThread());
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
  DCHECK(GetNetworkTaskRunner()->BelongsToCurrentThread());
  if (Method::REGISTER == incoming_response->refer_to()->method()) {
    if (last_request_->id() != incoming_response->refer_to()->id()) {
      // Discard, as it was unrelated to current REGISTER request
      return;
    } else if (PHONE_STATE_REGISTERING == state_) {  // result of Register
      int response_code = incoming_response->response_code();
      if (response_code / 100 == 1) {
        // Do nothing, wait for a final response
        return;
      } else if (response_code / 100 == 2) {
        // Save the time when the registration will expire
        unsigned int expiration = GetContactExpiration(incoming_response);
        register_expires_ = base::Time::Now() +
            base::TimeDelta::FromSeconds(expiration);
        base::AutoLock lock(lock_);
        state_ = PHONE_STATE_REGISTERED;
      } else {
        base::AutoLock lock(lock_);
        state_ = PHONE_STATE_READY;
      }
      // Notify completion
      on_register_completed_.Run(
          StatusCodeToCompletionStatus(incoming_response->response_code()));
    } else if (PHONE_STATE_REGISTERED == state_) {  // after refresh register
      int response_code = incoming_response->response_code();
      if (response_code / 100 == 1) {
        // Do nothing, wait for a final response
        return;
      } else if (response_code / 100 == 2) {
        // Start the timer to refresh login again
        unsigned int expiration = GetContactExpiration(incoming_response);
        register_expires_ = base::Time::Now() +
            base::TimeDelta::FromSeconds(expiration);
        OnStartRefreshRegister();
      } else {
        {
          base::AutoLock lock(lock_);
          state_ = PHONE_STATE_READY;
        }
        // Notify the problem upwards
        on_refresh_completed_.Run(
            StatusCodeToCompletionStatus(incoming_response->response_code()));
      }
    } else if (PHONE_STATE_UNREGISTERING == state_) {  // result of Unregister
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
      on_unregister_completed_.Run(
          StatusCodeToCompletionStatus(incoming_response->response_code()));
    }
  } else if (Method::INVITE == incoming_response->refer_to()->method()
             || Method::BYE == incoming_response->refer_to()->method()) {
    CallImpl *call = RouteToCall(incoming_response->refer_to());
    if (call)
      call->OnIncomingResponse(incoming_response, dialog);
  }
}

void PhoneImpl::OnTimedOut(
    const scoped_refptr<Request> &request) {
  DCHECK(GetNetworkTaskRunner()->BelongsToCurrentThread());
  if (Method::INVITE == request->method()
      || Method::BYE == request->method()) {
    CallImpl *call = RouteToCall(request);
    if (call)
      call->OnTimedOut(request);
  } else if (Method::REGISTER == request->method()) {
    if (PHONE_STATE_REGISTERING == state_)
      on_register_completed_.Run(net::ERR_TIMED_OUT);
    else if (PHONE_STATE_REGISTERED == state_)
      on_refresh_completed_.Run(net::ERR_TIMED_OUT);
    else if (PHONE_STATE_UNREGISTERING == state_)
      on_unregister_completed_.Run(net::ERR_TIMED_OUT);
  }
}

void PhoneImpl::OnTransportError(
    const scoped_refptr<Request> &request, int error) {
  DCHECK(GetNetworkTaskRunner()->BelongsToCurrentThread());
  if (Method::INVITE == request->method()
      || Method::BYE == request->method()) {
    CallImpl *call = RouteToCall(request);
    if (call)
      call->OnTransportError(request, error);
  } else if (Method::REGISTER == request->method()) {
    if (PHONE_STATE_REGISTERING == state_)
      on_register_completed_.Run(error);
    else if (PHONE_STATE_REGISTERED == state_)
      on_refresh_completed_.Run(error);
    else if (PHONE_STATE_UNREGISTERING == state_)
      on_unregister_completed_.Run(error);
  }
}

void PhoneImpl::OnRefreshRegister() {
  DCHECK(GetNetworkTaskRunner()->BelongsToCurrentThread());
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
    Contact *contact = cast<Contact>(i);
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
  if (!settings_.registrar_server().is_empty()) {
    return settings_.registrar_server().spec();
  } else {
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

scoped_refptr<base::SingleThreadTaskRunner>
PhoneImpl::GetNetworkTaskRunner() const {
  return network_thread_.task_runner();
}

base::MessageLoop *PhoneImpl::GetNetworkMessageLoop() const {
  return network_thread_.message_loop();
}

void Phone::Initialize() {
  // Initialize the SSL libraries
  rtc::InitializeSSL();
}

scoped_refptr<Phone> Phone::Create(Delegate *delegate) {
  return new PhoneImpl(delegate);
}

}  // namespace phone
}  // namespace sippet
