// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/test/standalone_test_server/standalone_test_server.h"

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/message_loop/message_loop.h"
#include "sippet/uri/uri.h"

using namespace sippet;

static void PrintUsage() {
  printf("standalone_test_server {--tcp|--udp|--tls} [--port=nnn]\n");
}

int main(int argc, char *argv[]) {
  base::AtExitManager at_exit_manager;
  base::MessageLoopForIO message_loop;

  // Process command line
  CommandLine::Init(argc, argv);
  CommandLine* command_line = CommandLine::ForCurrentProcess();

  logging::LoggingSettings settings;
  settings.logging_dest = logging::LOG_TO_ALL;
  settings.log_file = FILE_PATH_LITERAL("standalone_test_server.log");
  if (!logging::InitLogging(settings)) {
    printf("Error: could not initialize logging. Exiting.\n");
    return -1;
  }
  logging::SetMinLogLevel(-10);

  if (command_line->GetSwitches().empty() ||
      command_line->HasSwitch("help")) {
    PrintUsage();
    return -1;
  }

  int port = -1;
  if (command_line->HasSwitch("port")) {
    if (!base::StringToInt(
        command_line->GetSwitchValueASCII("port"), &port)) {
      PrintUsage();
      return -1;
    }
  }

  Protocol protocol;
  StandaloneTestServer::SSLOptions ssl_options;
  if (command_line->HasSwitch("udp")) {
    protocol = Protocol::UDP;
    if (port == -1) port = 5060;
  } else if (command_line->HasSwitch("tcp")) {
    protocol = Protocol::TCP;
    if (port == -1) port = 5060;
  } else if (command_line->HasSwitch("tls")) {
    protocol = Protocol::TLS;
    if (port == -1) port = 5061;
  }

  scoped_ptr<StandaloneTestServer> server(
    new StandaloneTestServer(protocol, ssl_options, port));

  if (!server->InitializeAndWaitUntilReady()) {
    printf("Error: could not initialize server. Exiting.\n");
    return -1;
  }

  printf("--\n"
         "Base URI: %s\n"
         "Server started. Press Ctrl+C when done...\n",
         server->base_uri().spec().c_str());

  message_loop.Run();

  if (!server->ShutdownAndWaitUntilComplete()) {
    printf("Error: could not shutdown server.\n");
    return -1;
  }

  return 0;
}

