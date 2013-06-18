// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_CONNECTION_H_
#define SIPPET_TRANSPORT_CONNECTION_H_

#include "net/base/completion_callback.h"
#include "base/memory/ref_counted.h"

namespace sippet {

class Connection :
  public base::RefCountedThreadSafe<Connection> {
 public:
  // Connect to the remote destination.
  virtual int Connect(const net::CompletionCallback &callback) = 0;

  // Close the connection.
  virtual void Close() = 0;

  // Close the socket emulating a given error.
  virtual void CloseWithError(int err) = 0;

  // Send a message through the connection.
  virtual int Send(const std::string &message,
                   const net::CompletionCallback &callback) = 0;
};

} /// End of sippet namespace

#endif // SIPPET_TRANSPORT_CONNECTION_H_