// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_CANON_H_
#define SIPPET_MESSAGE_CANON_H_

#include "sippet/uri/uri.h"

namespace sippet {
namespace uri {

// Normally, all canonicalization output is in narrow characters. We support
// the templates so it can also be used internally if a wide buffer is
// required.
using url_canon::CanonOutput;
using url_canon::CanonOutputW;
using url_canon::RawCanonOutput;
using url_canon::RawCanonOutputW;

// Character set converter ----------------------------------------------------
//
// Converts header strings into a custom encoding. The embedder can supply an
// implementation of this class to interface with their own character set
// conversion libraries.
//
// Embedders will want to see the unit test for the ICU version.
using url_canon::CharsetConverter;

// Whitespace -----------------------------------------------------------------

// Searches for whitespace that should be removed from the middle of URIs, and
// removes it. Removed whitespace are tabs and newlines, but NOT spaces. Spaces
// are preserved, which is what most browsers do. A pointer to the output will
// be returned, and the length of that output will be in |output_len|.
//
// This should be called before parsing if whitespace removal is desired (which
// it normally is when you are canonicalizing).
//
// If no whitespace is removed, this function will not use the buffer and will
// return a pointer to the input, to avoid the extra copy. If modification is
// required, the given |buffer| will be used and the returned pointer will
// point to the beginning of the buffer.
//
// Therefore, callers should not use the buffer, since it may actuall be empty,
// use the computed pointer and |*output_len| instead.
inline
const char* RemoveURIWhitespace(const char* input, int input_len,
                                CanonOutput* buffer,
                                int* output_len) {
  return url_canon::RemoveURLWhitespace(input, input_len, buffer, output_len);
}
inline
const base::char16* RemoveURLWhitespace(const base::char16* input, int input_len,
                                        CanonOutputW* buffer,
                                        int* output_len) {
  return url_canon::RemoveURLWhitespace(input, input_len, buffer, output_len);
}

// IDN ------------------------------------------------------------------------

// Converts the Unicode input representing a hostname to ASCII using IDN rules.
// The output must fall in the ASCII range, but will be encoded in UTF-16.
//
// On success, the output will be filled with the ASCII host name and it will
// return true. Unlike most other canonicalization functions, this assumes that
// the output is empty. The beginning of the host will be at offset 0, and
// the length of the output will be set to the length of the new host name.
//
// On error, returns false. The output in this case is undefined.
inline
bool IDNToASCII(const base::char16* src, int src_len, CanonOutputW* output) {
  return url_canon::IDNToASCII(src, src_len, output);
}

// Piece-by-piece canonicalizers ----------------------------------------------
//
// These individual canonicalizers append the canonicalized versions of the
// corresponding URL component to the given std::string. The spec and the
// previously-identified range of that component are the input. The range of
// the canonicalized component will be written to the output component.
//
// These functions all append to the output so they can be chained. Make sure
// the output is empty when you start.
//
// These functions returns boolean values indicating success. On failure, they
// will attempt to write something reasonable to the output so that, if
// displayed to the user, they will recognise it as something that's messed up.
// Nothing more should ever be done with these invalid URLs, however.

// Scheme: Appends the scheme and colon to the URL. The output component will
// indicate the range of characters up to but not including the colon.
//
// Canonical URLs always have a scheme. If the scheme is not present in the
// input, this will just write the colon to indicate an empty scheme. Does not
// append slashes which will be needed before any authority components for most
// URLs.
//
// The 8-bit version requires UTF-8 encoding.
inline
bool CanonicalizeScheme(const char* spec,
                        const uri::Component& scheme,
                        CanonOutput* output,
                        uri::Component* out_scheme) {
  return url_canon::CanonicalizeScheme(spec, scheme, output, out_scheme);
}
inline
bool CanonicalizeScheme(const base::char16* spec,
                        const uri::Component& scheme,
                        CanonOutput* output,
                        uri::Component* out_scheme) {
  return url_canon::CanonicalizeScheme(spec, scheme, output, out_scheme);
}

// User info: username/password. If present, this will add the delimiters so
// the output will be "<username>:<password>@" or "<username>@". Empty
// username/password pairs, or empty passwords, will get converted to
// nonexistant in the canonical version.
//
// The components for the username and password refer to ranges in the
// respective source strings. Usually, these will be the same string, which
// is legal as long as the two components don't overlap.
//
// The 8-bit version requires UTF-8 encoding.
//
// These versions differ from the HTTP URLs due to differences in character
// escaping.
bool CanonicalizeUserInfo(const char* username_source,
                          const uri::Component& username,
                          const char* password_source,
                          const uri::Component& password,
                          CanonOutput* output,
                          uri::Component* out_username,
                          uri::Component* out_password);
bool CanonicalizeUserInfo(const base::char16* username_source,
                          const uri::Component& username,
                          const base::char16* password_source,
                          const uri::Component& password,
                          CanonOutput* output,
                          uri::Component* out_username,
                          uri::Component* out_password);

// This structure holds detailed state exported from the IP/Host canonicalizers.
// Additional fields may be added as callers require them.
using url_canon::CanonHostInfo;

// Host.
//
// The 8-bit version requires UTF-8 encoding.  Use this version when you only
// need to know whether canonicalization succeeded.
inline
bool CanonicalizeHost(const char* spec,
                      const uri::Component& host,
                      CanonOutput* output,
                      uri::Component* out_host) {
  return url_canon::CanonicalizeHost(spec, host, output, out_host);
}
inline
bool CanonicalizeHost(const base::char16* spec,
                      const uri::Component& host,
                      CanonOutput* output,
                      uri::Component* out_host) {
  return url_canon::CanonicalizeHost(spec, host, output, out_host);
}

// Extended version of CanonicalizeHost, which returns additional information.
// Use this when you need to know whether the hostname was an IP address.
// A successful return is indicated by host_info->family != BROKEN.  See the
// definition of CanonHostInfo above for details.
inline
void CanonicalizeHostVerbose(const char* spec,
                             const uri::Component& host,
                             CanonOutput* output,
                             CanonHostInfo* host_info) {
  return url_canon::CanonicalizeHostVerbose(spec, host, output, host_info);
}
inline
void CanonicalizeHostVerbose(const base::char16* spec,
                             const uri::Component& host,
                             CanonOutput* output,
                             CanonHostInfo* host_info) {
  return url_canon::CanonicalizeHostVerbose(spec, host, output, host_info);
}

// IP addresses.
//
// Tries to interpret the given host name as an IPv4 or IPv6 address. If it is
// an IP address, it will canonicalize it as such, appending it to |output|.
// Additional status information is returned via the |*host_info| parameter.
// See the definition of CanonHostInfo above for details.
//
// This is called AUTOMATICALLY from the host canonicalizer, which ensures that
// the input is unescaped and name-prepped, etc. It should not normally be
// necessary or wise to call this directly.
inline
void CanonicalizeIPAddress(const char* spec,
                           const uri::Component& host,
                           CanonOutput* output,
                           CanonHostInfo* host_info) {
  return url_canon::CanonicalizeIPAddress(spec, host, output, host_info);
}
inline
void CanonicalizeIPAddress(const base::char16* spec,
                           const uri::Component& host,
                           CanonOutput* output,
                           CanonHostInfo* host_info) {
  return url_canon::CanonicalizeIPAddress(spec, host, output, host_info);
}

// Port: this function will add the colon for the port if a port is present.
// The caller can pass uri::PORT_UNSPECIFIED as the
// default_port_for_scheme argument if there is no default port.
//
// The 8-bit version requires UTF-8 encoding.
inline
bool CanonicalizePort(const char* spec,
                      const uri::Component& port,
                      int default_port_for_scheme,
                      CanonOutput* output,
                      uri::Component* out_port) {
  return url_canon::CanonicalizePort(spec, port, default_port_for_scheme,
                                     output, out_port);
}
inline
bool CanonicalizePort(const base::char16* spec,
                      const uri::Component& port,
                      int default_port_for_scheme,
                      CanonOutput* output,
                      uri::Component* out_port) {
  return url_canon::CanonicalizePort(spec, port, default_port_for_scheme,
                                     output, out_port);
}

// Returns the default port for the given canonical scheme, or PORT_UNSPECIFIED
// if the scheme is unknown.
int DefaultPortForScheme(const char* scheme, int scheme_len);

// Parameters.
//
// The 8-bit version assumes UTF-8 encoding, but does not verify the validity
// of the UTF-8 (i.e., you can have invalid UTF-8 sequences, invalid
// characters, etc.). Normally, URLs will come in as UTF-16, so this isn't
// an issue. Somebody giving us 8-bit parameters is responsible for generating
// the parameters that the server expects (we'll escape high-bit characters),
// so if something is invalid, it's their problem.
void CanonicalizeParameters(const char* spec,
                            const uri::Component& parameters,
                            CharsetConverter* converter,
                            CanonOutput* output,
                            uri::Component* out_parameters);
void CanonicalizeParameters(const base::char16* spec,
                            const uri::Component& parameters,
                            CharsetConverter* converter,
                            CanonOutput* output,
                            uri::Component* out_parameters);

// Headers: Prepends the ? if needed.
//
// The 8-bit version requires the input to be UTF-8 encoding. Incorrectly
// encoded characters (in UTF-8 or UTF-16) will be replaced with the Unicode
// "invalid character." This function can not fail, we always just try to do
// our best for crazy input here since web pages can set it themselves.
//
// This will convert the given input into the output encoding that the given
// character set converter object provides. The converter will only be called
// if necessary, for ASCII input, no conversions are necessary.
//
// The converter can be NULL. In this case, the output encoding will be UTF-8.
void CanonicalizeHeaders(const char* spec,
                         const uri::Component& headers,
                         CharsetConverter* converter,
                         CanonOutput* output,
                         uri::Component* out_headers);
void CanonicalizeHeaders(const base::char16* spec,
                         const uri::Component& headers,
                         CharsetConverter* converter,
                         CanonOutput* output,
                         uri::Component* out_headers);

// Full canonicalizer ---------------------------------------------------------
//
// These functions replace any string contents, rather than append as above.
// See the above piece-by-piece functions for information specific to
// canonicalizing individual components.
//
// The output will be ASCII except the reference fragment, which may be UTF-8.
//
// The 8-bit versions require UTF-8 encoding.

// Use for SIP-URIs.
bool CanonicalizeSipURI(const char* spec,
                        int spec_len,
                        const uri::Parsed& parsed,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        uri::Parsed* new_parsed);
bool CanonicalizeSipURI(const base::char16* spec,
                        int spec_len,
                        const uri::Parsed& parsed,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        uri::Parsed* new_parsed);

// Use for tel URIs.
bool CanonicalizeTelURI(const char* spec,
                        int spec_len,
                        const uri::Parsed& parsed,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        uri::Parsed* new_parsed);
bool CanonicalizeTelURI(const base::char16* spec,
                        int spec_len,
                        const uri::Parsed& parsed,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        uri::Parsed* new_parsed);

} // End of uri namespace
} // End of sippet namespace

#endif // SIPPET_MESSAGE_CANON_H_
