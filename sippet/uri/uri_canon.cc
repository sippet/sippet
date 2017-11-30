// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/uri/uri.h"
#include "sippet/uri/uri_canon_internal.h"

namespace sippet {
namespace uri {

namespace {

template<typename CHAR>
bool DoCanonicalizeTelURI(const URIComponentSource<CHAR>& source,
                          const uri::Parsed& parsed,
                          CharsetConverter* query_converter,
                          CanonOutput* output,
                          uri::Parsed* new_parsed) {
  // Scheme: this will append the colon.
  bool success = uri::CanonicalizeScheme(source.scheme, parsed.scheme,
                                               output, &new_parsed->scheme);

  if (parsed.username.is_valid()) {
    // User info is the tel-subscriber.
    success &= uri::CanonicalizeUserInfo(
        source.username, parsed.username, source.password, parsed.password,
        output, &new_parsed->username, &new_parsed->password);

    // Parameters
    uri::CanonicalizeParameters(
        source.parameters, parsed.parameters, query_converter,
        output, &new_parsed->parameters);
  } else {
    // No user info, clear the components.
    new_parsed->username.reset();
    new_parsed->parameters.reset();
    success = false;  // Standard URLs must have an authority.
  }

  return success;
}

template<typename CHAR>
bool DoCanonicalizeSipURI(const URIComponentSource<CHAR>& source,
                          const uri::Parsed& parsed,
                          CharsetConverter* query_converter,
                          CanonOutput* output,
                          uri::Parsed* new_parsed) {
  // Scheme: this will append the colon.
  bool success = uri::CanonicalizeScheme(source.scheme, parsed.scheme,
                                               output, &new_parsed->scheme);

  // Authority (username, password, host, port)
  if (parsed.username.is_valid() || parsed.password.is_valid() ||
      parsed.host.is_nonempty() || parsed.port.is_valid()) {
    // User info: the canonicalizer will handle the : and @.
    success &= uri::CanonicalizeUserInfo(
        source.username, parsed.username, source.password, parsed.password,
        output, &new_parsed->username, &new_parsed->password);

    success &= uri::CanonicalizeHost(source.host, parsed.host,
                                           output, &new_parsed->host);

    // Host must not be empty for standard URLs.
    if (!parsed.host.is_nonempty())
      success = false;

    // Port: the port canonicalizer will handle the colon.
    int default_port = DefaultPortForScheme(
        &output->data()[new_parsed->scheme.begin], new_parsed->scheme.len);
    success &= uri::CanonicalizePort(
        source.port, parsed.port, default_port,
        output, &new_parsed->port);

    // Parameters
    uri::CanonicalizeParameters(
        source.parameters, parsed.parameters, query_converter,
        output, &new_parsed->parameters);

    // Headers
    uri::CanonicalizeHeaders(
        source.headers, parsed.headers, query_converter,
        output, &new_parsed->headers);

  } else {
    // No authority, clear the components.
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
                const uri::Component& username,
                const CHAR* password_spec,
                const uri::Component& password,
                CanonOutput* output,
                uri::Component* out_username,
                uri::Component* out_password) {
  if (username.len <= 0 && password.len <= 0) {
    // Common case: no user info. We strip empty username/passwords.
    *out_username = uri::Component();
    *out_password = uri::Component();
    return true;
  }

  // Write the username.
  out_username->begin = output->length();
  if (username.len > 0) {
    // This will escape characters not valid for the username.
    AppendStringOfType(&username_spec[username.begin], username.len,
                       CHAR_USERINFO, output);
  }
  out_username->len = output->length() - out_username->begin;

  // When there is a password, we need the separator. Note that we strip
  // empty but specified passwords.
  if (password.len > 0) {
    output->push_back(':');
    out_password->begin = output->length();
    AppendStringOfType(&password_spec[password.begin], password.len,
                       CHAR_USERINFO, output);
    out_password->len = output->length() - out_password->begin;
  } else {
    *out_password = url::Component();
  }

  output->push_back('@');
  return true;
}

template<typename CHAR>
void DoParameters(const CHAR* spec,
                  const uri::Component& parameters,
                  CharsetConverter* converter,
                  CanonOutput* output,
                  uri::Component* out_parameters) {
  if (parameters.len <= 0) {
    // Common case: no parameters.
    *out_parameters = uri::Component();
  } else {
    // Write the parameters.
    out_parameters->begin = output->length();
    AppendStringOfType(&spec[parameters.begin], parameters.len,
                       CHAR_PARAMETERS, output);
    out_parameters->len = output->length() - out_parameters->begin;
  }
}

template<typename CHAR>
void DoHeaders(const CHAR* spec,
               const uri::Component& headers,
               CharsetConverter* converter,
               CanonOutput* output,
               uri::Component* out_headers) {
  if (headers.len <= 0) {
    // Common case: no headers.
    *out_headers = uri::Component();
  } else {
    // Write the headers.
    output->push_back('?');
    out_headers->begin = output->length();
    AppendStringOfType(&spec[headers.begin], headers.len,
                       CHAR_HEADERS, output);
    out_headers->len = output->length() - out_headers->begin;
  }
}

}  // namespace

bool CanonicalizeUserInfo(const char* username_source,
                          const uri::Component& username,
                          const char* password_source,
                          const uri::Component& password,
                          CanonOutput* output,
                          uri::Component* out_username,
                          uri::Component* out_password) {
  return DoUserInfo(
      username_source, username, password_source, password,
      output, out_username, out_password);
}

bool CanonicalizeUserInfo(const base::char16* username_source,
                          const uri::Component& username,
                          const base::char16* password_source,
                          const uri::Component& password,
                          CanonOutput* output,
                          uri::Component* out_username,
                          uri::Component* out_password) {
  return DoUserInfo(
      username_source, username, password_source, password,
      output, out_username, out_password);
}

int DefaultPortForScheme(const char* scheme, int scheme_len) {
  int default_port = uri::PORT_UNSPECIFIED;
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

void CanonicalizeParameters(const char* spec,
                            const uri::Component& parameters,
                            CharsetConverter* converter,
                            CanonOutput* output,
                            uri::Component* out_parameters) {
  DoParameters(spec, parameters, converter, output, out_parameters);
}

void CanonicalizeParameters(const base::char16* spec,
                            const uri::Component& parameters,
                            CharsetConverter* converter,
                            CanonOutput* output,
                            uri::Component* out_parameters) {
  DoParameters(spec, parameters, converter, output, out_parameters);
}

void CanonicalizeHeaders(const char* spec,
                         const uri::Component& headers,
                         CharsetConverter* converter,
                         CanonOutput* output,
                         uri::Component* out_headers) {
  DoHeaders(spec, headers, converter, output, out_headers);
}

void CanonicalizeHeaders(const base::char16* spec,
                         const uri::Component& headers,
                         CharsetConverter* converter,
                         CanonOutput* output,
                         uri::Component* out_headers) {
  DoHeaders(spec, headers, converter, output, out_headers);
}

bool CanonicalizeSipURI(const char* spec,
                        int spec_len,
                        const uri::Parsed& parsed,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        uri::Parsed* new_parsed) {
  return DoCanonicalizeSipURI(URIComponentSource<char>(spec), parsed,
                              query_converter, output, new_parsed);
}

bool CanonicalizeSipURI(const base::char16* spec,
                        int spec_len,
                        const uri::Parsed& parsed,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        uri::Parsed* new_parsed) {
  return DoCanonicalizeSipURI(URIComponentSource<base::char16>(spec), parsed,
                              query_converter, output, new_parsed);
}

bool CanonicalizeTelURI(const char* spec,
                        int spec_len,
                        const uri::Parsed& parsed,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        uri::Parsed* new_parsed) {
  return DoCanonicalizeTelURI(URIComponentSource<char>(spec), parsed,
                              query_converter, output, new_parsed);
}

bool CanonicalizeTelURI(const base::char16* spec,
                        int spec_len,
                        const uri::Parsed& parsed,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        uri::Parsed* new_parsed) {
  return DoCanonicalizeTelURI(URIComponentSource<base::char16>(spec), parsed,
                              query_converter, output, new_parsed);
}

}  // namespace uri
}  // namespace sippet
