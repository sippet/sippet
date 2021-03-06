// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/timer/timer.h"
#include "base/strings/utf_string_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_split.h"
#include "base/i18n/icu_util.h"
#include "base/message_loop/message_loop.h"
#include "base/threading/thread_checker.h"
#include "net/base/net_errors.h"
#include "sippet/phone/phone.h"
#include "sippet/phone/completion_status.h"

using sippet::phone::Settings;
using sippet::phone::Phone;
using sippet::phone::Call;

#if defined(OSX)
#include "base/mac/scoped_nsautorelease_pool.h"
#endif  // defined(OS_MACOSX)

static void PrintUsage() {
  std::cout << "sippet_examples_login"
            << " --username=user"
            << " --password=pass"
            << " --dial=phone-number"
            << " --route=route-addresses"
            << " \\" << std::endl
            << "    [--tcp|--udp|--tls|--ws|--wss]\n";
}

class Conductor :
    public Phone::Delegate {
 public:
  Conductor(const Settings& settings, const std::string& destination,
    base::MessageLoop* message_loop) :
    settings_(settings), destination_(destination),
    phone_(Phone::Create(this)), message_loop_(message_loop) {
  }
  virtual ~Conductor() {
    DCHECK(thread_checker_.CalledOnValidThread());
  }

  bool Start() {
    DCHECK(thread_checker_.CalledOnValidThread());
    if (!phone_->Init(settings_)) {
      LOG(ERROR) << "Phone::Init error";
      return false;
    }
    next_state_ = STATE_LOGIN;
    OnIOComplete(net::OK);
    return true;
  }

 private:
  Settings settings_;
  std::string destination_;
  scoped_refptr<Phone> phone_;
  base::MessageLoop* message_loop_;
  scoped_refptr<Call> call_;
  base::OneShotTimer<Conductor> call_timeout_;
  base::ThreadChecker thread_checker_;

  void OnNetworkError(int error_code) {
    LOG(ERROR) << "Network error: " << error_code
      << ", " << net::ErrorToString(error_code);
    message_loop_->PostTask(FROM_HERE,
        base::Bind(&Conductor::OnAbort,
            base::Unretained(this)));
  }

  void OnRegisterCompleted(int error) {
    if (sippet::OK == error) {
      message_loop_->PostTask(FROM_HERE,
          base::Bind(&Conductor::OnIOComplete,
              base::Unretained(this), net::OK));
    } else {
      OnError(error);
    }
  }

  void OnRefreshError(int error) {
    // do nothing
  }

  void OnUnregisterCompleted(int error) {
    // do nothing
  }

  void OnIncomingCall(const scoped_refptr<Call>& call) override {
    LOG(ERROR) << "Unexpected incoming call";
    message_loop_->PostTask(FROM_HERE,
        base::Bind(&Conductor::OnAbort,
            base::Unretained(this)));
  }

  void OnCallCompleted(int error) {
    if (sippet::ERR_SIP_RINGING == error) {
      OnRinging();
    } else if (sippet::OK == error) {
      OnEstablished();
    } else if (sippet::IsHangupCause(error)) {
      OnHungUp();
    } else {
      OnError(error);
    }
  }

  void OnHangUpCompleted(int error) {
    // Do nothing...
  }

  void OnError(int error) {
    LOG(ERROR) << "Error: "
               << sippet::ErrorToShortString(error);
    message_loop_->PostTask(FROM_HERE,
        base::Bind(&Conductor::OnIOComplete,
            base::Unretained(this), net::ERR_UNEXPECTED));
  }

  void OnRinging() {
    LOG(INFO) << "Ringing...";
  }

  void OnEstablished() {
    LOG(INFO) << "Established...";
  }

  void OnHungUp() {
    LOG(ERROR) << "Hung up call";
    message_loop_->PostTask(FROM_HERE,
        base::Bind(&Conductor::OnIOComplete,
            base::Unretained(this), net::ERR_UNEXPECTED));
  }

  enum State {
    STATE_LOGIN,
    STATE_LOGIN_COMPLETE,
    STATE_MAKE_CALL,
    STATE_MAKE_CALL_COMPLETE,
    STATE_CALL_TIMER,
    STATE_CALL_TIMER_COMPLETE,
    STATE_HANGUP,
    STATE_HANGUP_COMPLETE,
    STATE_LOGOUT,
    STATE_LOGOUT_COMPLETE,
    STATE_NONE
  };

  State next_state_;

  void OnIOComplete(int result) {
    DCHECK_NE(STATE_NONE, next_state_);
    DCHECK(thread_checker_.CalledOnValidThread());
    int rv = DoLoop(result);
    if (rv != net::ERR_IO_PENDING) {
      base::MessageLoop::current()->Quit();
    }
  }

  int DoLoop(int last_io_result) {
    DCHECK_NE(next_state_, STATE_NONE);
    int rv = last_io_result;
    do {
      State state = next_state_;
      next_state_ = STATE_NONE;
      switch (state) {
      case STATE_LOGIN:
        DCHECK_EQ(net::OK, rv);
        rv = DoLogin();
        break;
      case STATE_LOGIN_COMPLETE:
        rv = DoLoginComplete(rv);
        break;
      case STATE_MAKE_CALL:
        DCHECK_EQ(net::OK, rv);
        rv = DoMakeCall();
        break;
      case STATE_MAKE_CALL_COMPLETE:
        rv = DoMakeCallComplete(rv);
        break;
      case STATE_CALL_TIMER:
        DCHECK_EQ(net::OK, rv);
        rv = DoCallTimer();
        break;
      case STATE_CALL_TIMER_COMPLETE:
        rv = DoCallTimerComplete(rv);
        break;
      case STATE_HANGUP:
        DCHECK_EQ(net::OK, rv);
        rv = DoHangup();
        break;
      case STATE_HANGUP_COMPLETE:
        rv = DoHangupComplete(rv);
        break;
      case STATE_LOGOUT:
        DCHECK_EQ(net::OK, rv);
        rv = DoLogout();
        break;
      case STATE_LOGOUT_COMPLETE:
        rv = DoLogoutComplete(rv);
        break;
      default:
        NOTREACHED() << "bad state";
        rv = net::ERR_UNEXPECTED;
        break;
      }
    } while (rv != net::ERR_IO_PENDING && next_state_ != STATE_NONE);
    return rv;
  }

  int DoLogin() {
    next_state_ = STATE_LOGIN_COMPLETE;
    phone_->Register(
       base::Bind(&Conductor::OnRegisterCompleted,
           base::Unretained(this)));
    return net::ERR_IO_PENDING;
  }

  int DoLoginComplete(int result) {
    next_state_ = STATE_MAKE_CALL;
    return result;
  }

  int DoMakeCall() {
    next_state_ = STATE_MAKE_CALL_COMPLETE;
    call_ = phone_->MakeCall(destination_,
        base::Bind(&Conductor::OnCallCompleted,
            base::Unretained(this)));
    return net::ERR_IO_PENDING;
  }

  int DoMakeCallComplete(int result) {
    if (result == net::OK)
      next_state_ = STATE_CALL_TIMER;
    else
      next_state_ = STATE_LOGOUT;
    return net::OK;
  }

  int DoCallTimer() {
    next_state_ = STATE_CALL_TIMER_COMPLETE;
    call_timeout_.Start(FROM_HERE,
        base::TimeDelta::FromSeconds(3600),
        base::Bind(&Conductor::OnIOComplete,
            base::Unretained(this), net::OK));
    return net::ERR_IO_PENDING;
  }

  int DoCallTimerComplete(int result) {
    if (result == net::OK) {
      next_state_ = STATE_HANGUP;
    } else {
      next_state_ = STATE_LOGOUT;
      call_timeout_.Stop();
    }
    return net::OK;
  }

  int DoHangup() {
    next_state_ = STATE_HANGUP_COMPLETE;
    call_->HangUp(base::Bind(&Conductor::OnHangUpCompleted,
        base::Unretained(this)));
    return net::OK;
  }

  int DoHangupComplete(int result) {
    next_state_ = STATE_LOGOUT;
    return result;
  }

  int DoLogout() {
    next_state_ = STATE_LOGOUT_COMPLETE;
    phone_->Unregister(base::Bind(&Conductor::OnUnregisterCompleted,
        base::Unretained(this)));
    return net::OK;
  }

  int DoLogoutComplete(int result) {
    next_state_ = STATE_NONE;
    return result;
  }

  void OnAbort() {
    base::MessageLoop::current()->Quit();
  }
};

int main(int argc, char **argv) {
#if defined(OSX)
  // Needed so we don't leak objects when threads are created.
  base::mac::ScopedNSAutoreleasePool pool;
#endif
  base::AtExitManager at_exit_manager;

  Phone::Initialize();

  if (!base::i18n::InitializeICU()) {
    std::cerr << "Couldn't open ICU library, exiting...\n";
    exit(1);
  }

  base::CommandLine::Init(argc, argv);
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();

  logging::LoggingSettings log_settings;
  log_settings.logging_dest = logging::LOG_TO_ALL;
  log_settings.log_file = FILE_PATH_LITERAL("login.log");
  if (!logging::InitLogging(log_settings)) {
    std::cout << "Error: could not initialize logging. Exiting.\n";
    return false;
  }
  logging::SetMinLogLevel(-10);

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

  std::string destination;
  if (command_line->HasSwitch("dial")) {
    destination = command_line->GetSwitchValueASCII("dial");
  } else {
    PrintUsage();
    return -1;
  }

  std::string server = "localhost";
  if (command_line->HasSwitch("server")) {
    server = command_line->GetSwitchValueASCII("server");
  }

  std::string route_set;
  if (command_line->HasSwitch("route")) {
    route_set = command_line->GetSwitchValueASCII("route");
  }

  struct {
    const char *cmd_switch_;
    const char *registrar_uri_;
  } args[] = {
    { "udp", "sip:%s@%s" },
    { "tcp", "sip:%s@%s;transport=tcp" },
    { "tls", "sips:%s@%s" },
    { "ws", "sip:%s@%s;transport=ws" },
    { "wss", "sips:%s@%s;transport=ws" },
  };

  std::string uri;
  for (int i = 0; i < arraysize(args); i++) {
    if (command_line->HasSwitch(args[i].cmd_switch_)) {
      uri = base::StringPrintf(args[i].registrar_uri_,
          username.c_str(), server.c_str());
      break;
    }
  }
  if (uri.empty()) {
    uri = base::StringPrintf(args[0].registrar_uri_,
      username.c_str(), server.c_str());  // Defaults to UDP
  }

  base::MessageLoopForIO message_loop;

  Settings settings;
  settings.set_disable_encryption(true);
  settings.set_disable_sctp_data_channels(true);
  settings.set_uri(GURL(uri));
  settings.set_password(password);
  if (route_set.size() > 0) {
    std::vector<std::string> set;
    base::SplitString(route_set, ',', &set);
    for (std::vector<std::string>::iterator i = set.begin(), ie = set.end();
         i != ie; i++) {
      settings.route_set().push_back(GURL(*i));
    }
  }

  Conductor conductor(settings, destination, &message_loop);
  if (!conductor.Start())
    return -1;

  message_loop.Run();
  return 0;
}
