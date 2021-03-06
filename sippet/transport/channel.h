// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_CHANNEL_H_
#define SIPPET_TRANSPORT_CHANNEL_H_

#include "net/base/completion_callback.h"
#include "net/base/net_export.h"
#include "net/base/address_list.h"
#include "base/memory/ref_counted.h"
#include "sippet/transport/end_point.h"

namespace net {
class SSLInfo;
class X509Certificate;
}

namespace sippet {

class Message;

class NET_EXPORT_PRIVATE Channel :
  public base::RefCountedThreadSafe<Channel> {
 private:
  DISALLOW_COPY_AND_ASSIGN(Channel);
 public:
  class Delegate {
   public:
    virtual ~Delegate() {}

    // Called when the channel has been connected.
    virtual void OnChannelConnected(const scoped_refptr<Channel> &channel,
                                    int error) = 0;

    // Called when a message is received from the channel.
    virtual void OnIncomingMessage(const scoped_refptr<Channel> &channel,
                                   const scoped_refptr<Message> &message) = 0;

    // Called when the channel is closed.
    virtual void OnChannelClosed(const scoped_refptr<Channel> &channel,
                                 int error) = 0;

    // Called when there's a SSL certificate error.
    virtual void OnSSLCertificateError(const scoped_refptr<Channel> &channel,
                                       const net::SSLInfo &ssl_info,
                                       bool fatal) = 0;
  };

  Channel() {}

  // The origin (local address) of this channel.
  virtual int origin(EndPoint *origin) const = 0;

  // The destination of this channel.
  virtual const EndPoint& destination() const = 0;

  // Whether this channel is a secure channel.
  virtual bool is_secure() const = 0;

  // Whether this channel is connected.
  virtual bool is_connected() const = 0;

  // Whether this channel is a stream channel.
  virtual bool is_stream() const = 0;

  // Opens the connection on the IO thread.
  // Once the connection is established, calls delegate's OnChannelConnected.
  virtual void Connect() = 0;

  // Restarts the some previous connection attempt, ignoring the last error.
  // This method is used to continue past various SSL related errors.
  //
  // Not all errors can be ignored using this method.  See error code
  // descriptions for details about errors that can be ignored.
  virtual int ReconnectIgnoringLastError() = 0;

  // Restarts the internal channel with a client certificate.
  virtual int ReconnectWithCertificate(net::X509Certificate* client_cert) = 0;

  // Writes the message to the underlying socket.
  // ERR_IO_PENDING is returned if the operation could not be completed
  // synchronously, in which case the result will be passed to the callback
  // when available. While connecting is in progress, it also accepts the
  // message to be sent later and notified in the callback asynchronously.
  // Returns OK on success (synchronous send).
  // |message| the message to be sent.
  // |callback| the callback to be called on completion.
  virtual int Send(const scoped_refptr<Message> &message,
                   const net::CompletionCallback& callback) = 0;

  // Requests to close the connection.
  // Once the connection is closed, calls delegate's OnClose.
  virtual void Close() = 0;

  // Closes the channel emulating the given error for pending callbacks.
  // Once the connection is closed, calls delegate's OnChannelClosed.
  virtual void CloseWithError(int err) = 0;

  // Detach delegate.  Call before delegate is deleted.  Once delegate
  // is detached, close the channel and never call delegate back.
  virtual void DetachDelegate() = 0;

 protected:
  friend class base::RefCountedThreadSafe<Channel>;
  virtual ~Channel() {}
};

} /// End of sippet namespace

#endif // SIPPET_TRANSPORT_CHANNEL_H_
