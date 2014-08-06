// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_CHROME_CHROME_DATAGRAM_CHANNEL_H_
#define SIPPET_TRANSPORT_CHROME_CHROME_DATAGRAM_CHANNEL_H_

#include "sippet/transport/channel.h"
#include "sippet/transport/chrome/framed_write_stream_socket.h"
#include "base/memory/weak_ptr.h"
#include "net/base/address_list.h"
#include "net/dns/host_resolver.h"
#include "net/dns/single_request_host_resolver.h"
#include "url/gurl.h"

namespace net {
class ClientSocketFactory;
class DatagramClientSocket;
class URLRequestContextGetter;
class IOBufferWithSize;
class NetLog;
class HostResolver;
}

namespace sippet {

class Message;

class ChromeDatagramChannel : public Channel {
 public:
  ChromeDatagramChannel(const EndPoint& destination,
      Channel::Delegate *delegate,
      net::ClientSocketFactory* client_socket_factory,
      const scoped_refptr<net::URLRequestContextGetter>& request_context_getter);

  virtual int origin(EndPoint *origin) const OVERRIDE;
  virtual const EndPoint& destination() const OVERRIDE;

  virtual bool is_secure() const OVERRIDE;
  virtual bool is_connected() const OVERRIDE;
  virtual bool is_stream() const OVERRIDE;

  virtual void Connect() OVERRIDE;
  virtual int ReconnectIgnoringLastError() OVERRIDE;
  virtual int ReconnectWithCertificate(
      net::X509Certificate* client_cert) OVERRIDE;

  virtual int Send(const scoped_refptr<Message> &message,
                   const net::CompletionCallback& callback) OVERRIDE;

  virtual void Close() OVERRIDE;

  virtual void CloseWithError(int err) OVERRIDE;

  virtual void DetachDelegate() OVERRIDE;

 private:
  friend class base::RefCountedThreadSafe<Channel>;
  virtual ~ChromeDatagramChannel();

  enum State {
    STATE_RESOLVE_HOST,
    STATE_RESOLVE_HOST_COMPLETE,
    STATE_NONE,
  };

  enum ReadIOState {
    READ_IDLE,
    READ_POSTED,
    READ_PENDING,
  };

  void RunUserConnectCallback(int result);
  void RunUserChannelClosed(int result);

  void OnIOComplete(int result);

  int DoLoop(int last_io_result);
  int DoResolveHost();
  int DoResolveHostComplete(int result);

  // Read loop functions.
  void PostDoRead();
  void DoRead();
  void ProcessReadDone(int status);
  int ProcessReceivedData(int read_bytes);

  void CloseTransportSocket();

  State next_state_;

  EndPoint destination_;
  Channel::Delegate *delegate_;

  net::SingleRequestHostResolver host_resolver_;
  net::AddressList addresses_;

  net::ClientSocketFactory* client_socket_factory_;
  scoped_ptr<net::DatagramClientSocket> socket_;
  scoped_ptr<FramedWriteStreamSocket> datagram_socket_;
  net::BoundNetLog bound_net_log_;

  bool is_connected_;

  ReadIOState read_state_;
  scoped_refptr<net::IOBufferWithSize> read_buf_;

  base::WeakPtrFactory<ChromeDatagramChannel> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(ChromeDatagramChannel);
};

} /// End of sippet namespace

#endif // SIPPET_TRANSPORT_CHROME_CHROME_DATAGRAM_CHANNEL_H_
