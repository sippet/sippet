// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_UA_PASSWORD_HANDLER_H_
#define SIPPET_UA_PASSWORD_HANDLER_H_

#include "base/memory/scoped_ptr.h"
#include "base/strings/string16.h"
#include "net/base/completion_callback.h"

namespace net {
class AuthChallengeInfo;
}

namespace sippet {

// This is the interface for the application-specific class that will route
// authentication info to the user and collect his username and password.
class PasswordHandler {
 public:
  class Factory {
   public:
    virtual ~Factory() {}

    // Returns the application-specific |PasswordHandler| implementation.
    virtual scoped_ptr<PasswordHandler> CreatePasswordHandler() = 0;
  };

  virtual ~PasswordHandler() {}

  // Gets the user credentials for the provided |auth_info|. If the application
  // has to display a UI dialog to the user, this function shall forward the
  // request to the UI thread and return |ERR_IO_PENDING|.
  virtual int GetCredentials(const net::AuthChallengeInfo* auth_info,
                             string16 *username,
                             string16 *password,
                             const net::CompletionCallback& callback) = 0;
};

} // namespace sippet

#endif // SIPPET_UA_PASSWORD_HANDLER_H_
