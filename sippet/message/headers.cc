// Copyright (c) 2013-2017 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers.h"

#include "base/strings/string_number_conversions.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_util.h"
#include "base/time/time.h"
#include "sippet/message/sip_util.h"

namespace sippet {

namespace {

void CheckDoesNotHaveEmbededNulls(const std::string& str) {
  // Care needs to be taken when adding values to the raw headers string to
  // make sure it does not contain embeded NULLs. Any embeded '\0' may be
  // understood as line terminators and change how header lines get tokenized.
  CHECK(str.find('\0') == std::string::npos);
}

}  // namespace

struct Headers::ParsedHeader {
  // A header "continuation" contains only a subsequent value for the
  // preceding header.  (Header values are comma separated.)
  bool is_continuation() const { return name_begin == name_end; }

  std::string::const_iterator name_begin;
  std::string::const_iterator name_end;
  std::string::const_iterator value_begin;
  std::string::const_iterator value_end;
};

Headers::Headers()
  : response_code_(-1) {
}

Headers::~Headers() {}

bool Headers::Parse(const std::string& raw_input) {
  raw_headers_.reserve(raw_input.size());

  // ParseStartLine adds a normalized status line to raw_headers_
  std::string::const_iterator line_begin = raw_input.begin();
  std::string::const_iterator line_end =
      std::find(line_begin, raw_input.end(), '\0');
  if (!ParseStartLine(line_begin, line_end))
    return false;
  raw_headers_.push_back('\0');  // Terminate status line with a null.

  if (line_end == raw_input.end()) {
    raw_headers_.push_back('\0');  // Ensure the headers end with a double null.

    DCHECK_EQ('\0', raw_headers_[raw_headers_.size() - 2]);
    DCHECK_EQ('\0', raw_headers_[raw_headers_.size() - 1]);
    return true;
  }

  // Including a terminating null byte.
  size_t status_line_len = raw_headers_.size();

  // Now, we add the rest of the raw headers to raw_headers_, and begin parsing
  // it (to populate our parsed_ vector).
  raw_headers_.append(line_end + 1, raw_input.end());

  // Ensure the headers end with a double null.
  while (raw_headers_.size() < 2 ||
         raw_headers_[raw_headers_.size() - 2] != '\0' ||
         raw_headers_[raw_headers_.size() - 1] != '\0') {
    raw_headers_.push_back('\0');
  }

  // Adjust to point at the null byte following the status line
  line_end = raw_headers_.begin() + status_line_len - 1;

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

void Headers::RemoveHeader(const std::string& name) {
  // Copy up to the null byte.  This just copies the status line.
  std::string new_raw_headers(raw_headers_.c_str());
  new_raw_headers.push_back('\0');

  std::string lowercase_name = base::ToLowerASCII(name);
  HeaderSet to_remove;
  to_remove.insert(lowercase_name);
  MergeWithHeaders(new_raw_headers, to_remove);
}

void Headers::RemoveHeaders(
    const std::unordered_set<std::string>& header_names) {
  // Copy up to the null byte.  This just copies the status line.
  std::string new_raw_headers(raw_headers_.c_str());
  new_raw_headers.push_back('\0');

  HeaderSet to_remove;
  for (const auto& header_name : header_names) {
    to_remove.insert(base::ToLowerASCII(header_name));
  }
  MergeWithHeaders(new_raw_headers, to_remove);
}

void Headers::RemoveHeaderLine(const std::string& name,
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
  Parse(new_raw_headers);
}

void Headers::AddHeader(const std::string& header) {
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
  Parse(new_raw_headers);
}

bool Headers::GetNormalizedHeader(const std::string& name,
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

std::string Headers::GetStartLine() const {
  // copy up to the null byte.
  return std::string(raw_headers_.c_str());
}

bool Headers::EnumerateHeaderLines(size_t* iter,
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

bool Headers::EnumerateHeader(size_t* iter,
                              const base::StringPiece& name,
                              std::string* value) const {
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

  if (i == std::string::npos) {
    value->clear();
    return false;
  }

  if (iter)
    *iter = i + 1;
  value->assign(parsed_[i].value_begin, parsed_[i].value_end);
  return true;
}

bool Headers::HasHeaderValue(const base::StringPiece& name,
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

bool Headers::HasHeader(const base::StringPiece& name) const {
  return FindHeader(0, name) != std::string::npos;
}

void Headers::GetMimeTypeAndCharset(std::string* mime_type,
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

bool Headers::GetMimeType(std::string* mime_type) const {
  std::string unused;
  GetMimeTypeAndCharset(mime_type, &unused);
  return !mime_type->empty();
}

bool Headers::GetCharset(std::string* charset) const {
  std::string unused;
  GetMimeTypeAndCharset(&unused, charset);
  return !charset->empty();
}

bool Headers::GetTimeValuedHeader(const std::string& name,
    base::Time* result) const {
  std::string value;
  if (!EnumerateHeader(nullptr, name, &value))
    return false;

  // When parsing SIP dates it's beneficial to default to GMT.
  return base::Time::FromUTCString(value.c_str(), result);
}

int64_t Headers::GetContentLength() const {
  return GetInt64HeaderValue("content-length");
}

int64_t Headers::GetInt64HeaderValue(const std::string& header) const {
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

// Note: this implementation implicitly assumes that line_end points at a valid
// sentinel character (such as '\0').
// static
SipVersion Headers::ParseVersion(
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

// Note: this implementation implicitly assumes that line_end points at a valid
// sentinel character (such as '\0').
bool Headers::ParseStartLine(
    std::string::const_iterator line_begin,
    std::string::const_iterator line_end) {
  // Parse either a request line or a status line.
  if ((line_end - line_begin > 4)
      && base::LowerCaseEqualsASCII(
             base::StringPiece(line_begin, line_begin + 4), "sip/")) {
      return ParseStatusLine(line_begin, line_end);
  } else {
      return ParseRequestLine(line_begin, line_end);
  }
}

bool Headers::ParseRequestLine(std::string::const_iterator line_begin,
                               std::string::const_iterator line_end) {
  std::string::const_iterator p = std::find(line_begin, line_end, ' ');

  if (p == line_end) {
    DVLOG(1) << "missing method; rejecting";
    return false;
  }
  method_ = base::ToUpperASCII(base::StringPiece(line_begin, p));
  raw_headers_ = method_;

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
  raw_headers_.push_back(' ');
  raw_headers_.append(request_uri_.spec());

  // Skip whitespace.
  while (*p == ' ')
    ++p;

  // Extract the version number
  SipVersion parsed_sip_version = ParseVersion(p, line_end);

  // Clamp the version number to one of: {2.0}
  if (parsed_sip_version == SipVersion(2, 0)) {
    sip_version_ = SipVersion(2, 0);
    raw_headers_.append(" SIP/2.0");
  } else {
    // Ignore everything else
    DVLOG(1) << "rejecting SIP/" << parsed_sip_version.major_value() << "."
             << parsed_sip_version.minor_value();
    return false;
  }

  return true;
}

bool Headers::ParseStatusLine(std::string::const_iterator line_begin,
                              std::string::const_iterator line_end) {
  // Extract the version number
  SipVersion parsed_sip_version = ParseVersion(line_begin, line_end);

  // Clamp the version number to one of: {2.0}
  if (parsed_sip_version == SipVersion(2, 0)) {
    sip_version_ = SipVersion(2, 0);
    raw_headers_ = "SIP/2.0";
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
  raw_headers_.push_back(' ');
  raw_headers_.append(code, p);
  base::StringToInt(base::StringPiece(code, p), &response_code_);

  // Skip whitespace.
  while (p < line_end && *p == ' ')
    ++p;

  // Trim trailing whitespace.
  while (line_end > p && line_end[-1] == ' ')
    --line_end;

  if (p != line_end) {
    raw_headers_.push_back(' ');
    raw_headers_.append(p, line_end);
  }

  return true;
}

size_t Headers::FindHeader(size_t from,
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

void Headers::AddHeader(std::string::const_iterator name_begin,
                        std::string::const_iterator name_end,
                        std::string::const_iterator values_begin,
                        std::string::const_iterator values_end) {
  // If the header can be coalesced, then we should split it up.
  if (values_begin == values_end ||
      SipUtil::IsNonCoalescingHeader(name_begin, name_end)) {
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

void Headers::AddToParsed(std::string::const_iterator name_begin,
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

void Headers::MergeWithHeaders(const std::string& raw_headers,
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
  Parse(new_raw_headers);
}

}  // namespace sippet
