// Copyright (c) 2013-2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/message.h"

#include "base/strings/string_number_conversions.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_util.h"
#include "base/strings/string_tokenizer.h"
#include "base/time/time.h"
#include "net/base/parse_number.h"
#include "net/base/url_util.h"
#include "sippet/message/sip_util.h"
#include "sippet/message/request.h"
#include "sippet/message/response.h"

namespace sippet {

namespace {

void CheckDoesNotHaveEmbededNulls(const std::string& str) {
  // Care needs to be taken when adding values to the raw headers string to
  // make sure it does not contain embeded NULLs. Any embeded '\0' may be
  // understood as line terminators and change how header lines get tokenized.
  CHECK(str.find('\0') == std::string::npos);
}

void ReadNormalizingParameterNames(
    std::string::const_iterator param_begin,
    std::string::const_iterator param_end,
    std::unordered_map<std::string, std::string>* parameters) {
  SipUtil::NameValuePairsIterator pairs(param_begin, param_end, ';',
      SipUtil::NameValuePairsIterator::Values::NOT_REQUIRED,
      SipUtil::NameValuePairsIterator::Quotes::STRICT_QUOTES);
  while (pairs.GetNext()) {
    (*parameters)[base::ToLowerASCII(pairs.name())] = pairs.value();
  }
}

}  // namespace

const char Message::kTcp[] = "TCP";
const char Message::kTls[] = "TLS";
const char Message::kUdp[] = "UDP";
const char Message::kWs[] = "WS";
const char Message::kWss[] = "WSS";

struct Message::ParsedHeader {
  // A header "continuation" contains only a subsequent value for the
  // preceding header.  (Header values are comma separated.)
  bool is_continuation() const { return name_begin == name_end; }

  std::string::const_iterator name_begin;
  std::string::const_iterator name_end;
  std::string::const_iterator value_begin;
  std::string::const_iterator value_end;
};

Message::Message() {}

Message::~Message() {}

// static
scoped_refptr<Message> Message::Parse(const std::string& raw_input) {
  // Note: this implementation implicitly assumes that line_end points at a
  // valid sentinel character (such as '\0').

  // Parse either a request line or a status line.
  scoped_refptr<Message> message;
  if (raw_input.size() > 4 &&
          base::LowerCaseEqualsASCII(
              base::StringPiece(raw_input.data(), 4), "sip/")) {
      message = new Response;
  } else {
      message = new Request;
  }
  if (!message->ParseInternal(raw_input))
    return nullptr;
  return message;
}

void Message::Update(const Message& new_message) {
  // Copy up to the null byte.  This just copies the start-line.
  std::string new_raw_headers(raw_headers_.c_str());
  new_raw_headers.push_back('\0');

  HeaderSet updated_headers;

  // NOTE: we write the new headers then the old headers for convenience.  The
  // order should not matter.

  // Figure out which headers we want to take from new_message:
  for (size_t i = 0; i < new_message.parsed_.size(); ++i) {
    const HeaderList& new_parsed = new_message.parsed_;

    DCHECK(!new_parsed[i].is_continuation());

    // Locate the start of the next header.
    size_t k = i;
    while (++k < new_parsed.size() && new_parsed[k].is_continuation()) {}
    --k;

    base::StringPiece name(new_parsed[i].name_begin, new_parsed[i].name_end);
    if (HasHeader(name)) {
      std::string name_lower = base::ToLowerASCII(name);
      updated_headers.insert(name_lower);

      // Preserve this header line in the merged result, making sure there is
      // a null after the value.
      if (name_lower == "cseq" && IsRequest()) {
        // When merging the CSeq header, keep the method.
        int64_t sequence = new_message.GetCSeq(nullptr);
        new_raw_headers.append(new_parsed[i].name_begin,
            new_parsed[i].name_end);
        new_raw_headers.append(": " + base::Int64ToString(sequence));
        new_raw_headers.push_back(' ');
        new_raw_headers.append(as_request()->request_method());
      } else {
        new_raw_headers.append(new_parsed[i].name_begin,
            new_parsed[k].value_end);
      }
      new_raw_headers.push_back('\0');
    }

    i = k;
  }

  // Now, build the new raw headers.
  MergeWithMessage(new_raw_headers, updated_headers);
}

void Message::RemoveHeader(const std::string& name) {
  // Copy up to the null byte.  This just copies the status line.
  std::string new_raw_headers(raw_headers_.c_str());
  new_raw_headers.push_back('\0');

  std::string lowercase_name = base::ToLowerASCII(name);
  HeaderSet to_remove;
  to_remove.insert(lowercase_name);
  MergeWithMessage(new_raw_headers, to_remove);
}

void Message::RemoveHeaders(
    const std::unordered_set<std::string>& header_names) {
  // Copy up to the null byte.  This just copies the status line.
  std::string new_raw_headers(raw_headers_.c_str());
  new_raw_headers.push_back('\0');

  HeaderSet to_remove;
  for (const auto& header_name : header_names) {
    to_remove.insert(base::ToLowerASCII(header_name));
  }
  MergeWithMessage(new_raw_headers, to_remove);
}

void Message::RemoveHeaderLine(const std::string& name,
    const std::string& value) {
  std::string name_lowercase = base::ToLowerASCII(name);

  std::string new_raw_headers(GetStartLine());
  new_raw_headers.push_back('\0');

  new_raw_headers.reserve(raw_headers_.size());

  size_t iter = 0;
  std::string old_header_name;
  std::string old_header_value;
  while (EnumerateHeaderLines(&iter, &old_header_name, &old_header_value)) {
    std::string old_header_name_lowercase = base::ToLowerASCII(old_header_name);
    if (name_lowercase == old_header_name_lowercase &&
        value == old_header_value)
      continue;

    new_raw_headers.append(old_header_name);
    new_raw_headers.push_back(':');
    new_raw_headers.push_back(' ');
    new_raw_headers.append(old_header_value);
    new_raw_headers.push_back('\0');
  }
  new_raw_headers.push_back('\0');

  // Make this object hold the new data.
  raw_headers_.clear();
  parsed_.clear();
  ParseInternal(new_raw_headers);
}

void Message::AddHeader(const std::string& header) {
  CheckDoesNotHaveEmbededNulls(header);
  DCHECK_EQ('\0', raw_headers_[raw_headers_.size() - 2]);
  DCHECK_EQ('\0', raw_headers_[raw_headers_.size() - 1]);
  // Don't copy the last null.
  std::string new_raw_headers(raw_headers_, 0, raw_headers_.size() - 1);
  new_raw_headers.append(header);
  new_raw_headers.push_back('\0');
  new_raw_headers.push_back('\0');

  // Make this object hold the new data.
  raw_headers_.clear();
  parsed_.clear();
  ParseInternal(new_raw_headers);
}

bool Message::GetNormalizedHeader(const std::string& name,
    std::string* value) const {
  // If you hit this assertion, please use EnumerateHeader instead!
  DCHECK(!SipUtil::IsNonCoalescingHeader(name));

  value->clear();

  bool found = false;
  size_t i = 0;
  while (i < parsed_.size()) {
    i = FindHeader(i, name);
    if (i == std::string::npos)
      break;

    found = true;

    if (!value->empty())
      value->append(", ");

    std::string::const_iterator value_begin = parsed_[i].value_begin;
    std::string::const_iterator value_end = parsed_[i].value_end;
    while (++i < parsed_.size() && parsed_[i].is_continuation())
      value_end = parsed_[i].value_end;
    value->append(value_begin, value_end);
  }

  return found;
}

std::string Message::GetStartLine() const {
  // copy up to the null byte.
  return std::string(raw_headers_.c_str());
}

bool Message::EnumerateHeaderLines(size_t* iter,
                                   std::string* name,
                                   std::string* value) const {
  size_t i = *iter;
  if (i == parsed_.size())
    return false;

  DCHECK(!parsed_[i].is_continuation());

  name->assign(parsed_[i].name_begin, parsed_[i].name_end);

  std::string::const_iterator value_begin = parsed_[i].value_begin;
  std::string::const_iterator value_end = parsed_[i].value_end;
  while (++i < parsed_.size() && parsed_[i].is_continuation())
    value_end = parsed_[i].value_end;

  value->assign(value_begin, value_end);

  *iter = i;
  return true;
}

bool Message::EnumerateHeader(size_t* iter,
                              const base::StringPiece& name,
                              std::string::const_iterator* value_begin,
                              std::string::const_iterator* value_end) const {
  size_t i;
  if (!iter || !*iter) {
    i = FindHeader(0, name);
  } else {
    i = *iter;
    if (i >= parsed_.size()) {
      i = std::string::npos;
    } else if (!parsed_[i].is_continuation()) {
      i = FindHeader(i, name);
    }
  }

  if (i == std::string::npos)
    return false;

  if (iter)
    *iter = i + 1;
  *value_begin = parsed_[i].value_begin;
  *value_end = parsed_[i].value_end;
  return true;
}

bool Message::EnumerateHeader(size_t* iter,
                              const base::StringPiece& name,
                              std::string* value) const {
  std::string::const_iterator value_begin, value_end;
  if (EnumerateHeader(iter, name, &value_begin, &value_end)) {
    value->assign(value_begin, value_end);
    return true;
  } else {
    value->clear();
    return false;
  }
}

bool Message::HasHeaderValue(const base::StringPiece& name,
                             const base::StringPiece& value) const {
  // The value has to be an exact match.  This is important since
  // 'cache-control: no-cache' != 'cache-control: no-cache="foo"'
  size_t iter = 0;
  std::string temp;
  while (EnumerateHeader(&iter, name, &temp)) {
    if (base::EqualsCaseInsensitiveASCII(value, temp))
      return true;
  }
  return false;
}

bool Message::HasHeader(const base::StringPiece& name) const {
  return FindHeader(0, name) != std::string::npos;
}

void Message::GetMimeTypeAndCharset(std::string* mime_type,
                                    std::string* charset) const {
  mime_type->clear();
  charset->clear();

  std::string name = "content-type";
  std::string value;

  bool had_charset = false;

  size_t iter = 0;
  while (EnumerateHeader(&iter, name, &value))
    SipUtil::ParseContentType(value, mime_type, charset, &had_charset, NULL);
}

bool Message::GetMimeType(std::string* mime_type) const {
  std::string unused;
  GetMimeTypeAndCharset(mime_type, &unused);
  return !mime_type->empty();
}

bool Message::GetCharset(std::string* charset) const {
  std::string unused;
  GetMimeTypeAndCharset(&unused, charset);
  return !charset->empty();
}

bool Message::GetTimeValuedHeader(const std::string& name,
    base::Time* result) const {
  std::string value;
  if (!EnumerateHeader(nullptr, name, &value))
    return false;

  // When parsing SIP dates it's beneficial to default to GMT.
  return base::Time::FromUTCString(value.c_str(), result);
}

int64_t Message::GetContentLength() const {
  return GetInt64HeaderValue("content-length");
}

int64_t Message::GetMaxForwards() const {
  return GetInt64HeaderValue("max-forwards");
}

int64_t Message::GetInt64HeaderValue(const std::string& header) const {
  size_t iter = 0;
  std::string content_length_val;
  if (!EnumerateHeader(&iter, header, &content_length_val))
    return -1;

  if (content_length_val.empty())
    return -1;

  if (content_length_val[0] == '+')
    return -1;

  int64_t result;
  bool ok = base::StringToInt64(content_length_val, &result);
  if (!ok || result < 0)
    return -1;

  return result;
}

bool Message::GetFrom(
    std::string* display_name,
    GURL* address,
    std::unordered_map<std::string, std::string>* parameters) const {
  return EnumerateContactLikeHeader(nullptr, "from", display_name, address,
      parameters);
}

bool Message::GetTo(
    std::string* display_name,
    GURL* address,
    std::unordered_map<std::string, std::string>* parameters) const {
  return EnumerateContactLikeHeader(nullptr, "to", display_name, address,
      parameters);
}

bool Message::GetReplyTo(
    std::string* display_name,
    GURL* address,
    std::unordered_map<std::string, std::string>* parameters) const {
  return EnumerateContactLikeHeader(nullptr, "reply-to", display_name, address,
      parameters);
}

bool Message::EnumerateContact(
    size_t* iter,
    std::string* display_name,
    GURL* address,
    std::unordered_map<std::string, std::string>* parameters) const {
  return EnumerateContactLikeHeader(iter, "contact", display_name, address,
      parameters);
}

bool Message::EnumerateRoute(
    size_t* iter,
    GURL* address,
    std::unordered_map<std::string, std::string>* parameters) const {
  return EnumerateContactLikeHeader(iter, "route", nullptr, address,
      parameters);
}

bool Message::EnumerateRecordRoute(
    size_t* iter,
    GURL* address,
    std::unordered_map<std::string, std::string>* parameters) const {
  return EnumerateContactLikeHeader(iter, "record-route", nullptr, address,
      parameters);
}

bool Message::EnumerateContactLikeHeader(
    size_t* iter,
    const base::StringPiece& name,
    std::string* display_name,
    GURL* address,
    std::unordered_map<std::string, std::string>* parameters) const {
  std::string::const_iterator value_begin, value_end;
  if (!EnumerateHeader(iter, name, &value_begin, &value_end))
      return false;

  if (display_name)
    display_name->clear();
  *address = GURL();
  if (parameters)
    parameters->clear();

  // The contact-like headers are already normalized, so we don't need to parse
  // special cases as 'Contact: sip:foo@bar;parameters' or
  // 'Contact: Mr. Magoo <sip:foo@bar;parameters>'.
  bool next_is_param = false;
  base::StringTokenizer t(value_begin, value_end, "; ");
  t.set_quote_chars("\"");
  t.set_options(base::StringTokenizer::RETURN_DELIMS);
  while (t.GetNext()) {
    if (t.token_is_delim()) {
      switch (*t.token_begin()) {
        case ';':
          next_is_param = true;
          break;
      }
    } else {
      base::StringPiece token(t.token_piece());
      if (next_is_param) {
        if (parameters) {
          ReadNormalizingParameterNames(t.token_begin(), value_end,
              parameters);
        }
        break;
      } else if (token[0] == '"') {
        display_name->assign(token.begin() + 1, token.end() - 1);
      } else if (token[0] == '<') {
        *address = GURL(std::string(token.begin() + 1, token.end() - 1));
      } else {
        NOTREACHED() << "Logical error";
      }
    }
  }
  return true;
}

int64_t Message::GetCSeq(std::string* method) const {
  std::string::const_iterator value_begin, value_end;
  if (!EnumerateHeader(nullptr, "cseq", &value_begin, &value_end))
    return -1;

  base::StringTokenizer t(value_begin, value_end, " ");
  if (!t.GetNext())
    return -1;

  // Parse the sequence as 1*DIGIT.
  int64_t sequence;
  if (!net::ParseInt64(t.token(), net::ParseIntFormat::NON_NEGATIVE, &sequence)) {
    // If the sequence value cannot fit in a int64_t, return false.
    return -1;
  }

  if (!t.GetNext())
    return -1;

  if (method)
    *method = base::ToUpperASCII(t.token());
  return sequence;
}

bool Message::GetExpiresValue(base::TimeDelta* value) const {
  return GetTimeDeltaValue("expires", value);
}

bool Message::GetMinExpiresValue(base::TimeDelta* value) const {
  return GetTimeDeltaValue("min-expires", value);
}

bool Message::GetTimeDeltaValue(const base::StringPiece& name,
                                base::TimeDelta* result) const {
  std::string value;
  if (!EnumerateHeader(nullptr, name, &value))
    return false;

  // Parse the delta-seconds as 1*DIGIT.
  uint32_t seconds;
  net::ParseIntError error;
  if (!net::ParseUint32(value, &seconds, &error)) {
    if (error == net::ParseIntError::FAILED_OVERFLOW) {
      // If the Age value cannot fit in a uint32_t, saturate it to a maximum
      // value. This is similar to what RFC 2616 says in section 14.6 for how
      // caches should transmit values that overflow.
      seconds = std::numeric_limits<decltype(seconds)>::max();
    } else {
      return false;
    }
  }

  *result = base::TimeDelta::FromSeconds(seconds);
  return true;
}

void Message::SetViaReceived(const std::string& received) {
  DCHECK_EQ('\0', raw_headers_[raw_headers_.size() - 2]);
  DCHECK_EQ('\0', raw_headers_[raw_headers_.size() - 1]);

  std::string new_raw_headers(GetStartLine());
  new_raw_headers.push_back('\0');

  size_t iter = 0;
  std::string name;
  std::string value;
  bool is_first = true;
  while (EnumerateHeaderLines(&iter, &name, &value)) {
    if (base::EqualsCaseInsensitiveASCII(name, "via") && is_first) {
      std::string::iterator param_begin =
          std::find(value.begin(), value.end(), ';');
      if (param_begin != value.end()) {
        std::string new_value(value.begin(), param_begin);
        SipUtil::NameValuePairsIterator pairs(
            param_begin, value.end(), ';',
            SipUtil::NameValuePairsIterator::Values::NOT_REQUIRED,
            SipUtil::NameValuePairsIterator::Quotes::STRICT_QUOTES);
        while (pairs.GetNext()) {
          if (base::EqualsCaseInsensitiveASCII(
                base::StringPiece(pairs.name_begin(), pairs.name_end()),
                "received")) {
            continue;
          }
          new_value.append(";" + pairs.name() + "=" + pairs.value());
        }
        value.swap(new_value);
      }
      value.append(";received=" + received);
      is_first = false;
    }
    new_raw_headers.append(name + ": " + value);
    new_raw_headers.push_back('\0');
  }
  new_raw_headers.push_back('\0');

  // Make this object hold the new data.
  raw_headers_.clear();
  parsed_.clear();
  ParseInternal(new_raw_headers);
}

bool Message::EnumerateVia(
    size_t* iter,
    std::string* protocol,
    net::HostPortPair* sent_by,
    std::unordered_map<std::string, std::string>* parameters) const {
  std::string::const_iterator value_begin, value_end;
  if (!EnumerateHeader(iter, "via", &value_begin, &value_end))
      return false;

  std::string via_protocol;
  bool had_sip = false, had_version = false, had_protocol = false;
  bool had_sent_by = false, next_is_param = false;
  base::StringTokenizer t(value_begin, value_end, "/ ;");
  t.set_quote_chars("\"");
  t.set_options(base::StringTokenizer::RETURN_DELIMS);
  while (t.GetNext()) {
    if (t.token_is_delim()) {
      switch (*t.token_begin()) {
        case ';':
          next_is_param = true;
          break;
      }
    } else {
      base::StringPiece token(t.token_piece());
      if (next_is_param) {
        if (parameters) {
          ReadNormalizingParameterNames(t.token_begin(), value_end,
              parameters);
        }
        break;
      } else if (!had_sip) {
        had_sip = true;
      } else if (!had_version) {
        had_version = true;
      } else if (!had_protocol) {
        via_protocol = token.as_string();
        had_protocol = true;
      } else if (!had_sent_by) {
        if (sent_by) {
          std::string host;
          int port;
          net::ParseHostAndPort(token.as_string(), &host, &port);
          if (port == -1) {
            if (via_protocol == "UDP" || via_protocol == "TCP")
              port = 5060;
            else if (via_protocol == "TLS")
              port = 5061;
            else
              port = 0;
          }
          if (host[0] == '[')  // remove brackets from IPv6 addresses
            host = host.substr(1, host.size()-2);
          *sent_by = net::HostPortPair(host, port);
        }
        had_sent_by = true;
      } else {
        NOTREACHED() << "Logical error";
      }
    }
  }

  if (protocol)
    protocol->assign(via_protocol);
  return true;
}

bool Message::GetBranch(std::string* branch) const {
  std::unordered_map<std::string, std::string> parameters;
  if (!EnumerateVia(nullptr, nullptr, nullptr, &parameters))
    return false;
  auto it = parameters.find("branch");
  if (it == parameters.end())
    return false;
  *branch = it->second;
  return true;
}

bool Message::GetSentBy(net::HostPortPair* sent_by) const {
  return EnumerateVia(nullptr, nullptr, sent_by, nullptr);
}

std::string Message::client_key() const {
  CHECK(HasHeader("via"));
  std::string key("C->"), method, branch;
  if (IsRequest()) {
    method = as_request()->request_method();
  } else {
    GetCSeq(&method);
  }
  GetBranch(&branch);
  key.append(branch);
  key.push_back(':');
  key.append(method);
  return key;
}

std::string Message::server_key() const {
  CHECK(HasHeader("via"));
  std::string key("S->"), method, branch;
  net::HostPortPair sent_by;
  if (IsRequest()) {
    method = as_request()->request_method();
  } else {
    GetCSeq(&method);
  }
  GetBranch(&branch);
  GetSentBy(&sent_by);
  key.append(branch);
  key.push_back(':');
  key.append(method);
  key.push_back(':');
  key.append(sent_by.ToString());
  return key;
}

Request* Message::as_request() {
  return IsRequest() ? static_cast<Request*>(this) : nullptr;
}

Response* Message::as_response() {
  return IsResponse() ? static_cast<Response*>(this) : nullptr;
}

const Request* Message::as_request() const {
  return IsRequest() ? static_cast<const Request*>(this) : nullptr;
}

const Response* Message::as_response() const {
  return IsResponse() ? static_cast<const Response*>(this) : nullptr;
}

std::string Message::ToString() const {
  std::string result(GetStartLine() + "\r\n");

  for (size_t i = 0; i < parsed_.size(); ++i) {
    DCHECK(!parsed_[i].is_continuation());

    // Locate the start of the next header.
    size_t k = i;
    while (++k < parsed_.size() && parsed_[k].is_continuation()) {}
    --k;

    result.append(parsed_[i].name_begin, parsed_[k].value_end);
    result.append("\r\n");

    i = k;
  }
  result.append("\r\n");

  return result;
}

bool Message::NormalizeHeaders(std::string::const_iterator headers_begin,
                               std::string::const_iterator headers_end) {
  SipUtil::HeadersIterator headers(headers_begin, headers_end,
                                   std::string(1, '\0'));
  while (headers.GetNext()) {
    base::StringPiece header_name(headers.name_begin(), headers.name_end());
    if (header_name.size() == 1) {
      const char* long_name = SipUtil::ExpandHeader(*headers.name_begin());
      if (long_name)
        header_name = base::StringPiece(long_name);
    }
    raw_headers_.append(header_name.begin(), header_name.end());
    raw_headers_.append(": ");
    if (SipUtil::IsContactLikeHeader(header_name)) {
      if (!NormalizeContactLikeHeader(headers.values_begin(),
            headers.values_end()))
        return false;
    } else if (base::LowerCaseEqualsASCII(header_name, "contact")) {
      if (headers.values_end() - headers.values_begin() == 1
          && *headers.values_begin() == '*') {
        raw_headers_.push_back('*');
      } else {
        if (!NormalizeContactLikeHeader(headers.values_begin(),
              headers.values_end()))
          return false;
      }
    } else if (base::LowerCaseEqualsASCII(header_name, "via")) {
      if (!NormalizeViaHeader(headers.values_begin(),
            headers.values_end()))
        return false;
    } else {
      raw_headers_.append(headers.values_begin(), headers.values_end());
    }
    raw_headers_.push_back('\0');
  }

  // Ensure the compact_headers end with a double null.
  raw_headers_.push_back('\0');
  return true;
}

bool Message::NormalizeContactLikeHeader(
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  bool next_is_param = false;
  bool had_quoted_string = false, had_address = false, had_token = false;
  base::StringTokenizer t(values_begin, values_end, "; ,");
  t.set_quote_chars("\"");
  t.set_options(base::StringTokenizer::RETURN_DELIMS);
  while (t.GetNext()) {
    if (t.token_is_delim()) {
      switch (*t.token_begin()) {
        case ';':
          next_is_param = true;
          break;
        case ',':
          // Reset state
          next_is_param = false;
          had_quoted_string = had_address = had_token = false;
          raw_headers_.append(", ");
          break;
      }
    } else {
      base::StringPiece token(t.token_piece());
      if (token.empty())
        continue;
      if (next_is_param) {
        raw_headers_.push_back(';');
        raw_headers_.append(token.begin(), token.end());
      } else if (token[0] == '"') {
        if (had_quoted_string) {
          DVLOG(1) << "repeated name";
          return false;
        }
        if (token[1] != '"')
          raw_headers_.append(token.begin(), token.end());
        had_quoted_string = true;
      } else if (token[0] == '<') {
        if (had_address) {
          DVLOG(1) << "repeated addr-spec";
          return false;
        }
        if (had_token) {
          raw_headers_.append("\" ");
        } else if (had_quoted_string) {
          raw_headers_.push_back(' ');
        }
        raw_headers_.append(token.begin(), token.end());
        had_address = true;
      } else {
        if (had_quoted_string || had_address) {
          DVLOG(1) << "malformed contact-like header";
          return false;
        }
        if (token.starts_with("sip:") || token.starts_with("sips:")) {
          raw_headers_.push_back('<');
          raw_headers_.append(token.begin(), token.end());
          raw_headers_.push_back('>');
          had_address = true;
        } else {
          if (!had_token)
            raw_headers_.push_back('"');
          else
            raw_headers_.push_back(' ');
          raw_headers_.append(token.begin(), token.end());
          had_token = true;
        }
      }
    }
  }
  return true;
}

bool Message::NormalizeViaHeader(
    std::string::const_iterator values_begin,
    std::string::const_iterator values_end) {
  bool had_sip = false, had_version = false, had_protocol = false;
  bool had_sent_by = false, next_is_param = false;
  base::StringTokenizer t(values_begin, values_end, "/ ;,");
  t.set_quote_chars("\"");
  t.set_options(base::StringTokenizer::RETURN_DELIMS);
  while (t.GetNext()) {
    if (t.token_is_delim()) {
      switch (*t.token_begin()) {
        case ';':
          next_is_param = true;
          break;
        case ',':
          // Reset state
          next_is_param = false;
          had_sip = had_version = had_protocol = had_sent_by = false;
          raw_headers_.append(", ");
          break;
      }
    } else {
      base::StringPiece token(t.token_piece());
      if (next_is_param) {
        raw_headers_.push_back(';');
        raw_headers_.append(token.begin(), token.end());
      } else if (!had_sip) {
        if (!base::LowerCaseEqualsASCII(token, "sip")) {
          DVLOG(1) << "bad protocol-name";
          return false;
        }
        raw_headers_.append("SIP/");
        had_sip = true;
      } else if (!had_version) {
        if (!base::LowerCaseEqualsASCII(token, "2.0")) {
          DVLOG(1) << "bad protocol-version";
          return false;
        }
        raw_headers_.append("2.0/");
        had_version = true;
      } else if (!had_protocol) {
        raw_headers_.append(base::ToUpperASCII(token));
        had_protocol = true;
      } else if (!had_sent_by) {
        raw_headers_.push_back(' ');
        raw_headers_.append(token.begin(), token.end());
        had_sent_by = true;
      } else {
        DVLOG(1) << "malformed via-parm";
        return false;
      }
    }
  }
  return true;
}

bool Message::ParseInternal(const std::string& raw_input) {
  raw_headers_.reserve(raw_input.size());

  // ParseStartLine adds a normalized status line to raw_headers_
  std::string::const_iterator line_begin = raw_input.begin();
  std::string::const_iterator line_end =
      std::find(line_begin, raw_input.end(), '\0');
  if (!ParseStartLine(line_begin, line_end, &raw_headers_))
    return false;
  raw_headers_.push_back('\0');  // Terminate status line with a null.

  if (line_end == raw_input.end()) {
    raw_headers_.push_back('\0');  // Ensure the headers end with a double null.

    DCHECK_EQ('\0', raw_headers_[raw_headers_.size() - 2]);
    DCHECK_EQ('\0', raw_headers_[raw_headers_.size() - 1]);
    return true;
  }

  // Including a terminating null byte.
  size_t start_line_len = raw_headers_.size();

  // Expand compact headers in raw_headers_.
  if (!NormalizeHeaders(line_end + 1, raw_input.end()))
    return false;

  // Adjust to point at the null byte following the status line
  line_end = raw_headers_.begin() + start_line_len - 1;

  SipUtil::HeadersIterator headers(line_end + 1, raw_headers_.end(),
                                   std::string(1, '\0'));
  while (headers.GetNext()) {
    AddHeader(headers.name_begin(),
              headers.name_end(),
              headers.values_begin(),
              headers.values_end());
  }

  DCHECK_EQ('\0', raw_headers_[raw_headers_.size() - 2]);
  DCHECK_EQ('\0', raw_headers_[raw_headers_.size() - 1]);
  return true;
}

// Note: this implementation implicitly assumes that line_end points at a valid
// sentinel character (such as '\0').
// static
SipVersion Message::ParseVersion(
    std::string::const_iterator line_begin,
    std::string::const_iterator line_end) {
  std::string::const_iterator p = line_begin;

  // RFC3261: SIP-Version   = "SIP" "/" 1*DIGIT "." 1*DIGIT

  if (!base::StartsWith(base::StringPiece(line_begin, line_end), "sip",
                        base::CompareCase::INSENSITIVE_ASCII)) {
    DVLOG(1) << "missing version";
    return SipVersion();
  }

  p += 3;

  if (p >= line_end || *p != '/') {
    DVLOG(1) << "missing version";
    return SipVersion();
  }

  std::string::const_iterator dot = std::find(p, line_end, '.');
  if (dot == line_end) {
    DVLOG(1) << "malformed version";
    return SipVersion();
  }

  ++p;  // from / to first digit.
  ++dot;  // from . to second digit.

  if (!(base::IsAsciiDigit(*p) && base::IsAsciiDigit(*dot))) {
    DVLOG(1) << "malformed version number";
    return SipVersion();
  }

  uint16_t major = *p - '0';
  uint16_t minor = *dot - '0';

  return SipVersion(major, minor);
}

size_t Message::FindHeader(size_t from,
    const base::StringPiece& search) const {
  for (size_t i = from; i < parsed_.size(); ++i) {
    if (parsed_[i].is_continuation())
      continue;
    base::StringPiece name(parsed_[i].name_begin, parsed_[i].name_end);
    if (base::EqualsCaseInsensitiveASCII(search, name))
      return i;
  }

  return std::string::npos;
}

void Message::AddHeader(std::string::const_iterator name_begin,
                        std::string::const_iterator name_end,
                        std::string::const_iterator values_begin,
                        std::string::const_iterator values_end) {
  // If the header can be coalesced, then we should split it up.
  if (values_begin == values_end ||
      SipUtil::IsNonCoalescingHeader(
          base::StringPiece(name_begin, name_end))) {
    AddToParsed(name_begin, name_end, values_begin, values_end);
  } else {
    SipUtil::ValuesIterator it(values_begin, values_end, ',');
    while (it.GetNext()) {
      AddToParsed(name_begin, name_end, it.value_begin(), it.value_end());
      // clobber these so that subsequent values are treated as continuations
      name_begin = name_end = raw_headers_.end();
    }
  }
}

void Message::AddToParsed(std::string::const_iterator name_begin,
                          std::string::const_iterator name_end,
                          std::string::const_iterator value_begin,
                          std::string::const_iterator value_end) {
  ParsedHeader header;
  header.name_begin = name_begin;
  header.name_end = name_end;
  header.value_begin = value_begin;
  header.value_end = value_end;
  parsed_.push_back(header);
}

void Message::MergeWithMessage(const std::string& raw_headers,
                               const HeaderSet& headers_to_remove) {
  std::string new_raw_headers(raw_headers);
  for (size_t i = 0; i < parsed_.size(); ++i) {
    DCHECK(!parsed_[i].is_continuation());

    // Locate the start of the next header.
    size_t k = i;
    while (++k < parsed_.size() && parsed_[k].is_continuation()) {}
    --k;

    std::string name = base::ToLowerASCII(
        base::StringPiece(parsed_[i].name_begin, parsed_[i].name_end));
    if (headers_to_remove.find(name) == headers_to_remove.end()) {
      // It's ok to preserve this header in the final result.
      new_raw_headers.append(parsed_[i].name_begin, parsed_[k].value_end);
      new_raw_headers.push_back('\0');
    }

    i = k;
  }
  new_raw_headers.push_back('\0');

  // Make this object hold the new data.
  raw_headers_.clear();
  parsed_.clear();
  ParseInternal(new_raw_headers);
}

}  // namespace sippet
