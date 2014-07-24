// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdio>
#include <iostream>

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "net/socket/client_socket_factory.h"
#include "net/url_request/url_request_context_getter.h"
#include "sippet/transport/channel_factory.h"
#include "sippet/transport/chrome/chrome_channel_factory.h"
#include "sippet/transport/network_layer.h"
#include "sippet/ua/ua_user_agent.h"
#include "sippet/examples/login/url_request_context_getter.h"

static void PrintUsage() {
  printf("login --username=username\n"
         "      --password=password\n"
         "      [--tcp|--udp|--tls|--ws|--wss]\n");
}

int main(int argc, char **argv) {
  base::AtExitManager at_exit_manager;
  base::MessageLoopForIO message_loop;

  // Process command line
  CommandLine::Init(argc, argv);
  CommandLine* command_line = CommandLine::ForCurrentProcess();

  logging::LoggingSettings settings;
  settings.logging_dest = logging::LOG_TO_ALL;
  settings.log_file = FILE_PATH_LITERAL("login.log");
  if (!logging::InitLogging(settings)) {
    printf("Error: could not initialize logging. Exiting.\n");
    return -1;
  }

  if (command_line->GetSwitches().empty() ||
      command_line->HasSwitch("help")) {
    PrintUsage();
    return -1;
  }

  std::string username;
  if (command_line->HasSwitch("username")) {
    username = command_line->GetSwitchValueASCII("username");
  } else {
    PrintUsage();
    return -1;
  }

  std::string password;
  if (command_line->HasSwitch("password")) {
    password = command_line->GetSwitchValueASCII("password");
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
	  { "tcp", "sip:localhost;transport=TCP" },
	  { "tls", "sips:localhost" },
	  { "ws", "sip:localhost;transport=WS" },
	  { "wss", "sips:localhost;transport=WS" },
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

  net::SSLConfig ssl_config;

  net::ClientSocketFactory *client_socket_factory =
      net::ClientSocketFactory::GetDefaultFactory();

  scoped_refptr<sippet::ua::UserAgent> user_agent(new sippet::ua::UserAgent);
  scoped_refptr<sippet::NetworkLayer> network_layer;
  network_layer = new sippet::NetworkLayer(user_agent.get());

  // Register the channel factory
  scoped_ptr<sippet::ChromeChannelFactory> channel_factory(
      new sippet::ChromeChannelFactory(client_socket_factory,
                                       request_context_getter, ssl_config));
  network_layer->RegisterChannelFactory(sippet::Protocol::TCP, channel_factory.get());
  network_layer->RegisterChannelFactory(sippet::Protocol::TLS, channel_factory.get());

  /*
  // Set the network layer and start the login process
  login_handler->SetNetworkLayer(network_layer.get());
  login_handler->Login(username, password, username, SipURI(registrar_uri));
  */

  message_loop.Run();
  return 0;
}
