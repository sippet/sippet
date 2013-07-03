// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/uri/uri.h"
#include "sippet/uri/uri_parse.h"
#include "sippet/uri/uri_canon.h"
#include "sippet/uri/uri_util.h"
#include "base/logging.h"

namespace sippet {

namespace {

// External template that can handle initialization of either character type.
// The input spec is given, and the canonical version will be placed in
// |*canonical|, along with the parsing of the canonical spec in |*parsed|.
template<typename STR>
bool InitCanonical(const STR& input_spec,
                   std::string* canonical,
                   uri_parse::Parsed* parsed) {
  // Reserve enough room in the output for the input, plus some extra so that
  // we have room if we have to escape a few things without reallocating.
  canonical->reserve(input_spec.size() + 32);
  url_canon::StdStringCanonOutput output(canonical);
  bool success = uri_util::Canonicalize(
      input_spec.data(), static_cast<int>(input_spec.length()),
      NULL, &output, parsed);

  output.Complete();  // Must be done before using string.
  return success;
}

} // End of empty namespace

URI::URI() : is_valid_(false) {
}

URI::URI(const GURL &other) {
  is_valid_ = InitCanonical(
    other.possibly_invalid_spec(), &spec_, &parsed_);
}

URI::URI(const URI& other) : spec_(other.spec_),
  is_valid_(other.is_valid_),
  parsed_(other.parsed_) {
}

URI::URI(const std::string& uri_string) {
  is_valid_ = InitCanonical(uri_string, &spec_, &parsed_);
}

URI::URI(const string16& uri_string) {
  is_valid_ = InitCanonical(uri_string, &spec_, &parsed_);
}

URI::URI(const char* canonical_spec, size_t canonical_spec_len,
         const uri_parse::Parsed& parsed, bool is_valid)
    : spec_(canonical_spec, canonical_spec_len),
      is_valid_(is_valid),
      parsed_(parsed) {
#ifndef NDEBUG
  // For testing purposes, check that the parsed canonical URI is identical to
  // what we would have produced. Skip checking for invalid URIs have no meaning
  // and we can't always canonicalize then reproducabely.
  if (is_valid_) {
    uri_parse::Component scheme;
    if (scheme.begin == parsed.scheme.begin) {
      URI test_url(spec_);

      DCHECK(test_url.is_valid_ == is_valid_);
      DCHECK(test_url.spec_ == spec_);

      DCHECK(test_url.parsed_.scheme == parsed_.scheme);
      DCHECK(test_url.parsed_.username == parsed_.username);
      DCHECK(test_url.parsed_.password == parsed_.password);
      DCHECK(test_url.parsed_.host == parsed_.host);
      DCHECK(test_url.parsed_.port == parsed_.port);
      DCHECK(test_url.parsed_.parameters == parsed_.parameters);
      DCHECK(test_url.parsed_.headers == parsed_.headers);
    }
  }
#endif
}

URI::~URI() {
}

URI& URI::operator=(const URI& other) {
  spec_ = other.spec_;
  is_valid_ = other.is_valid_;
  parsed_ = other.parsed_;
  return *this;
}

URI URI::ConvertToSIP() {
  // TODO
  return *this;
}

const std::string& URI::spec() const {
  static std::string empty;
  if (is_valid_ || spec_.empty())
    return spec_;

  DCHECK(false) << "Trying to get the spec of an invalid URL!";
  return empty;
}

bool URI::Equivalent(const URI& other) {
  // TODO
  return false;
}

URI URI::GetWithEmptyHeaders() const {
  // This doesn't make sense for invalid or nonstandard URIs, so return
  // the empty URL.
  if (!is_valid_ || !IsStandard())
    return URI();

  // We could optimize this since we know that the URL is canonical, and we are
  // appending a canonical path, so avoiding re-parsing.
  URI other(*this);
  if (parsed_.headers.len == 0)
    return other;

  // Clear the headers.
  other.parsed_.headers.reset();
  return other;
}

URI URI::GetOrigin() const {
  // This doesn't make sense for invalid or nonstandard URLs, so return
  // the empty URL.
  if (!is_valid_ || !IsStandard())
    return URI();

  std::string spec;
  spec += scheme();
  spec += ":";
  spec += host();
  if (has_port())
    spec += port();

  return URI(spec);
}

bool URI::IsStandard() const {
  return SchemeIs("sip")
    || SchemeIs("sips")
    || SchemeIs("tel");
}

bool URI::SchemeIs(const char* lower_ascii_scheme) const {
  if (parsed_.scheme.len <= 0)
    return lower_ascii_scheme == NULL;
  return uri_util::LowerCaseEqualsASCII(spec_.data() + parsed_.scheme.begin,
                                        spec_.data() + parsed_.scheme.end(),
                                        lower_ascii_scheme);
}

bool URI::HostIsIPAddress() const {
  if (!is_valid_ || spec_.empty())
     return false;

  url_canon::RawCanonOutputT<char, 128> ignored_output;
  uri_canon::CanonHostInfo host_info;
  uri_canon::CanonicalizeIPAddress(spec_.c_str(), parsed_.host,
                                   &ignored_output, &host_info);
  return host_info.IsIPAddress();
}

int URI::IntPort() const {
  if (parsed_.port.is_nonempty())
    return ParsePort(spec_.data(), parsed_.port);
  return uri_parse::PORT_UNSPECIFIED;
}

int URI::EffectiveIntPort() const {
  int int_port = IntPort();
  if (int_port == uri_parse::PORT_UNSPECIFIED)
    return uri_canon::DefaultPortForScheme(spec_.data() + parsed_.scheme.begin,
                                           parsed_.scheme.len);
  return int_port;
}

std::string URI::HostNoBrackets() const {
  // If host looks like an IPv6 literal, strip the square brackets.
  uri_parse::Component h(parsed_.host);
  if (h.len >= 2 && spec_[h.begin] == '[' && spec_[h.end() - 1] == ']') {
    h.begin++;
    h.len -= 2;
  }
  return ComponentString(h);
}

bool URI::DomainIs(const char* lower_ascii_domain, int domain_len) const {
  // Return false if this URL is not valid or domain is empty.
  if (!is_valid_ || !domain_len)
    return false;

  // TEL-URIs have empty parsed_.host, so check this first.
  if (SchemeIsTel())
    return false;

  if (!parsed_.host.is_nonempty())
    return false;

  // Check whether the host name is end with a dot. If yes, treat it
  // the same as no-dot unless the input comparison domain is end
  // with dot.
  const char* last_pos = spec_.data() + parsed_.host.end() - 1;
  int host_len = parsed_.host.len;
  if ('.' == *last_pos && '.' != lower_ascii_domain[domain_len - 1]) {
    last_pos--;
    host_len--;
  }

  // Return false if host's length is less than domain's length.
  if (host_len < domain_len)
    return false;

  // Compare this url whether belong specific domain.
  const char* start_pos = spec_.data() + parsed_.host.begin +
                          host_len - domain_len;

  if (!uri_util::LowerCaseEqualsASCII(start_pos,
                                      last_pos + 1,
                                      lower_ascii_domain,
                                      lower_ascii_domain + domain_len))
    return false;

  // Check whether host has right domain start with dot, make sure we got
  // right domain range. For example www.google.com has domain
  // "google.com" but www.iamnotgoogle.com does not.
  if ('.' != lower_ascii_domain[0] && host_len > domain_len &&
      '.' != *(start_pos - 1))
    return false;

  return true;
}

void URI::Swap(URI* other) {
  spec_.swap(other->spec_);
  std::swap(is_valid_, other->is_valid_);
  std::swap(parsed_, other->parsed_);
}

const URI& URI::EmptyURI() {
  static URI empty;
  return empty;
}

} // End of sippet namespace

std::ostream& operator<<(std::ostream& out, const sippet::URI& url) {
  return out << url.possibly_invalid_spec();
}
