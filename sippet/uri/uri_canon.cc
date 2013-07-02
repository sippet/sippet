// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/uri/uri_canon.h"

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
  bool success = CanonicalizeScheme(source.scheme, parsed.scheme,
                                    output, &new_parsed->scheme);

  // Authority (username)
  bool have_authority;
  if (parsed.username.is_valid()) {
    have_authority = true;

    // User info: the canonicalizer will handle the : and @.
    success &= CanonicalizeUserInfo(
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
  bool success = CanonicalizeScheme(source.scheme, parsed.scheme,
                                    output, &new_parsed->scheme);

  // Authority (username, password, host, port)
  bool have_authority;
  if (parsed.username.is_valid() || parsed.password.is_valid() ||
      parsed.host.is_nonempty() || parsed.port.is_valid()) {
    have_authority = true;

    // User info: the canonicalizer will handle the : and @.
    success &= CanonicalizeUserInfo(
        source.username, parsed.username, source.password, parsed.password,
        output, &new_parsed->username, &new_parsed->password);

    success &= CanonicalizeHost(source.host, parsed.host,
                                output, &new_parsed->host);

    // Host must not be empty for standard URLs.
    if (!parsed.host.is_nonempty())
      success = false;

    // Port: the port canonicalizer will handle the colon.
    int default_port = DefaultPortForScheme(
        &output->data()[new_parsed->scheme.begin], new_parsed->scheme.len);
    success &= CanonicalizePort(
        source.port, parsed.port, default_port,
        output, &new_parsed->port);

    // Parameters
    CanonicalizeParameters(
        source.parameters, parsed.parameters, query_converter,
        output, &new_parsed->parameters);

    // Headers
    url_canon::CanonicalizeHeaders(
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

// Given a string and a range inside the string, compares it to the given
// lower-case |compare_to| buffer.
template<typename CHAR>
inline bool DoCompareSchemeComponent(const CHAR* spec,
                                     const uri_parse::Component& component,
                                     const char* compare_to) {
  if (!component.is_nonempty())
    return compare_to[0] == 0;  // When component is empty, match empty scheme.
  return LowerCaseEqualsASCII(&spec[component.begin],
                              &spec[component.end()],
                              compare_to);
}

template<typename CHAR>
bool DoCanonicalizeURI(const CHAR* in_spec, int in_spec_len,
                       CharsetConverter* charset_converter,
                       CanonOutput* output,
                       uri_parse::Parsed* output_parsed) {
  // Remove any whitespace from the middle of the relative URL, possibly
  // copying to the new buffer.
  url_canon::RawCanonOutputT<CHAR> whitespace_buffer;
  int spec_len;
  const CHAR* spec = RemoveURLWhitespace(in_spec, in_spec_len,
                                         &whitespace_buffer, &spec_len);

  uri_parse::Parsed parsed_input;
  uri_parse::Component scheme;
  if (!uri_parse::ExtractScheme(spec, spec_len, &scheme))
    return false;

  // This is the parsed version of the input URL, we have to canonicalize it
  // before storing it in our object.
  bool success = false;
  if (DoCompareSchemeComponent(spec, scheme, "sip") ||
      DoCompareSchemeComponent(spec, scheme, "sips")) {
    uri_parse::ParseSipURI(spec, spec_len, &parsed_input);
    success = CanonicalizeSipURI(
      URIComponentSource<CHAR>(spec), parsed_input,
      charset_converter, output, output_parsed);
  } else if (DoCompareSchemeComponent(spec, scheme, "tel")) {
    uri_parse::ParseTelURI(spec, spec_len, &parsed_input);
    success = CanonicalizeTelURI(
        URIComponentSource<CHAR>(spec), parsed_input,
        charset_converter, output, output_parsed);
  }
  return success;
}

} // End of empty namespace

bool CanonicalizeUserInfo(const char* username_source,
                          const uri_parse::Component& username,
                          const char* password_source,
                          const uri_parse::Component& password,
                          CanonOutput* output,
                          uri_parse::Component* out_username,
                          uri_parse::Component* out_password) {
  // XXX: should change the escaping method used in the username
  return url_canon::CanonicalizeUserInfo(username_source, username,
      password_source, password, output, out_username, out_password);
}

bool CanonicalizeUserInfo(const char16* username_source,
                          const uri_parse::Component& username,
                          const char16* password_source,
                          const uri_parse::Component& password,
                          CanonOutput* output,
                          uri_parse::Component* out_username,
                          uri_parse::Component* out_password) {
  // XXX: should change the escaping method used in the username
  return url_canon::CanonicalizeUserInfo(username_source, username,
      password_source, password, output, out_username, out_password);
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
                            CanonOutput* output,
                            uri_parse::Component* out_parameters) {
  // TODO
  return false;
}

bool CanonicalizeParameters(const char16* spec,
                            const uri_parse::Component& parameters,
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

bool CanonicalizeURI(const char* spec,
                     int spec_len,
                     const uri_parse::Parsed& parsed,
                     CharsetConverter* query_converter,
                     CanonOutput* output,
                     uri_parse::Parsed* new_parsed) {
  return DoCanonicalizeURI(spec, spec_len, parsed, query_converter,
                           output, new_parsed);
}

bool CanonicalizeURI(const char16* spec,
                     int spec_len,
                     const uri_parse::Parsed& parsed,
                     CharsetConverter* query_converter,
                     CanonOutput* output,
                     uri_parse::Parsed* new_parsed) {
  return DoCanonicalizeURI(spec, spec_len, parsed, query_converter,
                           output, new_parsed);
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