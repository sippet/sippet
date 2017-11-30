// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_CHROME_CHROME_DATAGRAM_CHANNEL_H_
#define SIPPET_TRANSPORT_CHROME_CHROME_DATAGRAM_CHANNEL_H_

#include "base/memory/weak_ptr.h"
#include "net/base/address_list.h"
#include "net/dns/host_resolver.h"
#include "net/log/net_log_with_source.h"
#include "url/gurl.h"
#include "sippet/transport/channel.h"
#include "sippet/transport/chrome/chrome_datagram_writer.h"
#include "sippet/transport/chrome/chrome_datagram_reader.h"

namespace base {
class RepeatingTimer;
}  // namespace base

namespace net {
class ClientSocketFactory;
class DatagramClientSocket;
class URLRequestContextGetter;
class IOBufferWithSize;
class NetLog;
class HostResolver;
}  // namespace net

namespace sippet {

class Message;

class ChromeDatagramChannel : public Channel {
 public:
  ChromeDatagramChannel(const EndPoint& destination,
      Channel::Delegate *delegate,
      net::ClientSocketFactory* client_socket_factory,
      const scoped_refptr<net::URLRequestContextGetter>& request_context_getter);

  int origin(EndPoint *origin) const override;
  const EndPoint& destination() const override;

  bool is_secure() const override;
  bool is_connected() const override;
  bool is_stream() const override;

  void Connect() override;
  int ReconnectIgnoringLastError() override;
  int ReconnectWithCertificate(net::X509Certificate* client_cert,
                               net::SSLPrivateKey* private_key) override;

  int Send(const scoped_refptr<Message> &message,
           const net::CompletionCallback& callback) override;

  void Close() override;

  void CloseWithError(int err) override;

  void DetachDelegate() override;

  void SetKeepAlive(int seconds) override;

 private:
  friend class base::RefCountedThreadSafe<Channel>;
  ~ChromeDatagramChannel() override;

  enum State {
    STATE_RESOLVE_HOST,
    STATE_RESOLVE_HOST_COMPLETE,
    STATE_NONE,
  };

  void RunUserConnectCallback(int result);
  void RunUserChannelClosed(int result);

  void OnIOComplete(int result);

  int DoLoop(int last_io_result);
  int DoResolveHost();
  int DoResolveHostComplete(int result);

  void CloseTransportSocket();

  void PostDoRead();
  void DoRead();
  void OnReadComplete(int result);

  void SendKeepAlive();

  State next_state_;

  EndPoint destination_;
  Channel::Delegate *delegate_;

  net::HostResolver* resolver_;
  std::unique_ptr<net::HostResolver::Request> resolve_request_;
  net::AddressList addresses_;

  net::ClientSocketFactory* client_socket_factory_;
  std::unique_ptr<net::DatagramClientSocket> socket_;
  std::unique_ptr<ChromeDatagramReader> datagram_reader_;
  std::unique_ptr<ChromeDatagramWriter> datagram_writer_;
  const net::NetLogWithSource net_log_;
  std::unique_ptr<base::RepeatingTimer> keep_alive_timer_;

  bool is_connected_;

  base::WeakPtrFactory<ChromeDatagramChannel> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(ChromeDatagramChannel);
};

} /// End of sippet namespace

#endif // SIPPET_TRANSPORT_CHROME_CHROME_DATAGRAM_CHANNEL_H_
