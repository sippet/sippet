// Copyright (c) 2013-2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_REQUEST_H_
#define SIPPET_MESSAGE_REQUEST_H_

#include "sippet/message/message.h"

namespace sippet {

class Request : public Message {
 public:
  // RFC 3261 methods:
  static const char kAck[];
  static const char kBye[];
  static const char kCancel[];
  static const char kInvite[];
  static const char kOptions[];
  static const char kRegister[];

  // Create a new |Request|.
  Request(const base::StringPiece& request_method,
          const GURL &request_uri);

  // Returns the SIP request method normalized in uppercase.  This is empty if
  // the request method could not be parsed.
  const std::string& request_method() const { return request_method_; }

  // Returns the SIP request URI.  This is empty if the request URI could not
  // be parsed.
  const GURL& request_uri() const { return request_uri_; };

  bool IsRequest() const override;

 private:
  friend class Message;
  friend class base::RefCountedThreadSafe<Request>;

  Request();
  ~Request() override;

  // Tries to extract the request line from a header block.
  bool ParseStartLine(std::string::const_iterator line_begin,
                      std::string::const_iterator line_end,
                      std::string* raw_headers) override;

  // This is the parsed SIP request method.
  std::string request_method_;

  // This is the parsed SIP Request-URI.
  GURL request_uri_;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_REQUEST_H_
