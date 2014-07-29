// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/test/standalone_test_server/standalone_test_server.h"

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"

using namespace sippet;

static void PrintUsage() {
  printf("standalone_test_server {--tcp|--udp|--tls} [--port nnn]\n");
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

  if (command_line->GetSwitches().empty() ||
      command_line->HasSwitch("help")) {
    PrintUsage();
    return -1;
  }

  std::string port;
  if (command_line->HasSwitch("port"))
    port = command_line->GetSwitchValueASCII("port");

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

  scoped_ptr<StandaloneTestServer> server;
  if (command_line->HasSwitch("udp")) {
    server.reset(new StandaloneTestServer(Protocol::UDP));
  } else if (command_line->HasSwitch("tcp")) {
    server.reset(new StandaloneTestServer(Protocol::UDP));
  } else if (command_line->HasSwitch("tls")) {
    server.reset(new StandaloneTestServer(Protocol::TLS));
  }

  if (!server->InitializeAndWaitUntilReady()) {
    printf("Error: could not initialize server. Exiting.\n");
    return -1;
  }

  printf("Server started. Press Ctrl+C when done...");
  message_loop.Run();

  if (!server->ShutdownAndWaitUntilComplete()) {
    printf("Error: could not shutdown server.\n");
    return -1;
  }

  return 0;
}

