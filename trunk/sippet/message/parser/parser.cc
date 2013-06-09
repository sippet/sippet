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
  std::string::const_iterator p = line_begin;

  // RFC3261: SIP-Version = "SIP" "/" 1*DIGIT "." 1*DIGIT

  if ((line_end - p < 3) || !LowerCaseEqualsASCII(p, p + 3, "sip")) {
    DVLOG(1) << "missing status line";
    return Version();
  }

  p += 3;

  if (p >= line_end || *p != '/') {
    DVLOG(1) << "missing version";
    return Version();
  }

  std::string::const_iterator dot = std::find(p, line_end, '.');
  if (dot == line_end) {
    DVLOG(1) << "malformed version";
    return Version();
  }

  ++p;  // from / to first digit.
  ++dot;  // from . to second digit.

  if (!(*p >= '0' && *p <= '9' && *dot >= '0' && *dot <= '9')) {
    DVLOG(1) << "malformed version number";
    return Version();
  }

  uint16 major = *p - '0';
  uint16 minor = *dot - '0';

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
    Method *method,
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

template<class HeaderType, class ParseValue>
bool ParseMultipleValues(std::string::const_iterator values_begin,
                         std::string::const_iterator values_end,
                         scoped_ptr<HeaderType> &header,
                         ParseValue parse) {
  bool retval = true;
  net::HttpUtil::ValuesIterator it(values_begin, values_end, ',');
  while (it.GetNext()) {
    if (!parse(it.value_begin(), it.value_end(), header)) {
      retval = false;
      break;
    }
  }
  return retval;
}

template<class HeaderType>
void ParseParameters(std::string::const_iterator value_begin,
                     std::string::const_iterator value_end,
                     scoped_ptr<HeaderType> &header) {
  net::HttpUtil::NameValuePairsIterator it(value_begin, value_end, ';');
  while (it.GetNext()) {
    std::string name(it.name_begin(), it.name_end());
    std::string value(it.value_begin(), it.value_end());
    header->back().param_set(name, value);
  }
}

template<class HeaderType>
struct ParserTraits;

template<>
struct ParserTraits<Accept> {
  static scoped_ptr<Header> Interpret(std::string::const_iterator values_begin,
                                              std::string::const_iterator values_end) {
    scoped_ptr<Accept> accept(new Accept);
    if (!ParseMultipleValues(values_begin, values_end, accept, Parse))
      return scoped_ptr<Header>();
    return accept.Pass();
  }
private:
  static bool Parse(std::string::const_iterator value_begin,
                    std::string::const_iterator value_end,
                    scoped_ptr<Accept> &accept) {
    Tokenizer tok(value_begin, value_end);
    std::string::const_iterator type_start = tok.Skip(HTTP_LWS);
    if (tok.End()) {
      // empty header is OK
      return true;
    }
    std::string type(type_start, tok.SkipNotIn(HTTP_LWS "/"));

    tok.SkipTo('/');
    tok.Skip();
    
    std::string::const_iterator subtype_start = tok.Skip(HTTP_LWS);
    if (tok.End()) {
      DVLOG(1) << "missing subtype";
      return false;
    }
    std::string subtype(subtype_start, tok.SkipNotIn(HTTP_LWS ";"));

    accept->push_back(MediaRange(type, subtype));

    std::string::const_iterator param_start = tok.SkipTo(';');
    if (tok.End())
      return true;

    tok.Skip();
    ParseParameters(tok.Current(), value_end, accept);
    return true;
  }
};

template<class HeaderType, class ElemType>
struct ParseMultiToken {
  static scoped_ptr<Header> Interpret(std::string::const_iterator values_begin,
                                              std::string::const_iterator values_end) {
    scoped_ptr<HeaderType> header(new HeaderType);
    if (!ParseMultipleValues(values_begin, values_end, header, Parse))
      return scoped_ptr<Header>();
    return header.Pass();
  }
private:
  static bool Parse(std::string::const_iterator value_begin,
                    std::string::const_iterator value_end,
                    scoped_ptr<HeaderType> &header) {
    Tokenizer tok(value_begin, value_end);
    std::string::const_iterator token_start = tok.Skip(HTTP_LWS);
    if (tok.End()) {
      // empty header is OK
      return true;
    }
    std::string token(token_start, tok.SkipNotIn(HTTP_LWS ";"));

    header->push_back(ElemType(token));
    return true;
  }
};

template<class HeaderType, class ElemType>
struct ParseMultiTokenParams {
  static scoped_ptr<Header> Interpret(std::string::const_iterator values_begin,
                                              std::string::const_iterator values_end) {
    scoped_ptr<HeaderType> header(new HeaderType);
    if (!ParseMultipleValues(values_begin, values_end, header, Parse))
      return scoped_ptr<Header>();
    return header.Pass();
  }
private:
  static bool Parse(std::string::const_iterator value_begin,
                    std::string::const_iterator value_end,
                    scoped_ptr<HeaderType> &header) {
    Tokenizer tok(value_begin, value_end);
    std::string::const_iterator token_start = tok.Skip(HTTP_LWS);
    if (tok.End()) {
      // empty header is OK
      return true;
    }
    std::string token(token_start, tok.SkipNotIn(HTTP_LWS ";"));

    header->push_back(ElemType(token));

    std::string::const_iterator param_start = tok.SkipTo(';');
    if (tok.End())
      return true;

    tok.Skip();
    ParseParameters(tok.Current(), value_end, header);
    return true;
  }
};

template<>
struct ParserTraits<AcceptEncoding> {
  static scoped_ptr<Header> Interpret(std::string::const_iterator values_begin,
                                              std::string::const_iterator values_end) {
    return ParseMultiTokenParams<AcceptEncoding, Encoding>::Interpret(values_begin, values_end);
  }
};

template<>
struct ParserTraits<AcceptLanguage> {
  static scoped_ptr<Header> Interpret(std::string::const_iterator values_begin,
                                              std::string::const_iterator values_end) {
    return ParseMultiTokenParams<AcceptLanguage, LanguageRange>::Interpret(values_begin, values_end);
  }
};

template<>
struct ParserTraits<Allow> {
  static scoped_ptr<Header> Interpret(std::string::const_iterator values_begin,
                                      std::string::const_iterator values_end) {
    return ParseMultiToken<Allow, Method>::Interpret(values_begin, values_end);
  }
};

template<>
struct ParserTraits<ContentEncoding> {
  static scoped_ptr<Header> Interpret(std::string::const_iterator values_begin,
                                      std::string::const_iterator values_end) {
    return ParseMultiToken<ContentEncoding, std::string>::Interpret(values_begin, values_end);
  }
};

template<>
struct ParserTraits<ContentLanguage> {
  static scoped_ptr<Header> Interpret(std::string::const_iterator values_begin,
                                      std::string::const_iterator values_end) {
    return ParseMultiToken<ContentLanguage, std::string>::Interpret(values_begin, values_end);
  }
};

template<>
struct ParserTraits<InReplyTo> {
  static scoped_ptr<Header> Interpret(std::string::const_iterator values_begin,
                                      std::string::const_iterator values_end) {
    return ParseMultiToken<InReplyTo, std::string>::Interpret(values_begin, values_end);
  }
};

template<>
struct ParserTraits<ProxyRequire> {
  static scoped_ptr<Header> Interpret(std::string::const_iterator values_begin,
                                      std::string::const_iterator values_end) {
    return ParseMultiToken<ProxyRequire, std::string>::Interpret(values_begin, values_end);
  }
};

template<>
struct ParserTraits<Require> {
  static scoped_ptr<Header> Interpret(std::string::const_iterator values_begin,
                                      std::string::const_iterator values_end) {
    return ParseMultiToken<Require, std::string>::Interpret(values_begin, values_end);
  }
};

template<>
struct ParserTraits<Supported> {
  static scoped_ptr<Header> Interpret(std::string::const_iterator values_begin,
                                      std::string::const_iterator values_end) {
    return ParseMultiToken<Supported, std::string>::Interpret(values_begin, values_end);
  }
};

template<>
struct ParserTraits<Unsupported> {
  static scoped_ptr<Header> Interpret(std::string::const_iterator values_begin,
                                      std::string::const_iterator values_end) {
    return ParseMultiToken<Unsupported, std::string>::Interpret(values_begin, values_end);
  }
};

scoped_ptr<Header>
  ParseAccept(std::string::const_iterator values_begin,
              std::string::const_iterator values_end) {
  return ParserTraits<Accept>::Interpret(values_begin, values_end);
}

scoped_ptr<Header>
  ParseAcceptEncoding(std::string::const_iterator values_begin,
                      std::string::const_iterator values_end) {
  return ParserTraits<AcceptEncoding>::Interpret(values_begin, values_end);
}

scoped_ptr<Header>
  ParseAcceptLanguage(std::string::const_iterator values_begin,
                      std::string::const_iterator values_end) {
  return ParserTraits<AcceptLanguage>::Interpret(values_begin, values_end);
}

scoped_ptr<Header>
  ParseAlertInfo(std::string::const_iterator values_begin,
                 std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseAllow(std::string::const_iterator values_begin,
             std::string::const_iterator values_end) {
  return ParserTraits<Allow>::Interpret(values_begin, values_end);
}

scoped_ptr<Header>
  ParseAuthenticationInfo(std::string::const_iterator values_begin,
                          std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseAuthorization(std::string::const_iterator values_begin,
                     std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseCallId(std::string::const_iterator values_begin,
              std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseCallInfo(std::string::const_iterator values_begin,
                std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseContact(std::string::const_iterator values_begin,
               std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseContentDisposition(std::string::const_iterator values_begin,
                          std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseContentEncoding(std::string::const_iterator values_begin,
                       std::string::const_iterator values_end) {
  return ParserTraits<ContentEncoding>::Interpret(values_begin, values_end);
}

scoped_ptr<Header>
  ParseContentLanguage(std::string::const_iterator values_begin,
                       std::string::const_iterator values_end) {
  return ParserTraits<ContentLanguage>::Interpret(values_begin, values_end);
}

scoped_ptr<Header>
  ParseContentLength(std::string::const_iterator values_begin,
                     std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseContentType(std::string::const_iterator values_begin,
                   std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseCseq(std::string::const_iterator values_begin,
            std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseDate(std::string::const_iterator values_begin,
            std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseErrorInfo(std::string::const_iterator values_begin,
                 std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseExpires(std::string::const_iterator values_begin,
               std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseFrom(std::string::const_iterator values_begin,
            std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseInReplyTo(std::string::const_iterator values_begin,
                 std::string::const_iterator values_end) {
  return ParserTraits<InReplyTo>::Interpret(values_begin, values_end);
}

scoped_ptr<Header>
  ParseMaxForwards(std::string::const_iterator values_begin,
                   std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseMinExpires(std::string::const_iterator values_begin,
                  std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseMimeVersion(std::string::const_iterator values_begin,
                   std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseOrganization(std::string::const_iterator values_begin,
                    std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParsePriority(std::string::const_iterator values_begin,
                std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseProxyAuthenticate(std::string::const_iterator values_begin,
                         std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseProxyAuthorization(std::string::const_iterator values_begin,
                          std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseProxyRequire(std::string::const_iterator values_begin,
                    std::string::const_iterator values_end) {
  return ParserTraits<ProxyRequire>::Interpret(values_begin, values_end);
}

scoped_ptr<Header>
  ParseRecordRoute(std::string::const_iterator values_begin,
                   std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseReplyTo(std::string::const_iterator values_begin,
               std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseRequire(std::string::const_iterator values_begin,
               std::string::const_iterator values_end) {
  return ParserTraits<Require>::Interpret(values_begin, values_end);
}

scoped_ptr<Header>
  ParseRetryAfter(std::string::const_iterator values_begin,
                  std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseRoute(std::string::const_iterator values_begin,
             std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseServer(std::string::const_iterator values_begin,
              std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseSubject(std::string::const_iterator values_begin,
               std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseSupported(std::string::const_iterator values_begin,
                 std::string::const_iterator values_end) {
  return ParserTraits<Supported>::Interpret(values_begin, values_end);
}

scoped_ptr<Header>
  ParseTimestamp(std::string::const_iterator values_begin,
                 std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseTo(std::string::const_iterator values_begin,
          std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseUnsupported(std::string::const_iterator values_begin,
                   std::string::const_iterator values_end) {
  return ParserTraits<Unsupported>::Interpret(values_begin, values_end);
}

scoped_ptr<Header>
  ParseUserAgent(std::string::const_iterator values_begin,
                 std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseVia(std::string::const_iterator values_begin,
           std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseWarning(std::string::const_iterator values_begin,
               std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

scoped_ptr<Header>
  ParseWwwAuthenticate(std::string::const_iterator values_begin,
                       std::string::const_iterator values_end) {
  return scoped_ptr<Header>();
}

typedef scoped_ptr<Header> (*ParseFunction)(std::string::const_iterator,
                                                    std::string::const_iterator);

struct HeaderMap {
  const char *header_name;
  Header::Type header_type;
  ParseFunction parse_function;
};

// Attention here: those headers should be sorted
const HeaderMap headers[] = {
  {
    "accept",
    Header::HDR_ACCEPT,
    &ParseAccept
  },
  {
    "accept-encoding",
    Header::HDR_ACCEPT_ENCODING,
    &ParseAcceptEncoding
  },
  {
    "accept-language",
    Header::HDR_ACCEPT_LANGUAGE,
    &ParseAcceptLanguage
  },
  {
    "alert-info",
    Header::HDR_ALERT_INFO,
    &ParseAlertInfo
  },
  {
    "allow",
    Header::HDR_ALLOW,
    &ParseAllow
  },
  {
    "authentication-info",
    Header::HDR_AUTHENTICATION_INFO,
    &ParseAuthenticationInfo
  },
  {
    "authorization",
    Header::HDR_AUTHORIZATION,
    &ParseAuthorization
  },
  {
    "call-id",
    Header::HDR_CALL_ID,
    &ParseCallId
  },
  {
    "call-info",
    Header::HDR_CALL_INFO,
    &ParseCallInfo
  },
  {
    "contact",
    Header::HDR_CONTACT,
    &ParseContact
  },
  {
    "content-disposition",
    Header::HDR_CONTENT_DISPOSITION,
    &ParseContentDisposition
  },
  {
    "content-encoding",
    Header::HDR_CONTENT_ENCODING,
    &ParseContentEncoding
  },
  {
    "content-language",
    Header::HDR_CONTENT_LANGUAGE,
    &ParseContentLanguage
  },
  {
    "content-length",
    Header::HDR_CONTENT_LENGTH,
    &ParseContentLength
  },
  {
    "content-type",
    Header::HDR_CONTENT_TYPE,
    &ParseContentType
  },
  {
    "cseq",
    Header::HDR_CSEQ,
    &ParseCseq
  },
  {
    "date",
    Header::HDR_DATE,
    &ParseDate
  },
  {
    "error-info",
    Header::HDR_ERROR_INFO,
    &ParseErrorInfo
  },
  {
    "expires",
    Header::HDR_EXPIRES,
    &ParseExpires
  },
  {
    "from",
    Header::HDR_FROM,
    &ParseFrom
  },
  {
    "in-reply-to",
    Header::HDR_IN_REPLY_TO,
    &ParseInReplyTo
  },
  {
    "max-forwards",
    Header::HDR_MAX_FORWARDS,
    &ParseMaxForwards
  },
  {
    "mime-version",
    Header::HDR_MIME_VERSION,
    &ParseMimeVersion
  },
  {
    "min-expires",
    Header::HDR_MIN_EXPIRES,
    &ParseMinExpires
  },
  {
    "organization",
    Header::HDR_ORGANIZATION,
    &ParseOrganization
  },
  {
    "priority",
    Header::HDR_PRIORITY,
    &ParsePriority
  },
  {
    "proxy-authenticate",
    Header::HDR_PROXY_AUTHENTICATE,
    &ParseProxyAuthenticate
  },
  {
    "proxy-authorization",
    Header::HDR_PROXY_AUTHORIZATION,
    &ParseProxyAuthorization
  },
  {
    "proxy-require",
    Header::HDR_PROXY_REQUIRE,
    &ParseProxyRequire
  },
  {
    "record-route",
    Header::HDR_RECORD_ROUTE,
    &ParseRecordRoute
  },
  {
    "reply-to",
    Header::HDR_REPLY_TO,
    &ParseReplyTo
  },
  {
    "require",
    Header::HDR_REQUIRE,
    &ParseRequire
  },
  {
    "retry-after",
    Header::HDR_RETRY_AFTER,
    &ParseRetryAfter
  },
  {
    "route",
    Header::HDR_ROUTE,
    &ParseRoute
  },
  {
    "server",
    Header::HDR_SERVER,
    &ParseServer
  },
  {
    "subject",
    Header::HDR_SUBJECT,
    &ParseSubject
  },
  {
    "supported",
    Header::HDR_SUPPORTED,
    &ParseSupported
  },
  {
    "timestamp",
    Header::HDR_TIMESTAMP,
    &ParseTimestamp
  },
  {
    "to",
    Header::HDR_TO,
    &ParseTo
  },
  {
    "unsupported",
    Header::HDR_UNSUPPORTED,
    &ParseUnsupported
  },
  {
    "user-agent",
    Header::HDR_USER_AGENT,
    &ParseUserAgent
  },
  {
    "via",
    Header::HDR_VIA,
    &ParseVia
  },
  {
    "warning",
    Header::HDR_WARNING,
    &ParseWarning
  },
  {
    "www-authenticate",
    Header::HDR_WWW_AUTHENTICATE,
    &ParseWwwAuthenticate
  },
};

const int headers_size = sizeof(headers) / sizeof(headers[0]);

bool HeaderMapLess(const HeaderMap &a, const HeaderMap &b) {
  const char *end = a.header_name + strlen(a.header_name);
  return LowerCaseEqualsASCII(a.header_name, end, b.header_name);
}

Header::Type HeaderNameToType(
    std::string::const_iterator name_begin,
    std::string::const_iterator name_end) {
  // Perform a simple binary search
  std::string header(name_begin, name_end);
  const HeaderMap elem = { header.c_str() };
  const HeaderMap *first = headers;
  const HeaderMap *last = headers + headers_size;

  first = std::lower_bound(first, last, elem, HeaderMapLess);
  return first != last ? first->header_type : Header::HDR_GENERIC;
}
      
scoped_ptr<Header> ParseHeader(
    Header::Type t,
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  int index = static_cast<Header::Type>(t);
  const HeaderMap &elem = headers[index];
  return (*elem.parse_function)(values_begin, values_end);
}

} // End of empty namespace

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
    net::HttpUtil::HeadersIterator it(i, end, "\r\n");
    while (it.GetNext()) {
      scoped_ptr<Header> header;
      Header::Type t = HeaderNameToType(it.name_begin(), it.name_end());
      if (t == sippet::Header::HDR_GENERIC) {
        std::string header_name(it.name_begin(), it.name_end());
        std::string header_value(it.values_begin(), it.values_end());
        header.reset(new sippet::Generic(header_name, header_value));
      }
      else {
        header = ParseHeader(t, it.values_begin(), it.values_end());
      }
      if (header) {
        message->push_back(header.Pass());
      }
    }
  }

  return message;
}

} // End of sippet namespace
