// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_PHONE_H_
#define SIPPET_PHONE_PHONE_H_

#include <string>
#include <vector>

#include "base/memory/ref_counted.h"
#include "base/threading/thread.h"
#include "base/synchronization/lock.h"
#include "url/gurl.h"
#include "net/dns/host_resolver.h"
#include "net/url_request/url_request_context_getter.h"

#include "sippet/transport/network_layer.h"
#include "sippet/transport/ssl_cert_error_handler.h"
#include "sippet/transport/chrome/chrome_channel_factory.h"
#include "sippet/ua/auth_handler_factory.h"
#include "sippet/ua/password_handler.h"
#include "sippet/ua/ua_user_agent.h"
#include "sippet/phone/call.h"

namespace sippet {
namespace phone {

// Phone settings
class Settings {
 public:
  class IceServer {
   public:
    explicit IceServer(const std::string& uri) :
      uri_(uri) {
    }
    explicit IceServer(const std::string& uri,
                       const std::string& username,
                       const std::string& password) :
      uri_(uri),
      username_(username),
      password_(password) {
    }

    // URI example: stun:stun.l.google.com:19302
    const std::string &uri() const { return uri_; }

    // STUN/TURN username
    const std::string &username() const { return username_; }

    // STUN/TURN password
    const std::string &password() const { return password_; }

   private:
    std::string uri_; 
    std::string username_;
    std::string password_;
  };

  typedef std::vector<IceServer> IceServers;

  Settings() :
    disable_encryption_(false) {
  }

  // Enable/disable streaming encryption
  void set_disable_encryption(bool value) { disable_encryption_ = value; }
  bool disable_encryption() { return disable_encryption_; }

  // ICE servers list
  void AddIceServer(const IceServer& ice_server) {
    ice_servers_.push_back(ice_server);
  }
  IceServers::const_iterator ice_servers_begin() const {
    return ice_servers_.begin();
  }
  IceServers::const_iterator ice_servers_end() const {
    return ice_servers_.end();
  }

 private:
  std::vector<IceServer> ice_servers_;
  bool disable_encryption_;
};

// This class stores account data used for logging into the server.
class Account {
 public:
  Account() {}
  Account(const std::string& username,
          const std::string& password,
          const std::string& host) :
    username_(username),
    password_(password),
    host_(host) {
  }

  void set_username(const std::string &username) { username_ = username; }
  const std::string &username() const { return username_; }

  void set_password(const std::string &password) { password_ = password; }
  const std::string &password() const { return password_; }

  // Host has the following form:
  //
  //     host = scheme ":" host_part [ transport ]
  //     scheme = "sip" / "sips"
  //     host_part = hostname / ip_address
  //     transport = ";transport=" ( "UDP" / "TCP" / "WS" )
  //
  void set_host(const std::string &host) { host_ = host; }
  const std::string &host() const { return host_; }

 private:
  std::string username_;
  std::string password_;
  std::string host_;
};

// Phone Observer
class PhoneObserver {
 public:
  // Called to inform completion of the last login attempt
  virtual void OnNetworkError(int error_code) = 0;

  // Called to inform completion of the last login attempt
  virtual void OnLoginCompleted(int status_code,
                                const std::string& status_text) = 0;

  // Called on incoming calls
  virtual void OnIncomingCall(const scoped_refptr<Call>& call) = 0;

  // Called on call error
  virtual void OnCallError(int status_line,
                           const std::string& status_text,
                           const scoped_refptr<Call>& call) = 0;

  // Called when callee phone starts ringing
  virtual void OnCallRinging(const scoped_refptr<Call>& call) = 0;

  // Called when callee picks up the phone
  virtual void OnCallEstablished(const scoped_refptr<Call>& call) = 0;

  // Called when callee hangs up
  virtual void OnCallHungUp(const scoped_refptr<Call>& call) = 0;

 protected:
  // Dtor protected as objects shouldn't be deleted via this interface.
  ~PhoneObserver() {}
};

// Base phone class
class Phone :
  public base::RefCountedThreadSafe<Phone>,
  public ua::UserAgent::Delegate {
 private:
  DISALLOW_COPY_AND_ASSIGN(Phone);
 public:
  // Construct a |Phone|.
  Phone(PhoneObserver *phone_observer);

  // Initializes a |Phone| instance.
  bool Init(const Settings& settings);

  // Login the account.
  bool Login(const Account &account);

  // Starts a call to the given destination.
  scoped_refptr<Call> MakeCall(const std::string& destination);
    
  // Hangs up incoming and all active calls.
  void HangUpAll();

  // Hangup all active calls and logout account.
  void Logout();

 private:
  friend class Call;
  friend class base::RefCountedThreadSafe<Phone>;
  typedef std::vector<scoped_refptr<Call> > CallsVector;

  ~Phone();

  base::Lock lock_;
  CallsVector calls_;
  std::string username_;
  std::string scheme_;
  std::string host_;
  PhoneObserver *phone_observer_;

  base::Thread signalling_thread_;

  class AccountPasswordHandler : public PasswordHandler {
   public:
    class Factory : public PasswordHandler::Factory {
     public:
      Factory();
      ~Factory();

      const Account &account() const;
      void set_account(const Account &account);

      scoped_ptr<sippet::PasswordHandler> CreatePasswordHandler() override;

    private:
      Account account_;
    };

    AccountPasswordHandler(Factory *factory);
    ~AccountPasswordHandler() override;
    int GetCredentials(
        const net::AuthChallengeInfo* auth_info,
        base::string16 *username,
        base::string16 *password,
        const net::CompletionCallback& callback) override;

  private:
    Factory *factory_;
  };

  scoped_refptr<net::URLRequestContextGetter> request_context_getter_;
  scoped_ptr<net::HostResolver> host_resolver_;
  scoped_ptr<AuthHandlerFactory> auth_handler_factory_;
  net::BoundNetLog net_log_;
  scoped_ptr<AccountPasswordHandler::Factory> password_handler_factory_;
  scoped_refptr<ua::UserAgent> user_agent_;
  scoped_refptr<NetworkLayer> network_layer_;
  scoped_ptr<ChromeChannelFactory> channel_factory_;

  //
  // Call attributes
  //
  const std::string &username() { return username_; }
  const std::string &host() { return host_; }
  PhoneObserver *phone_observer() { return phone_observer_; }
  ua::UserAgent *user_agent() { return user_agent_.get(); }
  void RemoveCall(const scoped_refptr<Call>& call);

  //
  // Signalling thread callbacks
  //
  void OnInit();
  void OnDestroy();
  void OnLogin(const Account &account);
  void OnLogout();

  //
  // UserAgent callbacks
  //
  void OnRequestSent(int rv);

  //
  // ua::UserAgent::Delegate implementation
  //
  void OnChannelConnected(const EndPoint &destination, int err) override;
  void OnChannelClosed(const EndPoint &destination) override;
  void OnIncomingRequest(
    const scoped_refptr<Request> &incoming_request,
    const scoped_refptr<Dialog> &dialog) override;
  void OnIncomingResponse(
    const scoped_refptr<Response> &incoming_response,
    const scoped_refptr<Dialog> &dialog) override;
  void OnTimedOut(
    const scoped_refptr<Request> &request,
    const scoped_refptr<Dialog> &dialog) override;
  void OnTransportError(
    const scoped_refptr<Request> &request, int error,
    const scoped_refptr<Dialog> &dialog) override;

  template<typename... Args>
  void RouteToCall(const std::string& id,
    void (Call::*method)(Args...), Args...);
};

} // namespace sippet
} // namespace phone

#endif // SIPPET_PHONE_PHONE_H_
