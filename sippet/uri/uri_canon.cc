// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/uri/uri_canon.h"
#include "sippet/uri/uri_canon_internal.h"

namespace sippet {
namespace uri_canon {

namespace {

template<typename CHAR>
bool DoCanonicalizeTelURI(const URIComponentSource<CHAR>& source,
                          const uri_parse::Parsed& parsed,
                          CharsetConverter* query_converter,
                          CanonOutput* output,
                          uri_parse::Parsed* new_parsed) {
  // Scheme: this will append the colon.
  bool success = uri_canon::CanonicalizeScheme(source.scheme, parsed.scheme,
                                               output, &new_parsed->scheme);

  // Authority (username)
  bool have_authority;
  if (parsed.username.is_valid()) {
    have_authority = true;

    // User info: the canonicalizer will handle the : and @.
    success &= uri_canon::CanonicalizeUserInfo(
        source.username, parsed.username, source.password, parsed.password,
        output, &new_parsed->username, &new_parsed->password);
  } else {
    // No authority, clear the components.
    have_authority = false;
    new_parsed->username.reset();
    success = false;  // Standard URLs must have an authority.
  }

  return success;
}

template<typename CHAR>
bool DoCanonicalizeSipURI(const URIComponentSource<CHAR>& source,
                          const uri_parse::Parsed& parsed,
                          CharsetConverter* query_converter,
                          CanonOutput* output,
                          uri_parse::Parsed* new_parsed) {
  // Scheme: this will append the colon.
  bool success = uri_canon::CanonicalizeScheme(source.scheme, parsed.scheme,
                                               output, &new_parsed->scheme);

  // Authority (username, password, host, port)
  bool have_authority;
  if (parsed.username.is_valid() || parsed.password.is_valid() ||
      parsed.host.is_nonempty() || parsed.port.is_valid()) {
    have_authority = true;

    // User info: the canonicalizer will handle the : and @.
    success &= uri_canon::CanonicalizeUserInfo(
        source.username, parsed.username, source.password, parsed.password,
        output, &new_parsed->username, &new_parsed->password);

    success &= uri_canon::CanonicalizeHost(source.host, parsed.host,
                                           output, &new_parsed->host);

    // Host must not be empty for standard URLs.
    if (!parsed.host.is_nonempty())
      success = false;

    // Port: the port canonicalizer will handle the colon.
    int default_port = DefaultPortForScheme(
        &output->data()[new_parsed->scheme.begin], new_parsed->scheme.len);
    success &= uri_canon::CanonicalizePort(
        source.port, parsed.port, default_port,
        output, &new_parsed->port);

    // Parameters
    uri_canon::CanonicalizeParameters(
        source.parameters, parsed.parameters, query_converter,
        output, &new_parsed->parameters);

    // Headers
    uri_canon::CanonicalizeHeaders(
        source.headers, parsed.headers, query_converter,
        output, &new_parsed->headers);

  } else {
    // No authority, clear the components.
    have_authority = false;
    new_parsed->host.reset();
    new_parsed->username.reset();
    new_parsed->password.reset();
    new_parsed->port.reset();
    success = false;  // Standard URLs must have an authority.
  }

  return success;
}

// The username and password components reference ranges in the corresponding
// *_spec strings. Typically, these specs will be the same (we're
// canonicalizing a single source string), but may be different when
// replacing components.
template<typename CHAR>
bool DoUserInfo(const CHAR* username_spec,
                const uri_parse::Component& username,
                const CHAR* password_spec,
                const uri_parse::Component& password,
                CanonOutput* output,
                uri_parse::Component* out_username,
                uri_parse::Component* out_password) {
  if (username.len <= 0 && password.len <= 0) {
    // Common case: no user info. We strip empty username/passwords.
    *out_username = uri_parse::Component();
    *out_password = uri_parse::Component();
    return true;
  }

  // Write the username.
  out_username->begin = output->length();
  if (username.len > 0) {
    // This will escape characters not valid for the username.
    AppendUserInfoString(&username_spec[username.begin], username.len,
                         output);
  }
  out_username->len = output->length() - out_username->begin;

  // When there is a password, we need the separator. Note that we strip
  // empty but specified passwords.
  if (password.len > 0) {
    output->push_back(':');
    out_password->begin = output->length();
    AppendUserInfoString(&password_spec[password.begin], password.len,
                         output);
    out_password->len = output->length() - out_password->begin;
  } else {
    *out_password = url_parse::Component();
  }

  output->push_back('@');
  return true;
}

} // End of empty namespace

bool CanonicalizeUserInfo(const char* username_source,
                          const uri_parse::Component& username,
                          const char* password_source,
                          const uri_parse::Component& password,
                          CanonOutput* output,
                          uri_parse::Component* out_username,
                          uri_parse::Component* out_password) {
  return DoUserInfo(
      username_source, username, password_source, password,
      output, out_username, out_password);
}

bool CanonicalizeUserInfo(const char16* username_source,
                          const uri_parse::Component& username,
                          const char16* password_source,
                          const uri_parse::Component& password,
                          CanonOutput* output,
                          uri_parse::Component* out_username,
                          uri_parse::Component* out_password) {
  return DoUserInfo(
      username_source, username, password_source, password,
      output, out_username, out_password);
}

int DefaultPortForScheme(const char* scheme, int scheme_len) {
  int default_port = uri_parse::PORT_UNSPECIFIED;
  switch (scheme_len) {
    case 3:
      if (!strncmp(scheme, "sip", scheme_len))
        default_port = 5060;
      break;
    case 4:
      if (!strncmp(scheme, "sips", scheme_len))
        default_port = 5061;
      break;
  }
  return default_port;
}

bool CanonicalizeParameters(const char* spec,
                            const uri_parse::Component& parameters,
                            CharsetConverter* converter,
                            CanonOutput* output,
                            uri_parse::Component* out_parameters) {
  // TODO
  return false;
}

bool CanonicalizeParameters(const char16* spec,
                            const uri_parse::Component& parameters,
                            CharsetConverter* converter,
                            CanonOutput* output,
                            uri_parse::Component* out_parameters) {
  // TODO
  return false;
}

void CanonicalizeHeaders(const char* spec,
                         const uri_parse::Component& headers,
                         CharsetConverter* converter,
                         CanonOutput* output,
                         uri_parse::Component* out_headers) {
  // TODO
}

void CanonicalizeHeaders(const char16* spec,
                         const uri_parse::Component& headers,
                         CharsetConverter* converter,
                         CanonOutput* output,
                         uri_parse::Component* out_headers) {
  // TODO
}

bool CanonicalizeSipURI(const char* spec,
                        int spec_len,
                        const uri_parse::Parsed& parsed,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        uri_parse::Parsed* new_parsed) {
  return DoCanonicalizeSipURI(URIComponentSource<char>(spec), parsed,
                              query_converter, output, new_parsed);
}

bool CanonicalizeSipURI(const char16* spec,
                        int spec_len,
                        const uri_parse::Parsed& parsed,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        uri_parse::Parsed* new_parsed) {
  return DoCanonicalizeSipURI(URIComponentSource<char16>(spec), parsed,
                              query_converter, output, new_parsed);
}

bool CanonicalizeTelURI(const char* spec,
                        int spec_len,
                        const uri_parse::Parsed& parsed,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        uri_parse::Parsed* new_parsed) {
  return DoCanonicalizeTelURI(URIComponentSource<char>(spec), parsed,
                              query_converter, output, new_parsed);
}

bool CanonicalizeTelURI(const char16* spec,
                        int spec_len,
                        const uri_parse::Parsed& parsed,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        uri_parse::Parsed* new_parsed) {
  return DoCanonicalizeTelURI(URIComponentSource<char16>(spec), parsed,
                              query_converter, output, new_parsed);
}

} // End of uri_canon namespace
} // End of sippet namespace