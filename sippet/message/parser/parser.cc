/* 
 * Copyright (c) 2013, Guilherme Balena Versiani
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies, 
 * either expressed or implied, of the FreeBSD Project.
 */

#include "sippet/message/message.h"

#include <algorithm>

#include "sippet/message/request.h"
#include "sippet/message/response.h"
#include "base/basictypes.h"
#include "base/logging.h"
#include "base/string_number_conversions.h"
#include "base/string_util.h"
#include "base/string_piece.h"
#include "net/http/http_util.h"

namespace {

bool IsStatusLine(
      std::string::const_iterator line_begin,
      std::string::const_iterator line_end) {
  const int slop = 4;
  const int sip_len = 3;
  int buf_len = line_end - line_begin;

  if (buf_len >= sip_len) {
    int i_max = std::min(buf_len - sip_len, slop);
    for (int i = 0; i <= i_max; ++i) {
      if (LowerCaseEqualsASCII(line_begin + i,
          line_begin + i + sip_len, "sip"))
        return true;
    }
  }
  return false;
}

std::string::const_iterator FindLineEnd(
    std::string::const_iterator begin,
    std::string::const_iterator end) {
  size_t i = base::StringPiece(begin, end).find_first_of("\r\n");
  if (i == base::StringPiece::npos)
    return end;
  return begin + i;
}

sippet::Version ParseVersion(
    std::string::const_iterator line_begin,
    std::string::const_iterator line_end) {
  std::string::const_iterator p = line_begin;

  // RFC3261: SIP-Version = "SIP" "/" 1*DIGIT "." 1*DIGIT

  if ((line_end - p < 3) || !LowerCaseEqualsASCII(p, p + 3, "sip")) {
    DVLOG(1) << "missing status line";
    return sippet::Version();
  }

  p += 3;

  if (p >= line_end || *p != '/') {
    DVLOG(1) << "missing version";
    return sippet::Version();
  }

  std::string::const_iterator dot = std::find(p, line_end, '.');
  if (dot == line_end) {
    DVLOG(1) << "malformed version";
    return sippet::Version();
  }

  ++p;  // from / to first digit.
  ++dot;  // from . to second digit.

  if (!(*p >= '0' && *p <= '9' && *dot >= '0' && *dot <= '9')) {
    DVLOG(1) << "malformed version number";
    return sippet::Version();
  }

  uint16 major = *p - '0';
  uint16 minor = *dot - '0';

  return sippet::Version(major, minor);
}

bool ParseStatusLine(
    std::string::const_iterator line_begin,
    std::string::const_iterator line_end,
    sippet::Version *version,
    int *response_code,
    std::string *reason_phrase) {
  // Extract the version number
  *version = ParseVersion(line_begin, line_end);
  if (*version == sippet::Version()) {
    DVLOG(1) << "invalid response";
    return false;
  }

  // Clamp the version number to {2.0}
  if (*version != sippet::Version(2, 0)) {
    *version = sippet::Version(2, 0);
    DVLOG(1) << "assuming SIP/2.0";
  }

  std::string::const_iterator p = std::find(line_begin, line_end, ' ');

  if (p == line_end) {
    DVLOG(1) << "missing response status";
    return false;
  }

  // Skip whitespace.
  while (*p == ' ')
    ++p;

  std::string::const_iterator code = p;
  while (*p >= '0' && *p <= '9')
    ++p;

  if (p == code) {
    DVLOG(1) << "missing response status number";
    return false;
  }
  base::StringToInt(base::StringPiece(code, p), response_code);

  // Skip whitespace.
  while (*p == ' ')
    ++p;

  // Trim trailing whitespace.
  while (line_end > p && line_end[-1] == ' ')
    --line_end;

  if (p == line_end) {
    DVLOG(1) << "missing response status text; assuming empty string";
    reason_phrase->clear();
  } else {
    reason_phrase->assign(p, line_end);
  }

  return true;
}

bool ParseRequestLine(
    std::string::const_iterator line_begin,
    std::string::const_iterator line_end,
    sippet::Method *method,
    GURL *request_uri,
    sippet::Version *version) {

  // Skip any leading whitespace.
  while (line_begin != line_end &&
         (*line_begin == ' ' || *line_begin == '\t' ||
          *line_begin == '\r' || *line_begin == '\n'))
    ++line_begin;

  std::string::const_iterator meth = line_begin;
  std::string::const_iterator p = std::find(line_begin, line_end, ' ');

  if (p == line_end) {
    DVLOG(1) << "missing method";
    return false;
  }
  method->set_str(std::string(meth, p));

  // Skip whitespace.
  while (*p == ' ')
    ++p;

  std::string::const_iterator uri = p;
  p = std::find(p, line_end, ' ');

  if (p == line_end) {
    DVLOG(1) << "missing request-uri";
    return false;
  }

  *request_uri = GURL(std::string(uri, p));

  // Skip whitespace.
  while (*p == ' ')
    ++p;

  // Extract the version number
  *version = ParseVersion(p, line_end);
  if (*version == sippet::Version()) {
    DVLOG(1) << "invalid response";
    return false;
  }

  return true;
}

} // End of empty namespace

namespace sippet {

scoped_refptr<Message> Message::Parse(const std::string &raw_message) {
  scoped_refptr<Message> message;

  std::string::const_iterator i = raw_message.begin();
  std::string::const_iterator end = raw_message.end();
  std::string::const_iterator start = i;

  i = FindLineEnd(start, end);
  do {
    Version version;
    if (IsStatusLine(start, i)) {
      int code;
      std::string reason_phrase;
      if (!ParseStatusLine(start, i, &version, &code, &reason_phrase))
        break;
      message = new Response(code, reason_phrase, version);
    }
    else {
      Method method;
      GURL request_uri;
      if (!ParseRequestLine(start, i, &method, &request_uri, &version))
        break;
      message = new Request(method, request_uri, version);
    }
  } while (false);

  if (message) {
    // TODO
  }

  return message;
}

} // End of sippet namespace
