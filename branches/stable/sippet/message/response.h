// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_RESPONSE_H_
#define SIPPET_MESSAGE_RESPONSE_H_

#include "sippet/message/message.h"
#include "sippet/message/version.h"
#include "base/gtest_prod_util.h"

namespace sippet {

class Response :
  public Message {
private:
  DISALLOW_COPY_AND_ASSIGN(Response);
public:
  // Every processed |Response| object should refer to some
  // previous |Request|.
  const scoped_refptr<Request> &refer_to() const {
    return refer_to_;
  }

  int response_code() const { return response_code_; }
  void set_response_code(int response_code) {
    response_code_ = response_code;
  }

  std::string reason_phrase() const { return reason_phrase_; }
  void set_reason_phrase(const std::string &reason_phrase) {
    reason_phrase_ = reason_phrase;
  }

  Version version() const { return version_; }
  void set_version(const Version &version) {
    version_ = version;
  }

  virtual void print(raw_ostream &os) const OVERRIDE;

  // Get a the dialog identifier.
  virtual std::string GetDialogId() const OVERRIDE;

private:
  virtual ~Response();

  FRIEND_TEST_ALL_PREFIXES(NetworkLayerTest, StaticFunctions);
  FRIEND_TEST_ALL_PREFIXES(AuthControllerTest, NoExplicitCredentialsAllowed);

  friend class Request;
  friend class Message;
  friend class ClientTransactionImpl;
  friend class AuthControllerTest;

  Version version_;
  int response_code_;
  scoped_refptr<Request> refer_to_;
  std::string reason_phrase_;

  Response(int response_code,
           const std::string &reason_phrase,
           Direction direction,
           const Version &version = Version(2,0));

  void set_refer_to(const scoped_refptr<Request> &request) {
    refer_to_ = request;
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_RESPONSE_H_
