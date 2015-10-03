// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include "net/base/net_errors.h"
#include "base/strings/utf_string_conversions.h"
#include "base/i18n/icu_string_conversions.h"
#include "sippet/examples/program_main/program_main.h"

static void PrintUsage() {
  std::cout << "sippet_examples_login"
            << " --username=user"
            << " --password=pass"
            << " \\" << std::endl
            << "    [--tcp|--udp|--tls|--ws|--wss]\n";
}

class UserAgentHandler
  : public sippet::ua::UserAgent::Delegate {
 public:
  explicit UserAgentHandler(sippet::NetworkLayer *network_layer)
      : network_layer_(network_layer) {
  }

  void OnChannelConnected(const sippet::EndPoint &destination,
                          int err) override {
    std::cout << "Channel " << destination.ToString()
              << " connected, status = " << err << "\n";
  }

  void OnChannelClosed(const sippet::EndPoint &destination) override {
    std::cout << "Channel " << destination.ToString()
              << " closed.\n";

    base::MessageLoop::current()->Quit();
  }

  void OnIncomingRequest(
      const scoped_refptr<sippet::Request> &incoming_request,
      const scoped_refptr<sippet::Dialog> &dialog) override {
    std::cout << "Incoming request "
              << incoming_request->method().str()
              << "\n";
    if (dialog)
      std::cout << "-- Using dialog " << dialog->id() << "\n";
  }

  void OnIncomingResponse(
      const scoped_refptr<sippet::Response> &incoming_response,
      const scoped_refptr<sippet::Dialog> &dialog) override {
    std::cout << "Incoming response "
              << incoming_response->response_code()
              << " "
              << incoming_response->reason_phrase()
              << "\n";
    if (dialog)
      std::cout << "-- Using dialog " << dialog->id() << "\n";

    base::MessageLoop::current()->Quit();
  }

  void OnTimedOut(
      const scoped_refptr<sippet::Request> &request,
      const scoped_refptr<sippet::Dialog> &dialog) override {
    std::cout << "Timed out sending request "
              << request->method().str()
              << "\n";
    if (dialog)
      std::cout << "-- Using dialog " << dialog->id() << "\n";

    base::MessageLoop::current()->Quit();
  }

  void OnTransportError(
      const scoped_refptr<sippet::Request> &request, int error,
      const scoped_refptr<sippet::Dialog> &dialog) override {
    std::cout << "Transport error sending request "
              << request->method().str()
              << "\n";
    if (dialog)
      std::cout << "-- Using dialog " << dialog->id() << "\n";

    base::MessageLoop::current()->Quit();
  }

 private:
  sippet::NetworkLayer *network_layer_;
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
  ProgramMain program_main(argc, argv);
  base::CommandLine* command_line = program_main.command_line();

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

  std::string server("localhost");
  if (command_line->HasSwitch("server")) {
    server = command_line->GetSwitchValueASCII("server");
  }

  std::string registrar_uri;
  struct {
    const char *cmd_switch_;
    const char *registrar_uri_;
  } args[] = {
    { "udp", "sip:%s" },
    { "tcp", "sip:%s;transport=tcp" },
    { "tls", "sips:%s" },
    { "ws", "sip:%s;transport=ws" },
    { "wss", "sips:%s;transport=ws" },
  };

  for (int i = 0; i < arraysize(args); i++) {
    if (command_line->HasSwitch(args[i].cmd_switch_)) {
      registrar_uri =
          base::StringPrintf(args[i].registrar_uri_, server.c_str());
      break;
    }
  }
  if (registrar_uri.empty()) {
    // Defaults to UDP
    registrar_uri =
        base::StringPrintf(args[0].registrar_uri_, server.c_str());
  }

  program_main.set_username(base::ASCIIToUTF16(username));
  program_main.set_password(base::ASCIIToUTF16(password));
  if (!program_main.Init()) {
    return -1;
  }

  scoped_ptr<UserAgentHandler> handler(
      new UserAgentHandler(program_main.network_layer()));
  program_main.AppendHandler(handler.get());

  std::string from("sip:" + username + "@" + server);
  std::string to("sip:" + username + "@" + server);
  scoped_refptr<sippet::Request> request =
      program_main.user_agent()->CreateRequest(
          sippet::Method::REGISTER,
          GURL(registrar_uri),
          GURL(from),
          GURL(to));
  program_main.user_agent()->Send(request, base::Bind(&RequestSent));

  program_main.Run();
  return 0;
}
