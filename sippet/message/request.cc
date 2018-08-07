// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/request.h"

#include "base/strings/string_util.h"
#include "base/strings/string_number_conversions.h"

namespace sippet {

const char Request::kAck[] = "ACK";
const char Request::kBye[] = "BYE";
const char Request::kCancel[] = "CANCEL";
const char Request::kInvite[] = "INVITE";
const char Request::kOptions[] = "OPTIONS";
const char Request::kRegister[] = "REGISTER";

Request::Request() {}

Request::Request(const base::StringPiece& request_method,
                 const GURL &request_uri) {
  std::string request_line(request_method.begin(), request_method.end());
  request_line.push_back(' ');
  request_line.append(request_uri.spec());
  request_line.append(" SIP/2.0");
  request_line.push_back('\0');
  ParseInternal(request_line);
}

Request::~Request() {}

bool Request::IsRequest() const {
  return true;
}

scoped_refptr<Request> Request::CreateAck(const std::string& to_tag) {
  DCHECK(request_method_ == kInvite);

  scoped_refptr<Request> ack = new Request(kAck, request_uri_);

  size_t it;
  std::string value;

  it = 0;
  while (EnumerateHeader(&it, "Via", &value)) {
    ack->AddHeader("Via: " + value);
  }

  ack->AddHeader("Max-Forwards: 70");

  EnumerateHeader(nullptr, "From", &value);
  ack->AddHeader("From: " + value);

  EnumerateHeader(nullptr, "To", &value);
  ack->AddHeader("To: " + value + ";tag=" + to_tag);

  EnumerateHeader(nullptr, "Call-ID", &value);
  ack->AddHeader("Call-ID: " + value);

  int64_t sequence = GetCSeq(nullptr);
  ack->AddHeader("CSeq: " + base::Int64ToString(sequence) + " ACK");

  it = 0;
  while (EnumerateHeader(&it, "Route", &value)) {
    ack->AddHeader("Route: " + value);
  }

  return ack;
}

bool Request::ParseStartLine(std::string::const_iterator line_begin,
                             std::string::const_iterator line_end,
                             std::string* raw_headers) {
  std::string::const_iterator p = std::find(line_begin, line_end, ' ');

  if (p == line_end) {
    DVLOG(1) << "missing method; rejecting";
    return false;
  }
  request_method_ = base::ToUpperASCII(base::StringPiece(line_begin, p));
  *raw_headers = request_method_;

  // Skip whitespace.
  while (*p == ' ')
    ++p;

  std::string::const_iterator uri = p;
  p = std::find(p, line_end, ' ');

  if (p == line_end) {
    DVLOG(1) << "missing request-uri; rejecting";
    return false;
  }
  request_uri_ = GURL(std::string(uri, p));
  raw_headers->push_back(' ');
  raw_headers->append(request_uri_.spec());

  // Skip whitespace.
  while (*p == ' ')
    ++p;

  // Extract the version number
  SipVersion parsed_sip_version = ParseVersion(p, line_end);

  // Clamp the version number to one of: {2.0}
  if (parsed_sip_version == SipVersion(2, 0)) {
    raw_headers->append(" SIP/2.0");
  } else {
    // Ignore everything else
    DVLOG(1) << "rejecting SIP/" << parsed_sip_version.major_value() << "."
             << parsed_sip_version.minor_value();
    return false;
  }

  return true;
}

}  // namespace sippet
