// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_PHONE_IMPL_H_
#define SIPPET_PHONE_PHONE_IMPL_H_

#include "sippet/phone/phone.h"

#include "base/threading/thread.h"
#include "base/synchronization/waitable_event.h"
#include "base/synchronization/lock.h"
#include "base/timer/timer.h"
#include "net/dns/host_resolver.h"
#include "net/url_request/url_request_context_getter.h"

#include "sippet/transport/network_layer.h"
#include "sippet/transport/ssl_cert_error_handler.h"
#include "sippet/transport/chrome/chrome_channel_factory.h"
#include "sippet/ua/auth_handler_factory.h"
#include "sippet/ua/password_handler.h"
#include "sippet/ua/ua_user_agent.h"
#include "sippet/phone/call_impl.h"

#include "talk/app/webrtc/peerconnectioninterface.h"

namespace sippet {
namespace phone {

class PhoneImpl :
  public Phone,
  public ua::UserAgent::Delegate {
 private:
  DISALLOW_COPY_AND_ASSIGN(PhoneImpl);
 public:
  PhoneState state() const override;
  bool Init(const Settings& settings) override;
  void Register(const net::CompletionCallback& on_completed) override;
  void StartRefreshRegister(
      const net::CompletionCallback& on_completed) override;
  void StopRefreshRegister() override;
  void Unregister(const net::CompletionCallback& on_completed) override;
  void UnregisterAll(const net::CompletionCallback& on_completed) override;
  scoped_refptr<Call> MakeCall(const std::string& destination,
      const net::CompletionCallback& on_completed) override;

 private:
  friend class Phone;
  friend class CallImpl;
  friend class base::RefCountedThreadSafe<Phone>;
  typedef std::vector<scoped_refptr<CallImpl>> CallsVector;

  // Construct a |Phone|.
  PhoneImpl(Phone::Delegate *delegate);
  ~PhoneImpl() override;

  bool InitializePeerConnectionFactory(const Settings& settings);
  void DeletePeerConnectionFactory();

  PhoneState state_;
  PhoneState last_state_;
  base::Lock lock_;
  CallsVector calls_;
  std::string username_;
  std::string scheme_;
  std::string host_;
  Phone::Delegate *delegate_;
  Settings settings_;

  base::Thread signalling_thread_;
  base::WaitableEvent signalling_thread_event_;

  class PasswordHandler : public sippet::PasswordHandler {
   public:
    class Factory : public sippet::PasswordHandler::Factory {
     public:
      Factory(Settings *settings);
      ~Factory() override;

      const Settings *settings() const { return settings_;  }

      scoped_ptr<sippet::PasswordHandler> CreatePasswordHandler() override;

    private:
      Settings *settings_;
    };

    PasswordHandler(Factory *factory);
    ~PasswordHandler() override;
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
  scoped_ptr<PasswordHandler::Factory> password_handler_factory_;
  scoped_ptr<ua::UserAgent> user_agent_;
  scoped_ptr<NetworkLayer> network_layer_;
  scoped_ptr<ChromeChannelFactory> channel_factory_;
  scoped_ptr<base::OneShotTimer<PhoneImpl>> refresh_timer_;
  Settings::IceServers ice_servers_;
  base::Time register_expires_;
  net::CompletionCallback on_register_completed_;
  net::CompletionCallback on_unregister_completed_;
  net::CompletionCallback on_refresh_completed_;

  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
    peer_connection_factory_;

  scoped_refptr<Request> last_request_;

  //
  // Call attributes
  //
  const std::string &username() { return username_; }
  const std::string &host() { return host_; }
  Phone::Delegate *delegate() { return delegate_; }
  ua::UserAgent *user_agent() { return user_agent_.get(); }
  base::MessageLoop *signalling_message_loop() {
    return signalling_thread_.message_loop();
  }
  void RemoveCall(const scoped_refptr<Call>& call);

  //
  // Signalling thread callbacks
  //
  void OnInit(const Settings& settings);
  void OnDestroy();
  void OnRegister();
  void OnStartRefreshRegister();
  void OnStopRefreshRegister();
  void OnUnregister(bool all);

  //
  // UserAgent callbacks
  //
  static void OnRequestSent(const net::CompletionCallback& callback, int rv);

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

  //
  // Refresh register timer callback
  //
  void OnRefreshRegister();

  CallImpl *RouteToCall(const scoped_refptr<Request>& request);
  CallImpl *RouteToCall(const scoped_refptr<Dialog>& dialog);
  unsigned int GetContactExpiration(const scoped_refptr<Response>& response);

  std::string GetRegistrarUri() const;
  std::string GetFromUri() const;
  SipURI GetToUri(const std::string& destination) const;
};

} // namespace sippet
} // namespace phone

#endif // SIPPET_PHONE_PHONE_IMPL_H_
