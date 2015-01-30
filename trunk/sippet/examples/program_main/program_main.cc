// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/examples/program_main/program_main.h"

#include <iostream>
#include "base/i18n/icu_util.h"
#include "base/files/file_path.h"
#include "net/socket/client_socket_factory.h"
#include "sippet/ua/dialog_controller.h"

ProgramMain::ProgramMain(int argc, char **argv) {
  // Initialize ICU library
  if (!base::i18n::InitializeICU()) {
    std::cerr << "Couldn't open ICU library, exiting...\n";
    exit(1);
  }

  base::CommandLine::Init(argc, argv);
}

ProgramMain::~ProgramMain() {
}

base::CommandLine* ProgramMain::command_line() {
  return base::CommandLine::ForCurrentProcess();
}

void ProgramMain::set_username(const base::string16& username) {
  username_ = username;
}

void ProgramMain::set_password(const base::string16& password) {
  password_ = password;
}

bool ProgramMain::Init() {
  logging::LoggingSettings settings;
  settings.logging_dest = logging::LOG_TO_ALL;
  settings.log_file = FILE_PATH_LITERAL("login.log");
  if (!logging::InitLogging(settings)) {
    std::cout << "Error: could not initialize logging. Exiting.\n";
    return false;
  }
  logging::SetMinLogLevel(-10);

  request_context_getter_ =
      new URLRequestContextGetter(message_loop_.message_loop_proxy());

  net::ClientSocketFactory *client_socket_factory =
      net::ClientSocketFactory::GetDefaultFactory();
  host_resolver_ = net::HostResolver::CreateDefaultResolver(NULL);
  scoped_ptr<sippet::AuthHandlerRegistryFactory> auth_handler_factory(
      sippet::AuthHandlerFactory::CreateDefault(host_resolver_.get()));
  auth_handler_factory_ =
      auth_handler_factory.Pass();

  static_password_handler_factory_.reset(new StaticPasswordHandler::Factory(
      username_, password_));
  user_agent_ = new sippet::ua::UserAgent(auth_handler_factory_.get(),
      static_password_handler_factory_.get(),
      sippet::DialogController::GetDefaultDialogController(),
      net_log_);
  
  sippet::NetworkSettings network_settings;
  ssl_cert_error_handler_factory_.reset(new DumpSSLCertError::Factory(true));
  network_settings.set_ssl_cert_error_handler_factory(
      ssl_cert_error_handler_factory_.get());

  network_layer_ = new sippet::NetworkLayer(user_agent_.get(),
      network_settings);

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
  return true;
}

void ProgramMain::AppendHandler(sippet::ua::UserAgent::Delegate *delegate) {
  user_agent_->AppendHandler(delegate);
}

sippet::ua::UserAgent *ProgramMain::user_agent() {
  return user_agent_.get();
}

sippet::NetworkLayer* ProgramMain::network_layer() {
  return network_layer_.get();
}

void ProgramMain::Run() {
  message_loop_.Run();
}

