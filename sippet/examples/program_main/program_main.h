// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_EXAMPLES_PROGRAM_MAIN_PROGRAM_MAIN_H_
#define SIPPET_EXAMPLES_PROGRAM_MAIN_PROGRAM_MAIN_H_

#include "base/at_exit.h"
#include "base/command_line.h"
#include "net/dns/host_resolver.h"
#include "net/log/net_log.h"
#include "sippet/ua/ua_user_agent.h"
#include "sippet/ua/auth_handler_factory.h"
#include "sippet/transport/chrome/chrome_channel_factory.h"
#include "sippet/examples/common/url_request_context_getter.h"
#include "sippet/examples/common/static_password_handler.h"
#include "sippet/examples/common/dump_ssl_cert_error.h"

class ProgramMain {
 public:
  ProgramMain(int argc, char **argv);
  virtual ~ProgramMain();
  base::CommandLine* command_line();
  void set_username(const base::string16& username);
  void set_password(const base::string16& password);
  bool Init();
  void AddObserver(sippet::ua::UserAgent::Observer *observer);
  void RemoveObserver(sippet::ua::UserAgent::Observer *observer);
  sippet::ua::UserAgent *user_agent();
  sippet::NetworkLayer* network_layer();
  void Run();

 private:
  base::string16 username_;
  base::string16 password_;
  base::AtExitManager at_exit_manager_;
  base::MessageLoopForIO message_loop_;
  scoped_refptr<net::URLRequestContextGetter> request_context_getter_;
  std::unique_ptr<net::HostResolver> host_resolver_;
  std::unique_ptr<sippet::AuthHandlerFactory> auth_handler_factory_;
  net::BoundNetLog net_log_;
  std::unique_ptr<StaticPasswordHandler::Factory> static_password_handler_factory_;
  std::unique_ptr<sippet::ua::UserAgent> user_agent_;
  std::unique_ptr<DumpSSLCertError::Factory> ssl_cert_error_handler_factory_;
  std::unique_ptr<sippet::NetworkLayer> network_layer_;
  std::unique_ptr<sippet::ChromeChannelFactory> channel_factory_;
};

#endif // SIPPET_EXAMPLES_PROGRAM_MAIN_PROGRAM_MAIN_H_

