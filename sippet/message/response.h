// Copyright (c) 2013-2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_RESPONSE_H_
#define SIPPET_MESSAGE_RESPONSE_H_

#include "sippet/message/message.h"

namespace sippet {

class Response : public Message {
public:
  // Creates a new response Message.
  Response(int response_code);
  Response(int response_code, const base::StringPiece& status_text);

  // Returns the SIP response code.  This is -1 if the response code text could
  // not be parsed.
  int response_code() const { return response_code_; }

  // Get the SIP status text of the normalized status line.
  std::string GetStatusText() const;

  // Replaces the current status line with the provided one (|new_start| should
  // not have any EOL).
  void ReplaceStatusLine(const std::string& new_status);

  bool IsRequest() const override;

private:
  friend class Message;
  friend class base::RefCountedThreadSafe<Response>;

  Response();
  ~Response() override;

  void Init(int response_code, const base::StringPiece& status_text);

  // Tries to extract the status line from a header block.
  bool ParseStartLine(std::string::const_iterator line_begin,
                      std::string::const_iterator line_end,
                      std::string* raw_headers) override;

  // This is the parsed SIP response code.
  int response_code_;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_RESPONSE_H_
