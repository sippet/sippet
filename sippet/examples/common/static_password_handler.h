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
    Factory(const base::string16 &username, const base::string16 &password);
    ~Factory() override;

    std::unique_ptr<sippet::PasswordHandler>
        CreatePasswordHandler() override;

   private:
    base::string16 username_;
    base::string16 password_;
  };

  StaticPasswordHandler(const base::string16 &username,
                        const base::string16 &password);

  ~StaticPasswordHandler() override;

  int GetCredentials(
      const net::AuthChallengeInfo* auth_info,
      base::string16 *username,
      base::string16 *password,
      const net::CompletionCallback& callback) override;

 private:
  base::string16 username_;
  base::string16 password_;
};

#endif // SIPPET_EXAMPLES_COMMON_STATIC_PASSWORD_HANDLER_H_

