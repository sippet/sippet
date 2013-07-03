// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/uri/uri_parse.h"
#include "sippet/uri/uri_util.h"
#include "sippet/uri/uri_parse_internal.h"

#include "base/logging.h"
#include "base/string_util.h"

namespace sippet {
namespace uri_parse {

namespace {

template<typename CHAR>
bool DoExtractParametersKeyValue(const CHAR* spec,
                                 Component* query,
                                 Component* key,
                                 Component* value) {
  if (!query->is_nonempty())
    return false;

  int start = query->begin;
  int cur = start;
  int end = query->end();

  // We assume the beginning of the input is the beginning of the "key" and we
  // skip to the end of it.
  key->begin = cur;
  while (cur < end && spec[cur] != ';' && spec[cur] != '=')
    cur++;
  key->len = cur - key->begin;

  // Skip the separator after the key (if any).
  if (cur < end && spec[cur] == '=')
    cur++;

  // Find the value part.
  value->begin = cur;
  while (cur < end && spec[cur] != ';')
    cur++;
  value->len = cur - value->begin;

  // Finally skip the next separator if any
  if (cur < end && spec[cur] == ';')
    cur++;

  // Save the new query
  *query = uri_parse::MakeRange(cur, end);
  return true;
}

} // End of empty namespace

Parsed::Parsed() {
}

Parsed::Parsed(const Parsed& other) :
    scheme(other.scheme),
    username(other.username),
    password(other.password),
    host(other.host),
    port(other.port),
    parameters(other.parameters),
    headers(other.headers) {
}

Parsed& Parsed::operator=(const Parsed& other) {
  if (this != &other) {
    scheme = other.scheme;
    username = other.username;
    password = other.password;
    host = other.host;
    port = other.port;
    parameters = other.parameters;
    headers = other.headers;
  }
  return *this;
}

Parsed::~Parsed() {
}

int Parsed::Length() const {
  return headers.end();
}

int Parsed::CountCharactersBefore(ComponentType type,
                                  bool include_delimiter) const {
  if (type == SCHEME)
    return scheme.begin;

  int cur = 0;
  if (scheme.is_valid())
    cur = scheme.end() + 1;  // Advance over the ':' at the end of the scheme.

  if (username.is_valid()) {
    if (type <= USERNAME)
      return username.begin;
    cur = username.end() + 1;  // Advance over the '@' or ':' at the end.
  }

  if (password.is_valid()) {
    if (type <= PASSWORD)
      return password.begin;
    cur = password.end() + 1;  // Advance over the '@' at the end.
  }

  if (host.is_valid()) {
    if (type <= HOST)
      return host.begin;
    cur = host.end();
  }

  if (port.is_valid()) {
    if (type < PORT || (type == PORT && include_delimiter))
      return port.begin - 1;  // Back over delimiter.
    if (type == PORT)
      return port.begin;  // Don't want delimiter counted.
    cur = port.end();
  }

  if (parameters.is_valid()) {
    if (type <= PARAMETERS)
      return parameters.begin;
    cur = parameters.end();
  }

  if (headers.is_valid()) {
    if (type == HEADERS && !include_delimiter)
      return headers.begin;  // Don't want delimiter counted.
    
    // When there are headers and we get here, the component we wanted was before
    // this and not found, so we always know the beginning of the headers is right.
    return headers.begin - 1;  // Back over delimiter.
  }

  return cur;
}

// This handles everything that may be an authority terminator.
bool IsAuthorityTerminator(char16 ch) {
  return ch == ';' || ch == '?';
}

// Returns the offset of the next authority terminator in the input starting
// from start_offset. If no terminator is found, the return value will be equal
// to spec_len.
template<typename CHAR>
int FindNextAuthorityTerminator(const CHAR* spec,
                                int start_offset,
                                int spec_len) {
  int user_pass = start_offset;
  for (int i = user_pass; i < spec_len; i++) {
    if (spec[i] == '@') {
      user_pass = i;
      break;
    }
  }
  for (int i = user_pass; i < spec_len; i++) {
    if (IsAuthorityTerminator(spec[i]))
      return i;
  }
  return spec_len;  // Not found.
}

template<typename CHAR>
void DoParseAfterAuthority(const CHAR* spec,
                           const Component& remaining,
                           Component* parameters,
                           Component* headers) {
  // remaining = ;parameters?<headers>

  // Special case when there is no remaining.
  if (remaining.len == -1) {
    parameters->reset();
    headers->reset();
    return;
  }
  DCHECK(remaining.len > 0) << "We should never have 0 length remaining";

  // Search for first occurrence of ?.
  int remaining_end = remaining.begin + remaining.len;

  int headers_separator = -1;  // Index of the '?'
  for (int i = remaining.begin; i < remaining_end; i++) {
    if (spec[i] == '?' && headers_separator < 0) {
      // Only match the headers string if we haven't found one already.
      headers_separator = i;
      break;
    }
  }

  int parameters_end = remaining_end;

  // Headers fragment: everything from the ? to the end.
  if (headers_separator >= 0) {
    parameters_end = headers_separator;
    *headers = MakeRange(headers_separator + 1, remaining_end);
  } else {
    headers->reset();
  }

  // Parameters: everything from the ; to the next boundary (headers or
  // end of input).
  if (parameters_end != remaining.begin)
    *parameters = MakeRange(remaining.begin, parameters_end);
  else
    parameters->reset();
}

// Fills in all members of the Parsed structure except for the scheme.
//
// |spec| is the full spec being parsed, of length |spec_len|.
// |after_scheme| is the character immediately following the scheme (after the
//   colon) where we'll begin parsing.
template <typename CHAR>
void DoParseAfterScheme(const CHAR* spec,
                        int spec_len,
                        int after_scheme,
                        Parsed* parsed) {
  // First split into two main parts, the authority (username, password, host,
  // and port) and remaining (parameters and headers).
  Component authority;
  Component remaining;

  // Found "<some data>", should have an authority section. Treat everything
  // from there to the next ;/? (or end of spec) to be the authority.
  int end_auth = FindNextAuthorityTerminator(spec, after_scheme, spec_len);
  authority = Component(after_scheme, end_auth - after_scheme);

  if (end_auth == spec_len)  // No beginning of remaining found.
    remaining = Component();
  else  // Everything starting from the slash to the end is the path.
    remaining = Component(end_auth, spec_len - end_auth);

  // Now parse those two sub-parts.
  uri_parse::ParseAuthority(spec, authority, &parsed->username, &parsed->password,
                            &parsed->host, &parsed->port);
  uri_parse::ParseAfterAuthorityInternal(spec, remaining, &parsed->parameters,
                                         &parsed->headers);
}

// The main parsing function for SIP-URIs.
template<typename CHAR>
void DoParseSipURI(const CHAR* spec, int spec_len, Parsed* parsed) {
  DCHECK(spec_len >= 0);

  // Strip leading & trailing spaces and control characters.
  int begin = 0;
  uri_parse::TrimURI(spec, &begin, &spec_len);

  int after_scheme;
  if (uri_parse::ExtractScheme(spec, spec_len, &parsed->scheme)) {
    after_scheme = parsed->scheme.end() + 1;  // Skip past the colon.
    DoParseAfterScheme(spec, spec_len, after_scheme, parsed);
  }

  // Invalid URIs will lead to invalid results.
  // We can be more strict than GURL here..
}

// The main parsing function for SIP-URIs.
template<typename CHAR>
void DoParseTelURI(const CHAR* spec, int spec_len, Parsed* parsed) {
  DCHECK(spec_len >= 0);

  // Strip leading & trailing spaces and control characters.
  int begin = 0;
  uri_parse::TrimURI(spec, &begin, &spec_len);

  int after_scheme;
  if (uri_parse::ExtractScheme(spec, spec_len, &parsed->scheme)) {
    after_scheme = parsed->scheme.end() + 1;  // Skip past the colon.

    // First split into two main parts, the telephone-subscriber
    // and parameters.
    Component tel_subscriber;
    Component parameters;

    int tel_subscriber_end = spec_len;
    for (int i = after_scheme; i < spec_len; i++) {
      if (spec[i] == ';') {
        tel_subscriber_end = i;
        break;
      }
    }

    tel_subscriber = Component(after_scheme, tel_subscriber_end - after_scheme);
    if (tel_subscriber_end == spec_len)  // No parameters found.
      parameters = Component();
    else  // Everything starting from the semicolon to the end are params.
      parameters = Component(tel_subscriber_end, spec_len - tel_subscriber_end);

    // Now set those two sub-parts.
    parsed->username = tel_subscriber;
    parsed->parameters = parameters;
  }

  // Invalid URIs will lead to invalid results.
  // We can be more strict than GURL here..
}

void ParseSipURI(const char* uri, int uri_len, Parsed* parsed) {
  DoParseSipURI(uri, uri_len, parsed);
}

void ParseSipURI(const char16* uri, int uri_len, Parsed* parsed) {
  DoParseSipURI(uri, uri_len, parsed);
}

void ParseTelURI(const char* uri, int uri_len, Parsed* parsed) {
  DoParseTelURI(uri, uri_len, parsed);
}

void ParseTelURI(const char16* uri, int uri_len, Parsed* parsed) {
  DoParseTelURI(uri, uri_len, parsed);
}

bool ExtractParametersKeyValue(const char* uri,
                               Component* query,
                               Component* key,
                               Component* value) {
  return DoExtractParametersKeyValue(uri, query, key, value);
}

bool ExtractParametersKeyValue(const char16* uri,
                               Component* query,
                               Component* key,
                               Component* value) {
  return DoExtractParametersKeyValue(uri, query, key, value);
}

void ParseAfterAuthorityInternal(const char* spec,
                                 const Component& remaining,
                                 Component* parameters,
                                 Component* headers) {
  return DoParseAfterAuthority(spec, remaining, parameters, headers);
}

void ParseAfterAuthorityInternal(const char16* spec,
                                 const Component& remaining,
                                 Component* parameters,
                                 Component* headers) {
  return DoParseAfterAuthority(spec, remaining, parameters, headers);
}

void ParseAfterScheme(const char* spec,
                      int spec_len,
                      int after_scheme,
                      Parsed* parsed) {
  DoParseAfterScheme(spec, spec_len, after_scheme, parsed);
}

void ParseAfterScheme(const char16* spec,
                      int spec_len,
                      int after_scheme,
                      Parsed* parsed) {
  DoParseAfterScheme(spec, spec_len, after_scheme, parsed);
}

} // End of uri_parse namespace
} // End of sippet namespace
