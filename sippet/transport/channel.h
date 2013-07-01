// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_CHANNEL_H_
#define SIPPET_TRANSPORT_CHANNEL_H_

#include "net/base/completion_callback.h"
#include "net/base/net_export.h"
#include "net/base/net_log.h"
#include "net/base/address_list.h"
#include "net/base/io_buffer.h"
#include "net/base/net_log.h"
#include "net/socket_stream/socket_stream_metrics.h"
#include "base/memory/ref_counted.h"

namespace sippet {

class Message;

class NET_EXPORT_PRIVATE Channel :
  public base::RefCountedThreadSafe<Channel> {
 public:
  class Delegate {
   public:
    Delegate() {}
    virtual ~Delegate() {}

    // Called when the channel has been connected.
    virtual void OnConnected(Channel* channel) = 0;

    // Called when a message is received from the channel.
    virtual void OnReceived(Channel *channel,
                            const scoped_refptr<Message> &message) = 0;

    // Called when the channel is closed.
    virtual void OnClosed(Channel *channel, int error) = 0;
  };

  Channel(const GURL& url, Delegate *delegate);
  virtual ~Channel();

  // The URL representing the destination entity of this channel.
  const GURL& url() const { return url_; }

  // The channel delegate (normally the NetworkLayer).
  Delegate* delegate() const { return delegate_; }

  // Whether this channel is a secure channel.
  virtual bool is_secure() const = 0;

  // The channel resolved address list.
  virtual const net::AddressList& address_list() const = 0;

  // The attached NetLog.
  net::BoundNetLog* net_log() { return &net_log_; }

  // Opens the connection on the IO thread.
  // Once the connection is established, calls delegate's OnConnected.
  virtual void Connect() = 0;

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

  // Starts receiving messages asynchronously.
  // Once a message is received or the channel is closed, calls delegate's
  // OnReceived.
  virtual void Receive() = 0;

  // Requests to close the connection.
  // Once the connection is closed, calls delegate's OnClose.
  virtual void Close() = 0;

  // Closes the channel emulating the given error for pending callbacks.
  // Once the connection is closed, calls delegate's OnClose.
  virtual void CloseWithError(int err) = 0;

  // Detach delegate.  Call before delegate is deleted.  Once delegate
  // is detached, close the channel and never call delegate back.
  void DetachDelegate();

 protected:
  // Should be called by implementors when receiving messages.
  void DidReceive(const scoped_refptr<Message> &message);

  // Should be called by implementors when a disconnection is found.
  void DidClose();

  // Should be called to notify connection is alive.
  void LogChannelAlive();
  // Should be called to notify connection is dead.
  void LogChannelDead();
  // Should be called right before start connecting.
  void LogBeginConnect();
  // Should be called when the connection is successfuly established.
  void LogConnectionEstablished();
  // Should be called when the connection establishment fails.
  void LogConnectionError(int error);
  // Should be called when data is received.
  void LogReceivedData(int bytes);
  // Should be called when data is sent.
  void LogSentData(int bytes);

 private:
  Delegate *delegate_;
  GURL url_;
  BoundNetLog net_log_;

  DISALLOW_COPY_AND_ASSIGN(Channel);
};

} /// End of sippet namespace

#endif // SIPPET_TRANSPORT_CHANNEL_H_
