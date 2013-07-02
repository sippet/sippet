// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_PARSE_H_
#define SIPPET_MESSAGE_PARSE_H_

#include "googleurl/src/gurl.h"

namespace sippet {
namespace uri_parse {

using url_parse::Component;
using url_parse::SpecialPort;
using url_parse::PORT_UNSPECIFIED;
using url_parse::PORT_INVALID;

// Part replacer --------------------------------------------------------------

// Internal structure used for storing separate strings for each component.
// The basic canonicalization functions use this structure internally so that
// component replacement (different strings for different components) can be
// treated on the same code path as regular canonicalization (the same string
// for each component).
//
// A uri_parse::Parsed structure usually goes along with this. Those
// components identify offsets within these strings, so that they can all be
// in the same string, or spread arbitrarily across different ones.
//
// This structures does not own any data. It is the caller's responsibility to
// ensure that the data the pointers point to stays in scope and is not
// modified.
template<typename CHAR>
struct URIComponentSource {
  // Constructor normally used by callers wishing to replace components. This
  // will make them all NULL, which is no replacement. The caller would then
  // override the components they want to replace.
  URIComponentSource()
      : scheme(NULL),
        username(NULL),
        password(NULL),
        host(NULL),
        port(NULL),
        parameters(NULL),
        headers(NULL) {
  }

  // Constructor normally used internally to initialize all the components to
  // point to the same spec.
  explicit URIComponentSource(const CHAR* default_value)
      : scheme(default_value),
        username(default_value),
        password(default_value),
        host(default_value),
        port(default_value),
        parameters(default_value),
        headers(default_value) {
  }

  const CHAR* scheme;
  const CHAR* username;
  const CHAR* password;
  const CHAR* host;
  const CHAR* port;
  const CHAR* parameters;
  const CHAR* headers;
};

// Parsed ---------------------------------------------------------------------

// A structure that holds the identified parts of an input URI. This structure
// does NOT store the URI itself. The caller will have to store the URI text
// and its corresponding Parsed structure separately.
//
// Typical usage would be:
//
//    uri_parse::Parsed parsed;
//    uri_parse::Component scheme;
//    if (!uri_parse::ExtractScheme(uri, uri_len, &scheme))
//      return I_CAN_NOT_FIND_THE_SCHEME_DUDE;
//
//    if (IsSipURI(uri, scheme))  // Not provided by this component
//      uri_parse::ParseSipURI(uri, uri_len, &parsed);
//    else if (IsTelURI(uri, scheme))    // Not provided by this component
//      uri_parse::ParseTelURI(uri, uri_len, &parsed);
//    else
//      return I_CAN_NOT_FIND_THE_SCHEME_DUDE;
//
struct Parsed {
  // Identifies different components.
  enum ComponentType {
    SCHEME,
    USERNAME,
    PASSWORD,
    HOST,
    PORT,
    PARAMETERS,
    HEADERS,
  };

  // The default constructor is sufficient for the components, but inner_parsed_
  // requires special handling.
  Parsed();
  Parsed(const Parsed&);
  Parsed& operator=(const Parsed&);
  ~Parsed();

  // Returns the length of the URI (the end of the last component).
  int Length() const;

  // Returns the number of characters before the given component if it exists,
  // or where the component would be if it did exist. This will return the
  // string length if the component would be appended to the end.
  //
  // Note that this can get a little funny for the port, parameters, and
  // headers components which have a delimiter that is not counted as part of
  // the component. The |include_delimiter| flag controls if you want this
  // counted as part of the component or not when the component exists.
  //
  // This example shows the difference between the two flags for two of these
  // delimited components that is present (the port and query) and one that
  // isn't (the reference). The components that this flag affects are marked
  // with a *.
  //                 0         1         2         3
  //                 0123456789012345678901234567890
  // Example input:  sip:host.com:5555;parameters?headers
  //              include_delim=true,  ...=false  ("<-" indicates different)
  //      SCHEME: 0                    0
  //    USERNAME: 4                    4
  //    PASSWORD: 4                    4
  //        HOST: 4                    4
  //       *PORT: 12                   13 <-
  // *PARAMETERS: 17                   18 <-
  //    *HEADERS: 28                   29 <-
  //
  int CountCharactersBefore(ComponentType type,
                                     bool include_delimiter) const;

  // Scheme without the colon: "sip:foo.com" would have a scheme of "sip".
  // The length will be -1 if no scheme is specified ("foo.com"), or 0 if there
  // is a colon but no scheme (":foo"). Note that the scheme is not guaranteed
  // to start at the beginning of the string if there are preceeding whitespace
  // or control characters.
  Component scheme;

  // Username. Specified in URIs with an @ sign before the host. See |password|
  Component username;

  // Password. The length will be -1 if unspecified, 0 if specified but empty.
  // Not all URIs with a username have a password, as in "sip:me@foo.com".
  // The password is separated form the username with a colon, as in
  // "sip:me:secret@foo.com"
  Component password;

  // Host name.
  Component host;

  // Port number.
  Component port;

  // Parameters, this is everything following the host name. Length will be -1
  // if unspecified. This includes the preceeding semicolon, so the parameters
  // on sip:me@host;asdf" is ";asdf". As a result, it is impossible to have a
  // 0 length path, it will be -1 in cases like "sip:me@host?foo".
  Component parameters;

  // Stuff after the ?. This does not include the preceeding ? character.
  // Length will be -1 if unspecified, 0 if there is a question mark but no
  // headers.
  Component headers;
};

// SipURI is for sip: and sips: uris.
void ParseSipURI(const char* uri, int uri_len, Parsed* parsed);
void ParseSipURI(const char16* uri, int uri_len, Parsed* parsed);

// TelURI is for tel: uris.
void ParseTelURI(const char* uri, int uri_len, Parsed* parsed);
void ParseTelURI(const char16* uri, int uri_len, Parsed* parsed);

// Extract the first key/value from the range defined by |*parameters|. Updates
// |*parameters| to start at the end of the extracted key/value pair. This is
// designed for use in a loop: you can keep calling it with the same query
// object and it will iterate over all items in the query.
//
// Some key/value pairs may have the key, the value, or both be empty (for
// example, the parameters string ";;"). Empty keys won't be returned.
//
// The initial parameters component should include the first ';' (this is the
// default for parsed URIs).
//
// If no key/value are found |*key| and |*value| will be unchanged and it will
// return false.
bool ExtractParametersKeyValue(const char* uri,
                               Component* query,
                               Component* key,
                               Component* value);
bool ExtractParametersKeyValue(const char16* uri,
                               Component* query,
                               Component* key,
                               Component* value);

} // End of uri_parse namespace
} // End of sippet namespace

#endif // SIPPET_MESSAGE_PARSE_H_
