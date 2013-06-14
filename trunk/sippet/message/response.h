// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_RESPONSE_H_
#define SIPPET_MESSAGE_RESPONSE_H_

#include "sippet/message/message.h"
#include "sippet/message/version.h"

namespace sippet {

class Response :
  public Message {
private:
  DISALLOW_COPY_AND_ASSIGN(Response);

private:
  virtual ~Response() {}

public:
  // This constructor will take a default reason phrase for you
  Response(int response_code,
           const Version &version = Version(2,0))
    : Message(false) { /* TODO */ }

  // Just in case you want to define the reason phrase
  Response(int response_code,
           const std::string &reason_phrase,
           const Version &version = Version(2,0))
    : Message(false), response_code_(response_code),
      reason_phrase_(reason_phrase), version_(version) {}

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

private:
  Version version_;
  int response_code_;
  std::string reason_phrase_;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_RESPONSE_H_
