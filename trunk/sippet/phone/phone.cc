// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/phone.h"

#include "base/bind.h"
#include "base/strings/utf_string_conversions.h"
#include "net/base/net_errors.h"
#include "net/socket/client_socket_factory.h"
#include "net/proxy/proxy_config_service.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_builder.h"
#include "sippet/ua/dialog_controller.h"

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
  ~URLRequestContextGetter() override;

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
Phone::AccountPasswordHandler::Factory::Factory() {
}

Phone::AccountPasswordHandler::Factory::~Factory() {
}

const Account &Phone::AccountPasswordHandler::Factory::account() const {
  return account_;
}

void Phone::AccountPasswordHandler::Factory::set_account(const Account &account) {
  account_ = account;
}

scoped_ptr<sippet::PasswordHandler>
    Phone::AccountPasswordHandler::Factory::CreatePasswordHandler() {
  scoped_ptr<PasswordHandler> password_handler(
    new AccountPasswordHandler(this));
  return password_handler.Pass();
}

//
// Phone::AccountPasswordHandler implementation
//
Phone::AccountPasswordHandler::AccountPasswordHandler(Factory *factory) :
  factory_(factory) {
}

Phone::AccountPasswordHandler::~AccountPasswordHandler() {
}

int Phone::AccountPasswordHandler::GetCredentials(
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
Phone::Phone(PhoneObserver *phone_observer)
  : phone_observer_(phone_observer),
    signalling_thread_("PhoneSignalling"),
    password_handler_factory_(new AccountPasswordHandler::Factory) {
  DCHECK(phone_observer);
}

Phone::~Phone() {
  signalling_thread_.message_loop()->PostTask(FROM_HERE,
    base::Bind(&Phone::OnDestroy, base::Unretained(this))
  );
}

bool Phone::Init(const Settings& settings) {
  if (!signalling_thread_.Start()) {
    return false;
  }
  signalling_thread_.message_loop()->PostTask(FROM_HERE,
    base::Bind(&Phone::OnInit, base::Unretained(this))
  );
  return true;
}

bool Phone::Login(const Account &account) {
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
    base::Bind(&Phone::OnLogin, base::Unretained(this), account)
  );
  return true;
}

scoped_refptr<Call> Phone::MakeCall(const std::string& destination) {
  if (destination.empty()) {
    DVLOG(1) << "Empty destination";
    return false;
  }
  base::AutoLock lock(lock_);
  scoped_refptr<Call> call(new Call(
      SipURI(scheme_ + ":" + destination + "@" + host_), this));
  calls_.push_back(call);
  signalling_thread_.message_loop()->PostTask(FROM_HERE,
    base::Bind(&Call::OnMakeCall, base::Unretained(call.get()))
  );
  return call;
}

void Phone::HangUpAll() {
  base::AutoLock lock(lock_);
  for (CallsVector::iterator i = calls_.begin(); i != calls_.end(); ++i) {
    i->get()->HangUp();
  }
}

void Phone::Logout() {
  signalling_thread_.message_loop()->PostTask(FROM_HERE,
    base::Bind(&Phone::OnLogout, base::Unretained(this))
  );
}

void Phone::RemoveCall(const scoped_refptr<Call>& call) {
  base::AutoLock lock(lock_);
  CallsVector::iterator i;
  for (i = calls_.begin(); i != calls_.end(); ++i) {
    if (*i == call.get())
      break;
  }
  if (calls_.end() != i)
    calls_.erase(i);
}

void Phone::OnInit() {
  base::MessageLoop *message_loop = base::MessageLoop::current();
  request_context_getter_ =
    new URLRequestContextGetter(message_loop->message_loop_proxy());

  net::ClientSocketFactory *client_socket_factory =
    net::ClientSocketFactory::GetDefaultFactory();
  host_resolver_ = net::HostResolver::CreateDefaultResolver(nullptr);
  scoped_ptr<sippet::AuthHandlerRegistryFactory> auth_handler_factory(
    sippet::AuthHandlerFactory::CreateDefault(host_resolver_.get()));
  auth_handler_factory_ =
    auth_handler_factory.Pass();

  user_agent_ = new sippet::ua::UserAgent(auth_handler_factory_.get(),
    password_handler_factory_.get(),
    DialogController::GetDefaultDialogController(),
    net_log_);

  network_layer_ = new sippet::NetworkLayer(user_agent_.get());

  // Register the channel factory
  net::SSLConfig ssl_config;
  ssl_config.version_min = net::SSL_PROTOCOL_VERSION_TLS1;
  channel_factory_.reset(
    new sippet::ChromeChannelFactory(client_socket_factory,
    request_context_getter_, ssl_config));
  network_layer_->RegisterChannelFactory(sippet::Protocol::UDP,
    channel_factory_.get());
  network_layer_->RegisterChannelFactory(sippet::Protocol::TCP,
    channel_factory_.get());
  network_layer_->RegisterChannelFactory(sippet::Protocol::TLS,
    channel_factory_.get());
  user_agent_->SetNetworkLayer(network_layer_.get());
  user_agent_->AppendHandler(this);
}

void Phone::OnLogin(const Account &account) {
  std::string registrar_uri("sip:" + host_);
  std::string from("sip:" + username_ + "@" + host_);
  std::string to("sip:" + username_ + "@" + host_);

  scoped_refptr<sippet::Request> request =
    user_agent_->CreateRequest(
        sippet::Method::REGISTER,
        GURL(registrar_uri),
        GURL(from),
        GURL(to));

  int rv = user_agent_->Send(request,
      base::Bind(&Phone::OnRequestSent, base::Unretained(this)));
  if (net::OK != rv && net::ERR_IO_PENDING != rv) {
    phone_observer_->OnNetworkError(rv);
    return;
  }

  // Wait for SIP response now
}

void Phone::OnLogout() {
  std::string registrar_uri("sip:" + host_);
  std::string from("sip:" + username_ + "@" + host_);
  std::string to("sip:" + username_ + "@" + host_);

  scoped_refptr<sippet::Request> request =
    user_agent_->CreateRequest(
        sippet::Method::REGISTER,
        GURL(registrar_uri),
        GURL(from),
        GURL(to));

  // Modifies the Contact header to include an expires=0 at the end
  // of existing parameters
  sippet::Contact* contact = request->get<sippet::Contact>();
  contact->front().set_expires(0);

  int rv = user_agent_->Send(request,
      base::Bind(&Phone::OnRequestSent, base::Unretained(this)));
  if (net::OK != rv && net::ERR_IO_PENDING != rv) {
    phone_observer_->OnNetworkError(rv);
    return;
  }

  // Wait for SIP response now
}

void Phone::OnRequestSent(int rv) {
  if (net::OK != rv) {
    phone_observer_->OnNetworkError(rv);
  }
}

void Phone::OnChannelConnected(const EndPoint &destination, int err) {
  if (net::OK != err) {
    phone_observer_->OnNetworkError(err);
  }
}

void Phone::OnChannelClosed(const EndPoint &destination) {
  // Nothing to do
}

void Phone::OnIncomingRequest(
    const scoped_refptr<Request> &incoming_request,
    const scoped_refptr<Dialog> &dialog) {
  // TODO
}

void Phone::OnIncomingResponse(
    const scoped_refptr<Response> &incoming_response,
    const scoped_refptr<Dialog> &dialog) {
  if (Method::INVITE == incoming_response->refer_to()->method()
      || Method::BYE == incoming_response->refer_to()->method()) {
    RouteToCall<const scoped_refptr<Response>&, const scoped_refptr<Dialog>&>(
        incoming_response->refer_to()->id(), &Call::OnIncomingResponse,
        incoming_response, dialog);
  }
}

void Phone::OnTimedOut(
    const scoped_refptr<Request> &request,
    const scoped_refptr<Dialog> &dialog) {
  if (Method::INVITE == request->method()
      || Method::BYE == request->method()) {
    RouteToCall<const scoped_refptr<Request>&, const scoped_refptr<Dialog>&>(
        request->id(), &Call::OnTimedOut,
        request, dialog);
  }
}

void Phone::OnTransportError(
    const scoped_refptr<Request> &request, int error,
    const scoped_refptr<Dialog> &dialog) {
  if (Method::INVITE == request->method()
      || Method::BYE == request->method()) {
    RouteToCall<const scoped_refptr<Request>&, int,
                const scoped_refptr<Dialog>&>(
      request->id(), &Call::OnTransportError,
      request, error, dialog);
  }
}

template<typename... Args>
void RouteToCall(const std::string& id,
    void(Call::*method)(Args...), Args... args) {
  base::AutoLock lock(lock_);
  CallsVector::iterator i;
  // A simple linear search will do it for a small amount of
  // simultaneous calls... But you can improve it if you want
  for (i = calls_.begin(); i != calls_.end(); ++i) {
    if (i->get()->invite()->id() == id)
      break;
  }
  if (calls_.end() != i) {
    scoped_refptr<Call> call(i->get());
    (call.get()->*method)(args);
  } else {
    DVLOG(1) << "Couldn't route callback to call id '" << id << "'";
  }
}

} // namespace sippet
} // namespace phone