// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/examples/common/static_password_handler.h"

#include "net/base/net_errors.h"

StaticPasswordHandler::Factory::Factory(const base::string16 &username,
    const base::string16 &password)
  : username_(username), password_(password) {
}

StaticPasswordHandler::Factory::~Factory() {}

scoped_ptr<sippet::PasswordHandler>
    StaticPasswordHandler::Factory::CreatePasswordHandler() {
  scoped_ptr<PasswordHandler> password_handler(
    new StaticPasswordHandler(username_, password_));
  return password_handler.Pass();
}

StaticPasswordHandler::StaticPasswordHandler(const base::string16 &username,
    const base::string16 &password)
  : username_(username), password_(password) {
}

StaticPasswordHandler::~StaticPasswordHandler() {}

int StaticPasswordHandler::GetCredentials(
    const net::AuthChallengeInfo* auth_info,
    base::string16 *username,
    base::string16 *password,
    const net::CompletionCallback& callback) {
  *username = username_;
  *password = password_;
  return net::OK;
}
