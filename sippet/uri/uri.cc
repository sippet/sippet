// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/uri/uri.h"
#include "sippet/base/stl_extras.h"
#include "base/logging.h"

#include <sstream>

namespace sippet {

namespace {

// External template that can handle initialization of either character type.
// The input spec is given, and the canonical version will be placed in
// |*canonical|, along with the parsing of the canonical spec in |*parsed|.
template<typename STR>
bool InitCanonical(const STR& input_spec,
                   std::string* canonical,
                   uri::Parsed* parsed) {
  // Reserve enough room in the output for the input, plus some extra so that
  // we have room if we have to escape a few things without reallocating.
  canonical->reserve(input_spec.size() + 32);
  url::StdStringCanonOutput output(canonical);
  bool success = uri::Canonicalize(
      input_spec.data(), static_cast<int>(input_spec.length()),
      nullptr, &output, parsed);

  output.Complete();  // Must be done before using string.
  return success;
}

bool SchemeIs(const std::string& spec,
              const uri::Parsed& parsed,
              const char* lower_ascii_scheme) {
  return uri::LowerCaseEqualsASCII(spec.data() + parsed.scheme.begin,
                                        spec.data() + parsed.scheme.end(),
                                        lower_ascii_scheme);
}

template<typename STR>
bool InitCanonicalSipURI(const STR& input_spec,
                         std::string* canonical,
                         uri::Parsed* parsed) {
  bool success = true;
  if (!InitCanonical(input_spec, canonical, parsed)
      || (!SchemeIs(*canonical, *parsed, "sip")
          && !SchemeIs(*canonical, *parsed, "sips"))) {
    canonical->clear();
    *parsed = uri::Parsed();
    success = false;
  }
  return success;
}

template<typename STR>
bool InitCanonicalTelURI(const STR& input_spec,
                         std::string* canonical,
                         uri::Parsed* parsed) {
  bool success = true;
  if (!InitCanonical(input_spec, canonical, parsed)
      || !SchemeIs(*canonical, *parsed, "tel")) {
    canonical->clear();
    *parsed = uri::Parsed();
    success = false;
  }
  return success;
}

std::pair<bool, std::string> LookupKeyValue(
    const std::string &spec,
    const std::string &name,
    uri::Component query,
    bool (*extractor)(const char*, uri::Component*,
        uri::Component*, uri::Component*)) {
  uri::Component key, value;
  while ((*extractor)(
          spec.data(), &query, &key, &value)) {
      std::string unescaped_key(
          uri_details::UnescapedComponentString(spec, key));
      if (unescaped_key.length() != name.length())
        continue;
      if (base::strncasecmp(name.data(),
          unescaped_key.data(), unescaped_key.length()) == 0)
        return std::make_pair(true,
            uri_details::UnescapedComponentString(spec, value));
  }
  return std::make_pair(false, "");
}

} // End of empty namespace

// SipURI --------------------------------------------------------------------

SipURI::SipURI() : is_valid_(false) {
}

SipURI::SipURI(const GURL &other) {
  is_valid_ = InitCanonicalSipURI(
    other.possibly_invalid_spec(), &spec_, &parsed_);
}

SipURI::SipURI(const SipURI& other) : spec_(other.spec_),
  is_valid_(other.is_valid_),
  parsed_(other.parsed_) {
}

SipURI::SipURI(const std::string& uri_string) {
  is_valid_ = InitCanonicalSipURI(uri_string, &spec_, &parsed_);
}

SipURI::SipURI(const base::string16& uri_string) {
  is_valid_ = InitCanonicalSipURI(uri_string, &spec_, &parsed_);
}

SipURI::SipURI(const char* canonical_spec, size_t canonical_spec_len,
               const uri::Parsed& parsed, bool is_valid)
    : spec_(canonical_spec, canonical_spec_len),
      is_valid_(is_valid),
      parsed_(parsed) {
#ifndef NDEBUG
  // For testing purposes, check that the parsed canonical URI is identical to
  // what we would have produced. Skip checking for invalid URIs have no meaning
  // and we can't always canonicalize then reproducabely.
  if (is_valid_) {
    uri::Component scheme;
    if (scheme.begin == parsed.scheme.begin) {
      SipURI test_url(spec_);

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

SipURI::~SipURI() {
}

SipURI& SipURI::operator=(const SipURI& other) {
  spec_ = other.spec_;
  is_valid_ = other.is_valid_;
  parsed_ = other.parsed_;
  return *this;
}

const std::string& SipURI::spec() const {
  static std::string empty;
  if (is_valid_ || spec_.empty())
    return spec_;

  DCHECK(false) << "Trying to get the spec of an invalid URL!";
  return empty;
}

bool SipURI::Equivalent(const SipURI& other) {
  // TODO
  return false;
}

SipURI SipURI::GetWithEmptyHeaders() const {
  // This doesn't make sense for invalid or nonstandard URIs, so return
  // the empty URL.
  if (!is_valid_)
    return SipURI();

  // We could optimize this since we know that the URL is canonical, and we are
  // appending a canonical path, so avoiding re-parsing.
  SipURI other(*this);
  if (parsed_.headers.len == 0)
    return other;

  // Clear the headers.
  other.parsed_.headers.reset();
  return other;
}

SipURI SipURI::GetOrigin() const {
  // This doesn't make sense for invalid or nonstandard URLs, so return
  // the empty URL.
  if (!is_valid_)
    return SipURI();

  std::ostringstream spec;
  spec << scheme() << ":" << host();
  if (has_port())
    spec << port();
  bool exists;
  std::string value;
  tie(exists, value) = parameter("transport");
  if (exists)
    spec << ";transport=" << value;
  return SipURI(spec.str());
}

bool SipURI::SchemeIs(const char* lower_ascii_scheme) const {
  if (parsed_.scheme.len <= 0)
    return lower_ascii_scheme == nullptr;
  return sippet::SchemeIs(spec_, parsed_, lower_ascii_scheme);
}

bool SipURI::HostIsIPAddress() const {
  if (!is_valid_ || spec_.empty())
     return false;

  url::RawCanonOutputT<char, 128> ignored_output;
  uri::CanonHostInfo host_info;
  uri::CanonicalizeIPAddress(spec_.c_str(), parsed_.host,
                             &ignored_output, &host_info);
  return host_info.IsIPAddress();
}

int SipURI::IntPort() const {
  if (parsed_.port.is_nonempty())
    return ParsePort(spec_.data(), parsed_.port);
  return uri::PORT_UNSPECIFIED;
}

int SipURI::EffectiveIntPort() const {
  int int_port = IntPort();
  if (int_port == uri::PORT_UNSPECIFIED)
    return uri::DefaultPortForScheme(spec_.data() + parsed_.scheme.begin,
                                           parsed_.scheme.len);
  return int_port;
}

std::string SipURI::HostNoBrackets() const {
  // If host looks like an IPv6 literal, strip the square brackets.
  uri::Component h(parsed_.host);
  if (h.len >= 2 && spec_[h.begin] == '[' && spec_[h.end() - 1] == ']') {
    h.begin++;
    h.len -= 2;
  }
  return uri_details::ComponentString(spec_, h);
}

bool SipURI::DomainIs(const char* lower_ascii_domain, int domain_len) const {
  // Return false if this URL is not valid or domain is empty.
  if (!is_valid_ || !domain_len)
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

  if (!uri::LowerCaseEqualsASCII(start_pos,
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

void SipURI::Swap(SipURI* other) {
  spec_.swap(other->spec_);
  std::swap(is_valid_, other->is_valid_);
  std::swap(parsed_, other->parsed_);
}

const SipURI& SipURI::EmptyURI() {
  static SipURI empty;
  return empty;
}

std::pair<bool, std::string> SipURI::parameter(const std::string &name) const {
  return LookupKeyValue(spec_, name, parsed_.parameters,
      &uri::ExtractParametersKeyValue);
}

std::pair<bool, std::string> SipURI::header(const std::string &name) const {
  return LookupKeyValue(spec_, name, parsed_.headers,
      &uri::ExtractHeadersKeyValue);
}

// TelURI --------------------------------------------------------------------

TelURI::TelURI() : is_valid_(false) {
}

TelURI::TelURI(const GURL &other) {
  is_valid_ = InitCanonicalTelURI(
    other.possibly_invalid_spec(), &spec_, &parsed_);
}

TelURI::TelURI(const TelURI& other) : spec_(other.spec_),
  is_valid_(other.is_valid_),
  parsed_(other.parsed_) {
}

TelURI::TelURI(const std::string& uri_string) {
  is_valid_ = InitCanonicalTelURI(uri_string, &spec_, &parsed_);
}

TelURI::TelURI(const base::string16& uri_string) {
  is_valid_ = InitCanonicalTelURI(uri_string, &spec_, &parsed_);
}

TelURI::TelURI(const char* canonical_spec, size_t canonical_spec_len,
               const uri::Parsed& parsed, bool is_valid)
    : spec_(canonical_spec, canonical_spec_len),
      is_valid_(is_valid),
      parsed_(parsed) {
#ifndef NDEBUG
  // For testing purposes, check that the parsed canonical URI is identical to
  // what we would have produced. Skip checking for invalid URIs have no meaning
  // and we can't always canonicalize then reproducabely.
  if (is_valid_) {
    uri::Component scheme;
    if (scheme.begin == parsed.scheme.begin) {
      TelURI test_url(spec_);

      DCHECK(test_url.is_valid_ == is_valid_);
      DCHECK(test_url.spec_ == spec_);

      DCHECK(test_url.parsed_.scheme == parsed_.scheme);
      DCHECK(test_url.parsed_.username == parsed_.username);
      DCHECK(test_url.parsed_.parameters == parsed_.parameters);
    }
  }
#endif
}

TelURI::~TelURI() {
}

TelURI& TelURI::operator=(const TelURI& other) {
  spec_ = other.spec_;
  is_valid_ = other.is_valid_;
  parsed_ = other.parsed_;
  return *this;
}

SipURI TelURI::ToSipURI(const SipURI& origin) {
  if (!origin.is_valid())
    return SipURI();

  const std::string &origin_spec = origin.spec();
  const uri::Parsed &origin_parsed =
    origin.parsed_for_possibly_invalid_spec();

  std::string canonical;
  uri::Parsed parsed;
  
  // Reserve enough room in the output for the input, plus some extra so that
  // we have room if we have to escape a few things without reallocating.
  canonical.reserve(origin.spec().size() + 32);
  url::StdStringCanonOutput output(&canonical);

  // Append the same scheme as the origin
  parsed.scheme.begin = 0;
  output.Append(origin_spec.data() + origin_parsed.scheme.begin,
    origin_parsed.scheme.len);
  output.Append(":", 1);
  parsed.scheme.len = origin_parsed.scheme.len;

  // Append the telephone-subscriber and parameters portion
  parsed.username.begin = output.length();
  output.Append(spec_.data() + parsed_.username.begin,
    parsed_.username.len);
  if (parsed_.parameters.len > 0) {
    output.Append(spec_.data() + parsed_.parameters.begin,
      parsed_.parameters.len);
  }
  parsed.username.len = output.length() - parsed.username.begin;
  output.Append("@", 1);

  // Append the host and port portions
  parsed.host.begin = output.length();
  output.Append(origin_spec.data() + origin_parsed.host.begin,
    origin_parsed.host.len);
  parsed.host.len = origin_parsed.host.len;
  
  if (origin_parsed.port.len > 0) {
    output.Append(":", 1);
    parsed.port.begin = output.length();
    output.Append(origin_spec.data() + origin_parsed.port.begin,
      origin_parsed.port.len);
    parsed.port.len = origin_parsed.port.len;
  }

  // Append parameters
  parsed.parameters.begin = output.length();
  output.Append(";user=phone", 11);
  parsed.parameters.len = output.length() - parsed.parameters.begin;

  output.Complete();  // Must be done before using string.
  return SipURI(canonical.data(), canonical.length(), parsed, true);
}

const std::string& TelURI::spec() const {
  static std::string empty;
  if (is_valid_ || spec_.empty())
    return spec_;

  DCHECK(false) << "Trying to get the spec of an invalid URL!";
  return empty;
}

void TelURI::Swap(TelURI* other) {
  spec_.swap(other->spec_);
  std::swap(is_valid_, other->is_valid_);
  std::swap(parsed_, other->parsed_);
}

const TelURI& TelURI::EmptyURI() {
  static TelURI empty;
  return empty;
}

std::pair<bool, std::string> TelURI::parameter(const std::string &name) const {
  return LookupKeyValue(spec_, name, parsed_.parameters,
      &uri::ExtractParametersKeyValue);
}

} // End of sippet namespace

std::ostream& operator<<(std::ostream& out, const sippet::SipURI& uri) {
  return out << uri.possibly_invalid_spec();
}

std::ostream& operator<<(std::ostream& out, const sippet::TelURI& uri) {
  return out << uri.possibly_invalid_spec();
}
