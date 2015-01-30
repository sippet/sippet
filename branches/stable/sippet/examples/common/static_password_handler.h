// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_EXAMPLES_COMMON_STATIC_PASSWORD_HANDLER_H_
#define SIPPET_EXAMPLES_COMMON_STATIC_PASSWORD_HANDLER_H_

#include "sippet/ua/password_handler.h"

class StaticPasswordHandler : public sippet::PasswordHandler {
 public:
  class Factory : public sippet::PasswordHandler::Factory {
   public:
    Factory(const string16 &username, const string16 &password);
    virtual ~Factory();

    virtual scoped_ptr<sippet::PasswordHandler>
        CreatePasswordHandler() OVERRIDE;

   private:
    string16 username_;
    string16 password_;
  };

  StaticPasswordHandler(const string16 &username, const string16 &password);

  virtual ~StaticPasswordHandler();

  virtual int GetCredentials(
      const net::AuthChallengeInfo* auth_info,
      string16 *username,
      string16 *password,
      const net::CompletionCallback& callback) OVERRIDE;

 private:
  string16 username_;
  string16 password_;
};

#endif // SIPPET_EXAMPLES_COMMON_STATIC_PASSWORD_HANDLER_H_

