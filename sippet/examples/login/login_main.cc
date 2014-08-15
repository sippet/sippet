// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdio>
#include <iostream>

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/values.h"
#include "base/file_util.h"
#include "base/bind.h"
#include "base/path_service.h"
#include "base/message_loop/message_loop.h"
#include "base/i18n/icu_util.h"
#include "base/i18n/time_formatting.h"
#include "base/i18n/icu_string_conversions.h"
#include "net/base/net_errors.h"
#include "net/socket/client_socket_factory.h"
#include "net/url_request/url_request_context_getter.h"
#include "net/dns/host_resolver.h"
#include "net/base/net_log_logger.h"
#include "net/ssl/ssl_info.h"
#include "sippet/transport/channel_factory.h"
#include "sippet/transport/chrome/chrome_channel_factory.h"
#include "sippet/transport/network_layer.h"
#include "sippet/ua/ua_user_agent.h"
#include "sippet/ua/auth_handler_factory.h"
#include "sippet/examples/login/url_request_context_getter.h"

using namespace sippet;

static void PrintUsage() {
  std::cout << "login --username=username\n"
            << "      --password=password\n"
            << "      [--tcp|--udp|--tls|--ws|--wss]\n";
}

class UserAgentHandler
  : public ua::UserAgent::Delegate {
 public:
  UserAgentHandler(NetworkLayer *network_layer)
      : network_layer_(network_layer) {
  }

  virtual void OnChannelConnected(const EndPoint &destination,
                                  int err) OVERRIDE {
    std::cout << "Channel " << destination.ToString()
              << " connected, status = " << err << "\n";
  }

  virtual void OnChannelClosed(const EndPoint &destination) OVERRIDE {
    std::cout << "Channel " << destination.ToString()
              << " closed.\n";

    base::MessageLoop::current()->Quit();
  }

  virtual void OnSSLCertificateError(const EndPoint &destination,
                                     const net::SSLInfo &ssl_info,
                                     bool fatal) OVERRIDE {
    std::cout << "SSL certificate error for channel "
              << destination.ToString()
              << "\n";

    std::cout << "-- gravity: "
              << (fatal ? "fatal" : "non-fatal")
              << "\n";

    std::cout << "-- issued by "
              << (ssl_info.is_issued_by_known_root ? "known" : "unknown")
              << " root\n";

    if (ssl_info.cert) {
      std::cout << "-- issuer: "
                << ssl_info.cert->issuer().GetDisplayName()
                << "\n";

      std::cout << "-- subject: "
                << ssl_info.cert->subject().GetDisplayName()
                << "\n";

      size_t size;
      net::X509Certificate::PublicKeyType public_key_type;
      ssl_info.cert->GetPublicKeyInfo(ssl_info.cert->os_cert_handle(),
          &size, &public_key_type);
      const char *key_type;
      if (public_key_type == net::X509Certificate::kPublicKeyTypeDH) {
        key_type = "DH";
      } else if (public_key_type == net::X509Certificate::kPublicKeyTypeDSA) {
        key_type = "DSA";
      } else if (public_key_type == net::X509Certificate::kPublicKeyTypeECDH) {
        key_type = "ECDH";
      } else if (public_key_type == net::X509Certificate::kPublicKeyTypeECDSA) {
        key_type = "ECDSA";
      } else if (public_key_type == net::X509Certificate::kPublicKeyTypeRSA) {
        key_type = "RSA";
      } else {
        key_type = "Unknown";
      }
      std::cout << "-- public key: "
                << key_type
                << " (" << size << " bits)"
                << "\n";

      std::vector<std::string> dns_names;
      ssl_info.cert->GetDNSNames(&dns_names);
      std::cout << "-- DNS names: ";
      for (std::vector<std::string>::iterator i = dns_names.begin(),
           ie = dns_names.end(); i != ie; i++) {
        if (i != dns_names.begin())
          std::cout << ", ";
        std::cout << *i;
      }
      std::cout << "\n";

      if (!ssl_info.cert->valid_start().is_null()
          && !ssl_info.cert->valid_expiry().is_null()) {
        base::string16 start =
            base::TimeFormatShortDateAndTime(ssl_info.cert->valid_start());
        base::string16 end =
            base::TimeFormatShortDateAndTime(ssl_info.cert->valid_expiry());
        std::cout << "-- Valid from " << start << " to " << end << "\n";
      }
    }

    std::cout << "Accepting the certificate\n";
    network_layer_->ReconnectIgnoringLastError(destination);
  }

  virtual void OnIncomingRequest(
      const scoped_refptr<Request> &incoming_request,
      const scoped_refptr<Dialog> &dialog) OVERRIDE {
    std::cout << "Incoming request "
              << incoming_request->method().str()
              << "\n";
    if (dialog)
      std::cout << "-- Using dialog " << dialog->id() << "\n";
  }

  virtual void OnIncomingResponse(
      const scoped_refptr<Response> &incoming_response,
      const scoped_refptr<Dialog> &dialog) OVERRIDE {
    std::cout << "Incoming response "
              << incoming_response->response_code()
              << " "
              << incoming_response->reason_phrase()
              << "\n";
    if (dialog)
      std::cout << "-- Using dialog " << dialog->id() << "\n";

    base::MessageLoop::current()->Quit();
  }

  virtual void OnTimedOut(
      const scoped_refptr<Request> &request,
      const scoped_refptr<Dialog> &dialog) OVERRIDE {
    std::cout << "Timed out sending request "
              << request->method().str()
              << "\n";
    if (dialog)
      std::cout << "-- Using dialog " << dialog->id() << "\n";

    base::MessageLoop::current()->Quit();
  }

  virtual void OnTransportError(
      const scoped_refptr<Request> &request, int error,
      const scoped_refptr<Dialog> &dialog) OVERRIDE {
    std::cout << "Transport error sending request "
              << request->method().str()
              << "\n";
    if (dialog)
      std::cout << "-- Using dialog " << dialog->id() << "\n";

    base::MessageLoop::current()->Quit();
  }

 private:
  NetworkLayer *network_layer_;
};

class StaticPasswordHandler : public PasswordHandler {
 public:
  class Factory : public PasswordHandler::Factory {
   public:
    Factory(const string16 &username, const string16 &password)
      : username_(username), password_(password) {
    }
    virtual ~Factory() {}

    virtual scoped_ptr<PasswordHandler>
        CreatePasswordHandler() OVERRIDE {
      scoped_ptr<PasswordHandler> password_handler(
        new StaticPasswordHandler(username_, password_));
      return password_handler.Pass();
    }

   private:
    string16 username_;
    string16 password_;
  };

  StaticPasswordHandler(const string16 &username, const string16 &password)
    : username_(username), password_(password) {
  }

  virtual ~StaticPasswordHandler() {}

  virtual int GetCredentials(
      const net::AuthChallengeInfo* auth_info,
      string16 *username,
      string16 *password,
      const net::CompletionCallback& callback) OVERRIDE {
    *username = username_;
    *password = password_;
    return net::OK;
  }

 private:
  string16 username_;
  string16 password_;
};

class UserSSLCertErrorHandler : public SSLCertErrorHandler {
 public:
  class Factory : public SSLCertErrorHandler::Factory {
   public:
    Factory() {}
    virtual ~Factory() {}

    virtual scoped_ptr<SSLCertErrorHandler>
         CreateSSLCertificateErrorHandler() OVERRIDE {
      scoped_ptr<SSLCertErrorHandler> ssl_cert_error_handler(
        new UserSSLCertErrorHandler);
      return ssl_cert_error_handler.Pass();
    }
  };

  UserSSLCertErrorHandler() {}
  virtual ~UserSSLCertErrorHandler() {}

  virtual int GetUserApproval(
      const EndPoint &destination,
      const net::SSLInfo &ssl_info,
      bool *is_accepted,
      const net::CompletionCallback& callback) OVERRIDE {
    *is_accepted = true;
    return net::OK;
  }

  virtual int GetClientCert(
      const EndPoint &destination,
      const net::SSLInfo &ssl_info,
      scoped_refptr<net::X509Certificate> *client_cert,
      const net::CompletionCallback& callback) OVERRIDE {
    return net::ERR_FAILED;
  }
};

void RequestSent(int error) {
  std::cout << ">> REGISTER completed";
  if (error != net::OK) {
    std::cout << ", error = " << error
              << " (" << net::ErrorToString(error) << ")";
  }
  std::cout << "\n";
}

int main(int argc, char **argv) {
  base::AtExitManager at_exit_manager;
  base::MessageLoopForIO message_loop;

  // Initialize ICU library
  if (!base::i18n::InitializeICU()) {
    std::cerr << "Couldn't open ICU library, exiting...\n";
    return -1;
  }

  // Process command line
  CommandLine::Init(argc, argv);
  CommandLine* command_line = CommandLine::ForCurrentProcess();

  logging::LoggingSettings settings;
  settings.logging_dest = logging::LOG_TO_ALL;
  settings.log_file = FILE_PATH_LITERAL("login.log");
  if (!logging::InitLogging(settings)) {
    std::cout << "Error: could not initialize logging. Exiting.\n";
    return -1;
  }
  logging::SetMinLogLevel(-10);

  if (command_line->GetSwitches().empty() ||
      command_line->HasSwitch("help")) {
    PrintUsage();
    return -1;
  }

  string16 username;
  if (command_line->HasSwitch("username")) {
    base::CodepageToUTF16(command_line->GetSwitchValueASCII("username"),
        NULL, base::OnStringConversionError::FAIL, &username);
  } else {
    PrintUsage();
    return -1;
  }

  string16 password;
  if (command_line->HasSwitch("password")) {
    base::CodepageToUTF16(command_line->GetSwitchValueASCII("password"),
        NULL, base::OnStringConversionError::FAIL, &password);
  } else {
    PrintUsage();
    return -1;
  }

  std::string registrar_uri;
  struct {
    const char *cmd_switch_;
  	const char *registrar_uri_;
  } args[] = {
	  { "udp", "sip:localhost" },
	  { "tcp", "sip:localhost;transport=tcp" },
	  { "tls", "sips:localhost" },
	  { "ws", "sip:localhost;transport=ws" },
	  { "wss", "sips:localhost;transport=ws" },
  };

  for (int i = 0; i < arraysize(args); i++) {
	  if (command_line->HasSwitch(args[i].cmd_switch_)) {
      registrar_uri = args[i].registrar_uri_;
      break;
    }
  }
  if (registrar_uri.empty())
  	registrar_uri = args[0].registrar_uri_; // Defaults to UDP

  scoped_refptr<net::URLRequestContextGetter> request_context_getter(
      new URLRequestContextGetter(message_loop.message_loop_proxy()));

  net::ClientSocketFactory *client_socket_factory =
      net::ClientSocketFactory::GetDefaultFactory();
  scoped_ptr<net::HostResolver> host_resolver(
      net::HostResolver::CreateDefaultResolver(NULL));
  scoped_ptr<AuthHandlerFactory> auth_handler_factory(
      AuthHandlerFactory::CreateDefault(host_resolver.get()));

  net::BoundNetLog net_log;
  StaticPasswordHandler::Factory static_password_handler_factory(
      username, password);
  UserSSLCertErrorHandler::Factory ssl_cert_error_handler_factory;
  scoped_refptr<ua::UserAgent> user_agent(
      new ua::UserAgent(auth_handler_factory.get(),
          &static_password_handler_factory, &ssl_cert_error_handler_factory,
          net_log));
  scoped_refptr<NetworkLayer> network_layer;
  network_layer = new NetworkLayer(user_agent.get());

  // Register the channel factory
  net::SSLConfig ssl_config;
  ssl_config.version_min = net::SSL_PROTOCOL_VERSION_TLS1;
  scoped_ptr<ChromeChannelFactory> channel_factory(
      new ChromeChannelFactory(client_socket_factory,
                               request_context_getter, ssl_config));
  network_layer->RegisterChannelFactory(Protocol::UDP, channel_factory.get());
  network_layer->RegisterChannelFactory(Protocol::TCP, channel_factory.get());
  network_layer->RegisterChannelFactory(Protocol::TLS, channel_factory.get());
  user_agent->SetNetworkLayer(network_layer.get());

  scoped_ptr<UserAgentHandler> handler(
      new UserAgentHandler(network_layer.get()));
  user_agent->AppendHandler(handler.get());

  scoped_refptr<Request> request =
      user_agent->CreateRequest(
          Method::REGISTER,
          GURL(registrar_uri),
          GURL("sip:test@localhost"),
          GURL("sip:test@localhost"));
  user_agent->Send(request, base::Bind(&RequestSent));

  message_loop.Run();
  return 0;
}
