// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/response.h"

#include <string>

#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"

namespace sippet {

namespace {

void CheckDoesNotHaveEmbededNulls(const std::string& str) {
  // Care needs to be taken when adding values to the raw headers string to
  // make sure it does not contain embeded NULLs. Any embeded '\0' may be
  // understood as line terminators and change how header lines get tokenized.
  CHECK(str.find('\0') == std::string::npos);
}

const std::unordered_map<int, std::string> kDefaultStatusText = {
  // Provisional 1xx
  { 100, "Trying" },
  { 180, "Ringing" },
  { 181, "Call Is Being Forwarded" },
  { 182, "Queued" },
  { 183, "Session Progress" },
  { 199, "Early Dialog Terminated" },

  // Successful 2xx
  { 200, "OK" },
  { 202, "Accepted" },
  { 204, "No Notification" },

  // Redirection 3xx
  { 300, "Multiple Choices" },
  { 301, "Moved Permanently" },
  { 302, "Moved Temporarily" },
  { 305, "Use Proxy" },
  { 380, "Alternative Service" },

  // Request failure 4xx
  { 400, "Bad Request" },
  { 401, "Unauthorized" },
  { 402, "Payment Required" },
  { 403, "Forbidden" },
  { 404, "Not Found" },
  { 405, "Method Not Allowed" },
  { 406, "Not Acceptable" },
  { 407, "Proxy Authentication Required" },
  { 408, "Request Timeout" },
  { 409, "Conflict" },
  { 410, "Gone" },
  { 412, "Conditional Request Failed" },
  { 413, "Request Entity Too Large" },
  { 414, "Request-URI Too Long" },
  { 415, "Unsupported Media Type" },
  { 416, "Unsupported URI Scheme" },
  { 417, "Unknown Resource-Priority" },
  { 420, "Bad Extension" },
  { 421, "Extension Required" },
  { 422, "Session Interval Too Small" },
  { 423, "Interval Too Brief" },
  { 424, "Bad Location Information" },
  { 428, "Use Identity Header" },
  { 429, "Provide Referrer Identity" },
  { 430, "Flow Failed" },
  { 433, "Anonymity Disallowed" },
  { 436, "Bad Identity-Info" },
  { 437, "Unsupported Certificate" },
  { 438, "Invalid Identity Header" },
  { 439, "First Hop Lacks Outbound Support" },
  { 440, "Max-Breadth Exceeded" },
  { 469, "Bad Info Package" },
  { 470, "Consent Needed" },
  { 480, "Temporarily Unavailable" },
  { 481, "Call/Transaction Does Not Exist" },
  { 482, "Loop Detected" },
  { 483, "Too Many Hops" },
  { 484, "Address Incomplete" },
  { 485, "Ambiguous" },
  { 486, "Busy Here" },
  { 487, "Request Terminated" },
  { 488, "Not Acceptable Here" },
  { 489, "Bad Event" },
  { 491, "Request Pending" },
  { 493, "Undecipherable" },
  { 494, "Security Agreement Required" },

  // Server failure 5xx
  { 500, "Server Internal Error" },
  { 501, "Not Implemented" },
  { 502, "Bad Gateway" },
  { 503, "Service Unavailable" },
  { 504, "Server Time-out" },
  { 505, "Version Not Supported" },
  { 513, "Message Too Large" },
  { 580, "Precondition Failure" },

  // Global failure 6xx
  { 600, "Busy Everywhere" },
  { 603, "Decline" },
  { 604, "Does Not Exist Anywhere" },
  { 606, "Not Acceptable" },
};

}  // namespace

Response::Response() : response_code_(-1) {}

Response::Response(int response_code) {
  base::StringPiece status_text;
  auto kv = kDefaultStatusText.find(response_code);
  if (kv != kDefaultStatusText.end()) {
    status_text = base::StringPiece(kv->second);
  }
  Init(response_code, status_text);
}

Response::Response(int response_code,
                   const base::StringPiece& status_text) {
  Init(response_code, status_text);
}

Response::~Response() {}

bool Response::IsRequest() const {
  return false;
}

std::string Response::GetStatusText() const {
  // GetStatusLine() is already normalized, so it has the format:
  // '<sip_version> SP <response_code>' or
  // '<sip_version> SP <response_code> SP <status_text>'.
  std::string status_text = GetStartLine();
  std::string::const_iterator begin = status_text.begin();
  std::string::const_iterator end = status_text.end();
  // Seek to beginning of <response_code>.
  begin = std::find(begin, end, ' ');
  CHECK(begin != end);
  ++begin;
  CHECK(begin != end);
  // See if there is another space.
  begin = std::find(begin, end, ' ');
  if (begin == end)
    return std::string();
  ++begin;
  CHECK(begin != end);
  return std::string(begin, end);
}

void Response::ReplaceStatusLine(const std::string& new_status) {
  CheckDoesNotHaveEmbededNulls(new_status);
  // Copy up to the null byte.  This just copies the status line.
  std::string new_raw_headers(new_status);
  new_raw_headers.push_back('\0');

  HeaderSet empty_to_remove;
  MergeWithMessage(new_raw_headers, empty_to_remove);
}

void Response::Init(int response_code, const base::StringPiece& status_text) {
  DCHECK_GE(response_code, 100);
  DCHECK_LE(response_code, 699);
  std::string status_line("SIP/2.0 ");
  status_line.append(base::IntToString(response_code));
  status_line.push_back(' ');
  status_line.append(status_text.begin(), status_text.end());
  status_line.push_back('\0');
  ParseInternal(status_line);
}

bool Response::ParseStartLine(std::string::const_iterator line_begin,
                              std::string::const_iterator line_end,
                              std::string* raw_headers) {
  // Extract the version number
  SipVersion parsed_sip_version = ParseVersion(line_begin, line_end);

  // Clamp the version number to one of: {2.0}
  if (parsed_sip_version == SipVersion(2, 0)) {
    *raw_headers = "SIP/2.0";
  } else {
    // Ignore everything else
    DVLOG(1) << "rejecting SIP/" << parsed_sip_version.major_value() << "."
             << parsed_sip_version.minor_value();
    return false;
  }

  std::string::const_iterator p = std::find(line_begin, line_end, ' ');

  if (p == line_end) {
    DVLOG(1) << "missing response status; rejecting";
    return false;
  }

  // Skip whitespace.
  while (p < line_end && *p == ' ')
    ++p;

  std::string::const_iterator code = p;
  while (p < line_end && base::IsAsciiDigit(*p))
    ++p;

  if (p == code) {
    DVLOG(1) << "missing response status number; rejecting";
    return false;
  }
  raw_headers->push_back(' ');
  raw_headers->append(code, p);
  base::StringToInt(base::StringPiece(code, p), &response_code_);

  if (response_code_ < 100 || response_code_ > 699) {
    DVLOG(1) << "invalid response code " << response_code_ << "; rejecting";
    return false;
  }

  // Skip whitespace.
  while (p < line_end && *p == ' ')
    ++p;

  // Trim trailing whitespace.
  while (line_end > p && line_end[-1] == ' ')
    --line_end;

  if (p != line_end) {
    raw_headers->push_back(' ');
    raw_headers->append(p, line_end);
  }

  return true;
}

}  // namespace sippet
