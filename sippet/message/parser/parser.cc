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
#include "sippet/message/parser/tokenizer.h"
#include "base/basictypes.h"
#include "base/strings/string_split.h"
#include "base/logging.h"
#include "base/string_number_conversions.h"
#include "base/string_util.h"
#include "base/string_piece.h"
#include "net/http/http_util.h"
#include "net/base/net_util.h"

#include "sippet/message/headers/accept.h"
#include "sippet/message/headers/accept_encoding.h"
#include "sippet/message/headers/accept_language.h"
#include "sippet/message/headers/alert_info.h"
#include "sippet/message/headers/allow.h"
#include "sippet/message/headers/authentication_info.h"
#include "sippet/message/headers/authorization.h"
#include "sippet/message/headers/call_id.h"
#include "sippet/message/headers/call_info.h"
#include "sippet/message/headers/contact.h"
#include "sippet/message/headers/content_disposition.h"
#include "sippet/message/headers/content_encoding.h"
#include "sippet/message/headers/content_language.h"
#include "sippet/message/headers/content_length.h"
#include "sippet/message/headers/content_type.h"
#include "sippet/message/headers/cseq.h"
#include "sippet/message/headers/date.h"
#include "sippet/message/headers/error_info.h"
#include "sippet/message/headers/expires.h"
#include "sippet/message/headers/from.h"
#include "sippet/message/headers/bits/has_multiple.h"
#include "sippet/message/headers/bits/has_parameters.h"
#include "sippet/message/headers/in_reply_to.h"
#include "sippet/message/headers/max_forwards.h"
#include "sippet/message/headers/mime_version.h"
#include "sippet/message/headers/min_expires.h"
#include "sippet/message/headers/organization.h"
#include "sippet/message/headers/priority.h"
#include "sippet/message/headers/proxy_authenticate.h"
#include "sippet/message/headers/proxy_authorization.h"
#include "sippet/message/headers/proxy_require.h"
#include "sippet/message/headers/record_route.h"
#include "sippet/message/headers/reply_to.h"
#include "sippet/message/headers/require.h"
#include "sippet/message/headers/retry_after.h"
#include "sippet/message/headers/route.h"
#include "sippet/message/headers/server.h"
#include "sippet/message/headers/subject.h"
#include "sippet/message/headers/supported.h"
#include "sippet/message/headers/timestamp.h"
#include "sippet/message/headers/to.h"
#include "sippet/message/headers/unsupported.h"
#include "sippet/message/headers/user_agent.h"
#include "sippet/message/headers/via.h"
#include "sippet/message/headers/warning.h"
#include "sippet/message/headers/www_authenticate.h"
#include "sippet/message/headers/generic.h"

namespace sippet {

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

Version ParseVersion(
    std::string::const_iterator line_begin,
    std::string::const_iterator line_end) {
  Tokenizer tok(line_begin, line_end);

  if ((line_end - line_begin < 3) ||
      !LowerCaseEqualsASCII(line_begin, line_begin + 3, "sip")) {
    DVLOG(1) << "missing status line";
    return Version();
  }

  tok.Skip(3);
  tok.Skip(HTTP_LWS);

  if (tok.EndOfInput()
      || *tok.current() != '/') {
    DVLOG(1) << "missing version";
    return Version();
  }

  tok.Skip();
  std::string::const_iterator major_start = tok.Skip(HTTP_LWS);
  tok.SkipTo('.');
  tok.Skip();
  std::string::const_iterator minor_start = tok.Skip(HTTP_LWS);
  if (tok.EndOfInput()) {
    DVLOG(1) << "malformed version";
    return Version();
  }

  if (!isdigit(*major_start) || !isdigit(*minor_start)) {
    DVLOG(1) << "malformed version number";
    return Version();
  }

  uint16 major = *major_start - '0';
  uint16 minor = *minor_start - '0';

  return Version(major, minor);
}

bool ParseStatusLine(
    std::string::const_iterator line_begin,
    std::string::const_iterator line_end,
    Version *version,
    int *response_code,
    std::string *reason_phrase) {
  // Extract the version number
  *version = ParseVersion(line_begin, line_end);
  if (*version == Version()) {
    DVLOG(1) << "invalid response";
    return false;
  }

  // Clamp the version number to {2.0}
  if (*version != Version(2, 0)) {
    *version = Version(2, 0);
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
    Atom<Method> *method,
    GURL *request_uri,
    Version *version) {

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
  if (*version == Version()) {
    DVLOG(1) << "invalid response";
    return false;
  }

  return true;
}

template<class HeaderType>
bool ParseToken(Tokenizer &tok, scoped_ptr<HeaderType> &header,
                void (*build)(scoped_ptr<HeaderType> &, const std::string &)) {
  std::string::const_iterator token_start = tok.Skip(HTTP_LWS);
  if (tok.EndOfInput()) {
    DVLOG(1) << "empty value";
    return false;
  }
  std::string token(token_start, tok.SkipNotIn(HTTP_LWS ";"));
  build(header, token);
  return true;
}

template<class HeaderType>
bool ParseTypeSubtype(Tokenizer &tok, scoped_ptr<HeaderType> &header,
                      void (*build)(scoped_ptr<HeaderType> &,
                                    const std::string &,
                                    const std::string &)) {
  std::string::const_iterator type_start = tok.Skip(HTTP_LWS);
  if (tok.EndOfInput()) {
    // empty header is OK
    return true;
  }
  std::string type(type_start, tok.SkipNotIn(HTTP_LWS "/"));
  if (!net::HttpUtil::IsToken(type)) {
    DVLOG(1) << "invalid token";
    return false;
  }

  tok.SkipTo('/');
  tok.Skip();
    
  std::string::const_iterator subtype_start = tok.Skip(HTTP_LWS);
  if (tok.EndOfInput()) {
    DVLOG(1) << "missing subtype";
    return false;
  }
  std::string subtype(subtype_start, tok.SkipNotIn(HTTP_LWS ";"));
  if (!net::HttpUtil::IsToken(subtype)) {
    DVLOG(1) << "invalid token";
    return false;
  }

  build(header, type, subtype);
  return true;
}

template<class HeaderType>
bool ParseParameters(Tokenizer &tok, scoped_ptr<HeaderType> &header,
                     void (*setter)(scoped_ptr<HeaderType> &,
                                    const std::string &,
                                    const std::string &)) {
  std::string::const_iterator param_start = tok.SkipTo(';');
  if (tok.EndOfInput())
    return true;
  tok.Skip();
  net::HttpUtil::NameValuePairsIterator it(tok.current(), tok.end(), ';');
  while (it.GetNext()) {
    setter(header, it.name(), it.value());
  }
  return true;
}

template<class HeaderType>
bool ParseAuthScheme(Tokenizer &tok, scoped_ptr<HeaderType> &header) {
  std::string::const_iterator scheme_start = tok.Skip(HTTP_LWS);
  if (tok.EndOfInput()) {
    DVLOG(1) << "missing authentication scheme";
    return false;
  }
  std::string scheme(scheme_start, tok.SkipNotIn(HTTP_LWS));
  header.reset(new HeaderType(scheme));
  return true;
}

template<class HeaderType>
bool ParseAuthParams(Tokenizer &tok, scoped_ptr<HeaderType> &header) {
  net::HttpUtil::NameValuePairsIterator it(tok.current(), tok.end(), ',');
  while (it.GetNext()) {
    header->param_set(it.name(), it.raw_value());
  }
  return true;
}

template<class HeaderType>
bool ParseUri(Tokenizer &tok, scoped_ptr<HeaderType> &header) {
  tok.SkipTo('<');
  if (tok.EndOfInput()) {
    DVLOG(1) << "invalid uri";
    return false;
  }
  std::string::const_iterator uri_start = tok.Skip();
  std::string::const_iterator uri_end = tok.SkipTo('>');
  if (tok.EndOfInput()) {
    DVLOG(1) << "unclosed '<'";
    return false;
  }
  tok.Skip();
  std::string uri(uri_start, uri_end);  
  header->push_back(HeaderType::value_type(GURL(uri)));
  return true;
}

template<class HeaderType>
bool ParseContact(Tokenizer &tok, scoped_ptr<HeaderType> &header,
                  void (*builder)(scoped_ptr<HeaderType> &,
                                  const GURL &,
                                  const std::string &)) {
  std::string display_name;
  GURL address;
  tok.Skip(HTTP_LWS);
  if (net::HttpUtil::IsQuote(*tok.current())) {
    // contact-param = quoted-string LAQUOT addr-spec RAQUOT
    std::string::const_iterator display_name_start = tok.current();
    tok.Skip();
    for (; !tok.EndOfInput(); tok.Skip()) {
      if (*tok.current() == '\\') {
        tok.Skip();
        continue;
      }
      if (net::HttpUtil::IsQuote(*tok.current()))
        break;
    }
    if (tok.EndOfInput()) {
      DVLOG(1) << "unclosed quoted-string";
      return false;
    }
    display_name.assign(display_name_start, tok.Skip());
    tok.SkipTo('<');
    if (tok.EndOfInput()) {
      DVLOG(1) << "missing address";
      return false;
    }
    std::string::const_iterator address_start = tok.Skip();
    tok.SkipTo('>');
    if (tok.EndOfInput()) {
      DVLOG(1) << "unclosed '<'";
      return false;
    }
    address = GURL(std::string(address_start, tok.current()));
  }
  else {
    Tokenizer laquot(tok.current(), tok.end());
    laquot.SkipTo('<');
    if (!laquot.EndOfInput()) {
      // contact-param = *(token LWS) LAQUOT addr-spec RAQUOT
      display_name.assign(tok.current(), laquot.current());
      TrimString(display_name, HTTP_LWS, &display_name);
      std::string::const_iterator address_start = laquot.Skip();
      laquot.SkipTo('>');
      if (laquot.EndOfInput()) {
        DVLOG(1) << "unclosed '<'";
        return false;
      }
      address = GURL(std::string(address_start, laquot.current()));
      tok.set_current(laquot.Skip());
    }
    else if (net::HttpUtil::IsToken(tok.current(), tok.current()+1)) {
      std::string::const_iterator address_start = tok.current();
      address = GURL(std::string(address_start, tok.SkipNotIn(HTTP_LWS ";")));
    }
    else {
      DVLOG(1) << "invalid char found";
      return false;
    }
  }

  display_name = net::HttpUtil::Unquote(display_name);
  builder(header, address, display_name);
  return true;
}

template<class HeaderType>
bool ParseStar(Tokenizer &tok, scoped_ptr<HeaderType> &header) {
  Tokenizer star(tok.current(), tok.end());
  star.Skip(HTTP_LWS);
  if (star.EndOfInput())
    return false;
  if (*star.current() != '*')
    return false;
  header.reset(new HeaderType(HeaderType::All));
  return true;
}

template<class HeaderType>
bool ParseWarning(Tokenizer &tok, scoped_ptr<HeaderType> &header,
                  void (*builder)(scoped_ptr<HeaderType> &,
                                  unsigned,
                                  const std::string &,
                                  const std::string &)) {
  std::string::const_iterator code_start = tok.Skip(HTTP_LWS);
  if (tok.EndOfInput()) {
    DVLOG(1) << "empty input";
    return false;
  }
  std::string code_string(code_start, tok.SkipNotIn(HTTP_LWS));
  int code = 0;
  if (!base::StringToInt(code_string, &code)
      || code < 100 || code > 999) {
    DVLOG(1) << "invalid code";
    return false;
  }
  std::string::const_iterator agent_start = tok.Skip(HTTP_LWS);
  if (tok.EndOfInput()) {
    DVLOG(1) << "empty warn-agent";
    return false;
  }
  std::string agent(agent_start, tok.SkipNotIn(HTTP_LWS));
  tok.Skip(HTTP_LWS);
  if (tok.EndOfInput()) {
    DVLOG(1) << "missing warn-text";
    return false;
  }
  if (!net::HttpUtil::IsQuote(*tok.current())) {
    DVLOG(1) << "invalid warn-text";
    return false;
  }
  std::string::const_iterator text_start = tok.current();
  tok.Skip();
  for (; !tok.EndOfInput(); tok.Skip()) {
    if (*tok.current() == '\\') {
      tok.Skip();
      continue;
    }
    if (net::HttpUtil::IsQuote(*tok.current()))
      break;
  }
  if (tok.EndOfInput()) {
    DVLOG(1) << "unclosed quoted-string";
    return false;
  }
  std::string text(text_start, tok.Skip());
  text = net::HttpUtil::Unquote(text);
  builder(header, static_cast<unsigned>(code), agent, text);
  return true;
}

template<class HeaderType>
bool ParseVia(Tokenizer &tok, scoped_ptr<HeaderType> &header,
              void (*builder)(scoped_ptr<HeaderType> &,
                              const Version &,
                              const std::string &,
                              const net::HostPortPair &)) {
  std::string::const_iterator version_start = tok.Skip(HTTP_LWS);
  if ((tok.end() - tok.current() < 3)
      || !LowerCaseEqualsASCII(tok.current(), tok.current() + 3, "sip")) {
    DVLOG(1) << "unknown SIP-version";
    return false;
  }
  tok.SkipTo('/');
  tok.Skip();
  if (tok.EndOfInput()) {
    DVLOG(1) << "missing SIP-version";
    return false;
  }
  Version version = ParseVersion(version_start, tok.SkipTo('/'));
  if (version < Version(2,0)) {
    DVLOG(1) << "invalid SIP-version";
    return false;
  }
  std::string::const_iterator protocol_start = tok.Skip();
  if (tok.EndOfInput()) {
    DVLOG(1) << "missing sent-protocol";
    return false;
  }
  std::string protocol(protocol_start, tok.SkipNotIn(HTTP_LWS));
  StringToUpperASCII(&protocol);
  std::string::const_iterator sentby_start = tok.Skip(HTTP_LWS);
  if (tok.EndOfInput()) {
    DVLOG(1) << "missing sent-by";
    return false;
  }
  std::string sentby_string(sentby_start, tok.SkipTo(';'));
  TrimString(sentby_string, HTTP_LWS, &sentby_string);
  if (sentby_string.empty()) {
    DVLOG(1) << "missing sent-by";
    return false;
  }
  std::string host;
  int port;
  if (!net::ParseHostAndPort(sentby_string, &host, &port)) {
    DVLOG(1) << "invalid sent-by";
    return false;
  }
  if (port == -1) {
    if (protocol == "UDP" || protocol == "TCP")
      port = 5060;
    else if (protocol == "TLS")
      port = 5061;
    else
      port = 0;
  }
  if (host[0] == '[') // remove brackets from IPv6 addresses
    host = host.substr(1, host.size()-2);
  net::HostPortPair sentby(host, port);
  builder(header, version, protocol, sentby);
  return true;
}

template<class HeaderType>
void SingleBuilder(scoped_ptr<HeaderType> &header,
                   const std::string &p1) {
  header.reset(new HeaderType(p1));
}

template<class HeaderType>
void SingleBuilder(scoped_ptr<HeaderType> &header,
                   const std::string &p1,
                   const std::string &p2) {
  header.reset(new HeaderType(p1, p2));
}

template<class HeaderType>
void SingleBuilder(scoped_ptr<HeaderType> &header,
                   const GURL &p1,
                   const std::string &p2) {
  header.reset(new HeaderType(p1, p2));
}

template<class HeaderType>
void SingleParamSetter(scoped_ptr<HeaderType> &header,
                       const std::string &key,
                       const std::string &value) {
  header->param_set(key, value);
}

template<class HeaderType>
void MultipleBuilder(scoped_ptr<HeaderType> &header,
                     const std::string &p1) {
  header->push_back(HeaderType::value_type(p1));
}

template<class HeaderType>
void MultipleBuilder(scoped_ptr<HeaderType> &header,
                     const std::string &p1,
                     const std::string &p2) {
  header->push_back(HeaderType::value_type(p1, p2));
}

template<class HeaderType>
void MultipleBuilder(scoped_ptr<HeaderType> &header,
                     const GURL &p1,
                     const std::string &p2) {
  header->push_back(HeaderType::value_type(p1, p2));
}

template<class HeaderType>
void MultipleBuilder(scoped_ptr<HeaderType> &header,
                     unsigned p1,
                     const std::string &p2,
                     const std::string &p3) {
  header->push_back(HeaderType::value_type(p1, p2, p3));
}

template<class HeaderType>
void MultipleBuilder(scoped_ptr<HeaderType> &header,
                     const Version &p1,
                     const std::string &p2,
                     const net::HostPortPair &p3) {
  header->push_back(HeaderType::value_type(p1, p2, p3));
}

template<class HeaderType>
void MultipleParamSetter(scoped_ptr<HeaderType> &header,
                         const std::string &key,
                         const std::string &value) {
  header->back().param_set(key, value);
}

template<class HeaderType>
scoped_ptr<Header> ParseSingleToken(
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  scoped_ptr<HeaderType> retval;
  Tokenizer tok(values_begin, values_end);
  if (!ParseToken(tok, retval, &SingleBuilder<HeaderType>))
    return scoped_ptr<Header>();
  return retval.Pass();
}

template<class HeaderType>
scoped_ptr<Header> ParseSingleTokenParams(
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  scoped_ptr<HeaderType> retval;
  Tokenizer tok(values_begin, values_end);
  if (!ParseToken(tok, retval, &SingleBuilder<HeaderType>))
    return scoped_ptr<Header>();
  return retval.Pass();
}

template<class HeaderType>
scoped_ptr<Header> ParseMultipleTokens(
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  scoped_ptr<HeaderType> retval(new HeaderType);
  net::HttpUtil::ValuesIterator it(values_begin, values_end, ',');
  while (it.GetNext()) {
    Tokenizer tok(it.value_begin(), it.value_end());
    if (!ParseToken(tok, retval, &MultipleBuilder<HeaderType>))
      return scoped_ptr<Header>();
  }
  return retval.Pass();
}

template<class HeaderType>
scoped_ptr<Header> ParseMultipleTokenParams(
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  scoped_ptr<HeaderType> retval(new HeaderType);
  net::HttpUtil::ValuesIterator it(values_begin, values_end, ',');
  while (it.GetNext()) {
    Tokenizer tok(it.value_begin(), it.value_end());
    if (!ParseToken(tok, retval, &MultipleBuilder<HeaderType>)
        || !ParseParameters(tok, retval, &MultipleParamSetter<HeaderType>))
      return scoped_ptr<Header>();
  }
  return retval.Pass();
}

template<class HeaderType>
scoped_ptr<Header> ParseSingleTypeSubtypeParams(
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  scoped_ptr<HeaderType> retval;
  Tokenizer tok(values_begin, values_end);
  if (!ParseTypeSubtype(tok, retval, &SingleBuilder<HeaderType>)
      || !ParseParameters(tok, retval, &SingleParamSetter<HeaderType>))
    return scoped_ptr<Header>();
  return retval.Pass();
}

template<class HeaderType>
scoped_ptr<Header> ParseMultipleTypeSubtypeParams(
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  scoped_ptr<HeaderType> retval(new HeaderType);
  net::HttpUtil::ValuesIterator it(values_begin, values_end, ',');
  while (it.GetNext()) {
    Tokenizer tok(it.value_begin(), it.value_end());
    if (!ParseTypeSubtype(tok, retval, &MultipleBuilder<HeaderType>)
        || !ParseParameters(tok, retval, &MultipleParamSetter<HeaderType>))
      return scoped_ptr<Header>();
  }
  return retval.Pass();
}

template<class HeaderType>
scoped_ptr<Header> ParseMultipleUriParams(
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  scoped_ptr<HeaderType> retval(new HeaderType);
  net::HttpUtil::ValuesIterator it(values_begin, values_end, ',');
  while (it.GetNext()) {
    Tokenizer tok(it.value_begin(), it.value_end());
    if (!ParseUri(tok, retval)
        || !ParseParameters(tok, retval, &MultipleParamSetter<HeaderType>))
      return scoped_ptr<Header>();
  }
  return retval.Pass();
}

template<class HeaderType>
scoped_ptr<Header> ParseSingleInteger(
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  Tokenizer tok(values_begin, values_end);
  std::string::const_iterator token_start = tok.Skip(HTTP_LWS);
  std::string digits(token_start, tok.SkipNotIn(HTTP_LWS));
  int output = 0;
  if (!base::StringToInt(digits, &output)) {
    DVLOG(1) << "invalid digits";
    return scoped_ptr<Header>();
  }
  unsigned integer = static_cast<unsigned>(output);
  scoped_ptr<HeaderType> header(new HeaderType(integer));
  return header.Pass();
}

template<class HeaderType>
scoped_ptr<Header> ParseOnlyAuthParams(
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  scoped_ptr<HeaderType> header(new HeaderType);
  Tokenizer tok(values_begin, values_end);
  if (!ParseAuthParams(tok, header))
      return scoped_ptr<Header>();
  return header.Pass();
}

template<class HeaderType>
scoped_ptr<Header> ParseSchemeAndAuthParams(
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  scoped_ptr<HeaderType> header;
  Tokenizer tok(values_begin, values_end);
  if (!ParseAuthScheme(tok, header)
      || !ParseAuthParams(tok, header))
      return scoped_ptr<Header>();
  return header.Pass();
}

template<class HeaderType>
scoped_ptr<Header> ParseSingleContactParams(
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  scoped_ptr<HeaderType> retval;
  Tokenizer tok(values_begin, values_end);
  if (!ParseContact(tok, retval, &SingleBuilder<HeaderType>)
      || !ParseParameters(tok, retval, &SingleParamSetter<HeaderType>))
    return scoped_ptr<Header>();
  return retval.Pass();
}

template<class HeaderType>
scoped_ptr<Header> ParseMultipleContactParams(
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  scoped_ptr<HeaderType> retval(new HeaderType);
  net::HttpUtil::ValuesIterator it(values_begin, values_end, ',');
  while (it.GetNext()) {
    Tokenizer tok(it.value_begin(), it.value_end());
    if (!ParseContact(tok, retval, &MultipleBuilder<HeaderType>)
        || !ParseParameters(tok, retval, &MultipleParamSetter<HeaderType>))
      return scoped_ptr<Header>();
  }
  return retval.Pass();
}

template<class HeaderType>
scoped_ptr<Header> ParseStarOrMultipleContactParams(
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  scoped_ptr<HeaderType> retval(new HeaderType);
  net::HttpUtil::ValuesIterator it(values_begin, values_end, ',');
  while (it.GetNext()) {
    Tokenizer tok(it.value_begin(), it.value_end());
    if (!ParseStar(tok, retval)) {
      if (!ParseContact(tok, retval, &MultipleBuilder<HeaderType>)
          || !ParseParameters(tok, retval, &MultipleParamSetter<HeaderType>))
        return scoped_ptr<Header>();
    }
  }
  return retval.Pass();
}

template<class HeaderType>
scoped_ptr<Header> ParseTrimmedUtf8(
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  std::string value(values_begin, values_end);
  TrimString(value, HTTP_LWS, &value);
  return scoped_ptr<HeaderType>(new HeaderType(value));
}

template<class HeaderType>
scoped_ptr<Header> ParseCseq(
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  scoped_ptr<HeaderType> retval;
  Tokenizer tok(values_begin, values_end);
  do {
    std::string::const_iterator integer_start = tok.Skip(HTTP_LWS);
    if (tok.EndOfInput()) {
      DVLOG(1) << "missing sequence";
      break;
    }
    std::string integer_string(integer_start, tok.SkipNotIn(HTTP_LWS));
    int sequence = 0;
    if (!base::StringToInt(integer_string, &sequence)) {
      DVLOG(1) << "invalid sequence";
      break;
    }
    std::string::const_iterator method_start = tok.Skip(HTTP_LWS);
    if (tok.EndOfInput()) {
      DVLOG(1) << "missing method";
      break;
    }
    std::string method_name(method_start, tok.SkipNotIn(HTTP_LWS));
    Atom<Method> method(method_name);
    retval.reset(new HeaderType(sequence, method));
  } while (false);
  return retval.Pass();
}

template<class HeaderType>
scoped_ptr<Header> ParseDate(
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  scoped_ptr<HeaderType> retval;
  do {
    std::string date(values_begin, values_end);
    base::Time parsed_time;
    if (!base::Time::FromString(date.c_str(), &parsed_time)) {
      DVLOG(1) << "invalid date spec";
      break;
    }
    retval.reset(new HeaderType(parsed_time));
  } while(false);
  return retval.Pass();
}

template<class HeaderType>
scoped_ptr<Header> ParseTimestamp(
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  scoped_ptr<HeaderType> retval;
  Tokenizer tok(values_begin, values_end);
  do {
    std::string::const_iterator timestamp_start = tok.Skip(HTTP_LWS);
    if (tok.EndOfInput()) {
      DVLOG(1) << "missing timestamp";
      break;
    }
    std::string timestamp_string(timestamp_start, tok.SkipNotIn(HTTP_LWS));
    double timestamp = .0;
    if (!base::StringToDouble(timestamp_string, &timestamp)) {
      DVLOG(1) << "invalid timestamp";
      break;
    }
    // delay is optional
    double delay = .0;
    std::string::const_iterator delay_start = tok.Skip(HTTP_LWS);
    if (!tok.EndOfInput()) {
      std::string delay_string(delay_start, tok.SkipNotIn(HTTP_LWS));
      base::StringToDouble(delay_string, &delay);
      // ignore errors parsing the optional delay
    }
    retval.reset(new HeaderType(timestamp, delay));
  } while(false);
  return retval.Pass();
}

template<class HeaderType>
scoped_ptr<Header> ParseMimeVersion(
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  scoped_ptr<HeaderType> retval;
  Tokenizer tok(values_begin, values_end);
  do {
    std::string::const_iterator major_start = tok.Skip(HTTP_LWS);
    if (tok.EndOfInput()) {
      DVLOG(1) << "missing major";
      break;
    }
    std::string major_string(major_start, tok.SkipTo('.'));
    int major = 0;
    if (major_string.empty()
        || !base::StringToInt(major_string, &major)) {
      DVLOG(1) << "missing or invalid major";
      break;
    }
    tok.Skip();
    std::string::const_iterator minor_start = tok.Skip(HTTP_LWS);
    std::string minor_string(minor_start, tok.end());
    int minor = 0;
    if (minor_string.empty()
        || !base::StringToInt(minor_string, &minor)) {
      DVLOG(1) << "invalid minor";
      break;
    }
    retval.reset(new HeaderType(static_cast<unsigned>(major),
                                static_cast<unsigned>(minor)));
  } while(false);
  return retval.Pass();
}

template<class HeaderType>
scoped_ptr<Header> ParseRetryAfter(
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  scoped_ptr<HeaderType> retval;
  Tokenizer tok(values_begin, values_end);
  do {
    std::string::const_iterator delta_start = tok.Skip(HTTP_LWS);
    if (tok.EndOfInput()) {
      DVLOG(1) << "missing delta-seconds";
      break;
    }
    std::string delta_string(delta_start, tok.SkipNotIn(HTTP_LWS "(;"));
    int delta_seconds = 0;
    if (delta_string.empty()
        || !base::StringToInt(delta_string, &delta_seconds)) {
      DVLOG(1) << "missing or invalid delta-seconds";
      break;
    }
    retval.reset(new HeaderType(static_cast<unsigned>(delta_seconds)));
    // ignoring comments
    tok.SkipTo(';');
    if (!tok.EndOfInput()) {
      ParseParameters(tok, retval, &SingleParamSetter<HeaderType>);
    }
  } while(false);
  return retval.Pass();
}

template<class HeaderType>
scoped_ptr<Header> ParseMultipleWarnings(
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  scoped_ptr<HeaderType> retval(new HeaderType);
  net::HttpUtil::ValuesIterator it(values_begin, values_end, ',');
  while (it.GetNext()) {
    Tokenizer tok(it.value_begin(), it.value_end());
    if (!ParseWarning(tok, retval, &MultipleBuilder<HeaderType>)) {
      return scoped_ptr<Header>();
    }
  }
  return retval.Pass();
}

template<class HeaderType>
scoped_ptr<Header> ParseMultipleVias(
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  scoped_ptr<HeaderType> retval(new HeaderType);
  net::HttpUtil::ValuesIterator it(values_begin, values_end, ',');
  while (it.GetNext()) {
    Tokenizer tok(it.value_begin(), it.value_end());
    if (!ParseVia(tok, retval, &MultipleBuilder<HeaderType>)
        || !ParseParameters(tok, retval, &MultipleParamSetter<HeaderType>)) {
      return scoped_ptr<Header>();
    }
  }
  return retval.Pass();
}

typedef scoped_ptr<Header> (*ParseFunction)(std::string::const_iterator,
                                            std::string::const_iterator);

// Attention here: those headers should be sorted
const ParseFunction parsers[] = {
#define X(class_name, compact_form, header_name, enum_name, format) \
  &Parse##format<class_name>,
#include "sippet/message/known_headers.h"
#undef X
};

scoped_ptr<Header> ParseHeader(
    std::string::const_iterator name_begin,
    std::string::const_iterator name_end,
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  scoped_ptr<Header> retval;
  std::string header_name(name_begin, name_end);
  Header::Type t = AtomTraits<Header::Type>::coerce(header_name.c_str());
  if (t == sippet::Header::HDR_GENERIC) {
    std::string header_name(name_begin, name_end);
    std::string header_value(values_begin, values_end);
    retval.reset(new sippet::Generic(header_name, header_value));
  }
  else {
    ParseFunction f = parsers[static_cast<Header::Type>(t)];
    return (*f)(values_begin, values_end);
  }
  return retval.Pass();
}

bool AssembleRawHeaders(const std::string &input, std::string *output) {
  Tokenizer tok(input.begin(), input.end());
  std::string::const_iterator line_start, line_end;
  
  output->reserve(input.size());
  for (;;) {
    line_start = tok.current();
    line_end = tok.SkipNotIn("\r\n");
    if (line_start != line_end)
      output->append(line_start, line_end);
    if (tok.EndOfInput())
      break;
    if (*tok.current() == '\n')
      tok.Skip(); // accept single LF
    else if (*tok.current() == '\r') {
      tok.Skip();
      if (*tok.current() == '\n')
        tok.Skip(); // default CRLF sequence
      else
        return false; // invalid CRLF sequence
    }
    if (tok.EndOfInput())
      break;
    if (!net::HttpUtil::IsLWS(*tok.current()))
      output->append(1, '\n'); // not line folding
  }
  
  return true;
}

} // End of empty namespace

scoped_ptr<Header> Header::Parse(const std::string &raw_header) {
  scoped_ptr<Header> header;
  net::HttpUtil::HeadersIterator it(raw_header.begin(),
    raw_header.end(), "\r\n");
  if (it.GetNext()) {
    header = ParseHeader(it.name_begin(), it.name_end(),
      it.values_begin(), it.values_end());
  }
  return header.Pass();
}

scoped_refptr<Message> Message::Parse(const std::string &raw_message) {
  std::string input;
  AssembleRawHeaders(raw_message, &input);

  scoped_refptr<Message> message;
  std::string::const_iterator i = input.begin();
  std::string::const_iterator end = input.end();
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
      Atom<Method> method;
      GURL request_uri;
      if (!ParseRequestLine(start, i, &method, &request_uri, &version))
        break;
      message = new Request(method, request_uri, version);
    }
  } while (false);

  // Jump over next CRLF
  if (i != end) {
    if (*i == '\r')
      ++i;
    if (i != end && *i == '\n')
      ++i;
  }

  if (message) {
    net::HttpUtil::HeadersIterator it(i, end, "\r\n");
    while (it.GetNext()) {
      scoped_ptr<Header> header =
        ParseHeader(it.name_begin(), it.name_end(),
                    it.values_begin(), it.values_end());
      if (header)
        message->push_back(header.Pass());
    }
  }

  return message;
}

} // End of sippet namespace
