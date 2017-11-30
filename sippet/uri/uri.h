// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_URI_H_
#define SIPPET_MESSAGE_URI_H_

#include "url/gurl.h"
#include "sippet/uri/uri_parse.h"
#include "sippet/uri/uri_canon.h"
#include "sippet/uri/uri_util.h"
#include "base/strings/string_util.h"

namespace sippet {

namespace uri_details {

// Returns the substring of the input identified by the given component.
inline
std::string ComponentString(const std::string &spec,
                            const uri::Component& comp) {
  if (comp.len <= 0)
    return std::string();
  return std::string(spec, comp.begin, comp.len);
}

// Returns the unescaped substring of the input identified by the given
// component.
inline
std::string UnescapedComponentString(const std::string &spec,
                                     const uri::Component& comp) {
  std::string unescaped;
  url::StdStringCanonOutput output(&unescaped);
  uri::DecodeURIEscapeSequences(spec.data() + comp.begin, comp.len, &output);
  output.Complete();
  return unescaped;
}

} // End of uri_details namespace

// The SipURI object accepts sip and sips schemes.
class SipURI {
 public:
  // Creates an empty, invalid SIP-URI.
  SipURI();

  // Creates a SIP-URI from a GURL object.
  explicit SipURI(const GURL &other);

  // Copy construction is relatively inexpensive, with most of the time going
  // to reallocating the string. It does not re-parse.
  SipURI(const SipURI& other);

  // The narrow version requires the input be UTF-8. Invalid UTF-8 input will
  // result in an invalid URI.
  //
  // The wide version should also take an encoding parameter so we know how to
  // encode the query parameters. It is probably sufficient for the narrow
  // version to assume the query parameter encoding should be the same as the
  // input encoding.
  explicit SipURI(const std::string& uri_string);
  explicit SipURI(const base::string16& uri_string);

  // Constructor for URIs that have already been parsed and canonicalized. The
  // caller must supply all information associated with the URI, which must be
  // correct and consistent.
  SipURI(const char* canonical_spec, size_t canonical_spec_len,
         const uri::Parsed& parsed, bool is_valid);

  ~SipURI();

  SipURI& operator=(const SipURI& other);

  // Returns true when this object represents a valid parsed URI. When not
  // valid, other functions will still succeed, but you will not get canonical
  // data out in the format you may be expecting. Instead, we keep something
  // "reasonable looking" so that the user can see how it's busted if
  // displayed to them.
  bool is_valid() const {
    return is_valid_;
  }

  // Returns true if the URI is zero-length. Note that empty URIs are also
  // invalid, and is_valid() will return false for them. This is provided
  // because some users may want to treat the empty case differently.
  bool is_empty() const {
    return spec_.empty();
  }

  // Returns the raw spec, i.e., the full text of the URI, in canonical UTF-8,
  // if the URI is valid. If the URI is not valid, this will assert and return
  // the empty string (for safety in release builds, to keep them from being
  // misused which might be a security problem).
  //
  // The URI will be ASCII except the reference fragment, which may be UTF-8.
  // It is guaranteed to be valid UTF-8.
  //
  // The exception is for empty() URIs (which are !is_valid()) but this will
  // return the empty string without asserting.
  //
  // Used invalid_spec() below to get the unusable spec of an invalid URI. This
  // separation is designed to prevent errors that may cause security problems
  // that could result from the mistaken use of an invalid URI.
  const std::string& spec() const;

  // Returns the potentially invalid spec for a the URI. This spec MUST NOT be
  // modified or sent over the network. It is designed to be displayed in error
  // messages to the user, as the apperance of the spec may explain the error.
  // If the spec is valid, the valid spec will be returned.
  //
  // The returned string is guaranteed to be valid UTF-8.
  const std::string& possibly_invalid_spec() const {
    return spec_;
  }

  // Getter for the raw parsed structure. This allows callers to locate parts
  // of the URI within the spec themselves. Most callers should consider using
  // the individual component getters below.
  //
  // The returned parsed structure will reference into the raw spec, which may
  // or may not be valid. If you are using this to index into the spec, BE
  // SURE YOU ARE USING possibly_invalid_spec() to get the spec, and that you
  // don't do anything "important" with invalid specs.
  const uri::Parsed& parsed_for_possibly_invalid_spec() const {
    return parsed_;
  }

  // Defiant equality operator!
  bool operator==(const SipURI& other) const {
    return spec_ == other.spec_;
  }
  bool operator!=(const SipURI& other) const {
    return spec_ != other.spec_;
  }

  // Allows URI to used as a key in STL (for example, a std::set or std::map).
  bool operator<(const SipURI& other) const {
    return spec_ < other.spec_;
  }

  // Performs equality comparison using RFC 3261 standard.  It fails to compare
  // SIP headers, therefore it cannot be used to compare these kind of URIs.
  bool Equivalent(const SipURI& other);

  // A helper function that is equivalent to removing all headers
  SipURI GetWithEmptyHeaders() const;

  // A helper function to return a URI containing just the scheme, host,
  // and port from a URI.
  SipURI GetOrigin() const;

  // Returns true if the given parameter (should be lower-case ASCII to match
  // the canonicalized scheme) is the scheme for this URI. This call is more
  // efficient than getting the scheme and comparing it because no copies or
  // object constructions are done.
  bool SchemeIs(const char* lower_ascii_scheme) const;

  // If the scheme indicates a secure connection
  bool SchemeIsSecure() const {
    return SchemeIs("sips");
  }

  // Returns true if the hostname is an IP address. Note: this function isn't
  // as cheap as a simple getter because it re-parses the hostname to verify.
  bool HostIsIPAddress() const;

  // Getters for various components of the URI. The returned string will be
  // empty if the component is empty or is not present.
  std::string scheme() const {  // Not including the colon. See also SchemeIs.
    return uri_details::ComponentString(spec_, parsed_.scheme);
  }
  std::string username() const {
    return uri_details::UnescapedComponentString(spec_, parsed_.username);
  }
  std::string password() const {
    return uri_details::UnescapedComponentString(spec_, parsed_.password);
  }
  // Note that this may be a hostname, an IPv4 address, or an IPv6 literal
  // surrounded by square brackets, like "[2001:db8::1]".  To exclude these
  // brackets, use HostNoBrackets() below.
  std::string host() const {
    return uri_details::ComponentString(spec_, parsed_.host);
  }
  std::string port() const {  // Returns -1 if "default"
    return uri_details::ComponentString(spec_, parsed_.port);
  }
  std::string parameters() const {  // Including first semicolon following host
    return uri_details::ComponentString(spec_, parsed_.parameters);
  }
  std::string headers() const {  // Stuff following '?'
    return uri_details::ComponentString(spec_, parsed_.headers);
  }

  // Existance querying. These functions will return true if the corresponding
  // URI component exists in this URI. Note that existance is different than
  // being nonempty. sip:user@domain.com? has headers that just happens to
  // be empty, and has_headers() will return true.
  bool has_scheme() const {
    return parsed_.scheme.len >= 0;
  }
  bool has_username() const {
    return parsed_.username.len >= 0;
  }
  bool has_password() const {
    return parsed_.password.len >= 0;
  }
  bool has_host() const {
    // Note that hosts are special, absense of host means length 0.
    return parsed_.host.len > 0;
  }
  bool has_port() const {
    return parsed_.port.len >= 0;
  }
  bool has_parameters() const {
    return parsed_.parameters.len >= 0;
  }
  bool has_headers() const {
    return parsed_.headers.len >= 0;
  }

  // Returns a parsed version of the port. Can also be any of the special
  // values defined in Parsed for ExtractPort.
  int IntPort() const;

  // Returns the port number of the uri, or the default port number.
  // If the scheme has no concept of port (or unknown default) returns
  // PORT_UNSPECIFIED.
  int EffectiveIntPort() const;

  // Returns the host, excluding the square brackets surrounding IPv6 address
  // literals.  This can be useful for passing to getaddrinfo().
  std::string HostNoBrackets() const;

  // Returns true if this URI's host matches or is in the same domain as
  // the given input string. For example if this URI was "google.com",
  // this would match "com" and "google.com" (input domain should be
  // lower-case ASCII to match the canonicalized scheme). This call
  // is more efficient than getting the host and check whether host has
  // the specific domain or not because no copies or object constructions
  // are done.
  //
  // If function DomainIs has parameter domain_len, which means the parameter
  // lower_ascii_domain does not guarantee to terminate with NULL character.
  bool DomainIs(const char* lower_ascii_domain,
                int domain_len) const;

  // Swaps the contents of this URI object with the argument without doing
  // any memory allocations.
  void Swap(SipURI* other);

  // Returns a reference to a singleton empty URI. This object is for callers
  // who return references but don't have anything to return in some cases.
  // This function may be called from any thread.
  static const SipURI& EmptyURI();

  // Returns the given parameter if available.
  std::pair<bool, std::string> parameter(const std::string &name) const;

  // Returns the given header if available.
  std::pair<bool, std::string> header(const std::string &name) const;

 private:
  // The actual text of the URI, in canonical ASCII form.
  std::string spec_;

  // Set when the given URI is valid. Otherwise, we may still have a spec and
  // components, but they may not identify valid resources (for example, an
  // invalid port number, invalid characters in the scheme, etc.).
  bool is_valid_;

  // Identified components of the canonical spec.
  uri::Parsed parsed_;
};

// The TelURI object accepts only TEL-URI schemes.
class TelURI {
 public:
  // Creates an empty, invalid TEL-URI.
  TelURI();

  // Creates a TEL-URI from a GURL object.
  explicit TelURI(const GURL &other);

  // Copy construction is relatively inexpensive, with most of the time going
  // to reallocating the string. It does not re-parse.
  TelURI(const TelURI& other);

  // The narrow version requires the input be UTF-8. Invalid UTF-8 input will
  // result in an invalid URI.
  //
  // The wide version should also take an encoding parameter so we know how to
  // encode the query parameters. It is probably sufficient for the narrow
  // version to assume the query parameter encoding should be the same as the
  // input encoding.
  explicit TelURI(const std::string& uri_string);
  explicit TelURI(const base::string16& uri_string);

  // Constructor for URIs that have already been parsed and canonicalized. The
  // caller must supply all information associated with the URI, which must be
  // correct and consistent.
  TelURI(const char* canonical_spec, size_t canonical_spec_len,
         const uri::Parsed& parsed, bool is_valid);

  ~TelURI();

  TelURI& operator=(const TelURI& other);

  // Convert a TEL-URI into SIP-URI using RFC3261, section 19.1.6.
  SipURI ToSipURI(const SipURI& origin);

  // Returns true when this object represents a valid parsed URI. When not
  // valid, other functions will still succeed, but you will not get canonical
  // data out in the format you may be expecting. Instead, we keep something
  // "reasonable looking" so that the user can see how it's busted if
  // displayed to them.
  bool is_valid() const {
    return is_valid_;
  }

  // Returns true if the URI is zero-length. Note that empty URIs are also
  // invalid, and is_valid() will return false for them. This is provided
  // because some users may want to treat the empty case differently.
  bool is_empty() const {
    return spec_.empty();
  }

  // Returns the raw spec, i.e., the full text of the URI, in canonical UTF-8,
  // if the URI is valid. If the URI is not valid, this will assert and return
  // the empty string (for safety in release builds, to keep them from being
  // misused which might be a security problem).
  //
  // The URI will be ASCII except the reference fragment, which may be UTF-8.
  // It is guaranteed to be valid UTF-8.
  //
  // The exception is for empty() URIs (which are !is_valid()) but this will
  // return the empty string without asserting.
  //
  // Used invalid_spec() below to get the unusable spec of an invalid URI. This
  // separation is designed to prevent errors that may cause security problems
  // that could result from the mistaken use of an invalid URI.
  const std::string& spec() const;

  // Returns the potentially invalid spec for a the URI. This spec MUST NOT be
  // modified or sent over the network. It is designed to be displayed in error
  // messages to the user, as the apperance of the spec may explain the error.
  // If the spec is valid, the valid spec will be returned.
  //
  // The returned string is guaranteed to be valid UTF-8.
  const std::string& possibly_invalid_spec() const {
    return spec_;
  }

  // Getter for the raw parsed structure. This allows callers to locate parts
  // of the URI within the spec themselves. Most callers should consider using
  // the individual component getters below.
  //
  // The returned parsed structure will reference into the raw spec, which may
  // or may not be valid. If you are using this to index into the spec, BE
  // SURE YOU ARE USING possibly_invalid_spec() to get the spec, and that you
  // don't do anything "important" with invalid specs.
  const uri::Parsed& parsed_for_possibly_invalid_spec() const {
    return parsed_;
  }

  // Defiant equality operator!
  bool operator==(const TelURI& other) const {
    return spec_ == other.spec_;
  }
  bool operator!=(const TelURI& other) const {
    return spec_ != other.spec_;
  }

  // Allows URI to used as a key in STL (for example, a std::set or std::map).
  bool operator<(const TelURI& other) const {
    return spec_ < other.spec_;
  }

  // Getters for various components of the URI. The returned string will be
  // empty if the component is empty or is not present.
  std::string scheme() const {  // Not including the colon. See also SchemeIs.
    return uri_details::ComponentString(spec_, parsed_.scheme);
  }
  std::string telephone_subscriber() const {
    return uri_details::ComponentString(spec_, parsed_.username);
  }
  std::string parameters() const {  // Including first semicolon following host
    return uri_details::ComponentString(spec_, parsed_.parameters);
  }

  // Existance querying. These functions will return true if the corresponding
  // URI component exists in this URI. Note that existance is different than
  // being nonempty. sip:user@domain.com? has headers that just happens to
  // be empty, and has_headers() will return true.
  bool has_scheme() const {
    return parsed_.scheme.len >= 0;
  }
  bool has_telephone_subscriber() const {
    return parsed_.username.len >= 0;
  }
  bool has_parameters() const {
    return parsed_.parameters.len >= 0;
  }

  // Swaps the contents of this URI object with the argument without doing
  // any memory allocations.
  void Swap(TelURI* other);

  // Returns a reference to a singleton empty URI. This object is for callers
  // who return references but don't have anything to return in some cases.
  // This function may be called from any thread.
  static const TelURI& EmptyURI();

  // Returns the given parameter if available.
  std::pair<bool, std::string> parameter(const std::string &name) const;

 private:
  // The actual text of the URI, in canonical ASCII form.
  std::string spec_;

  // Set when the given URI is valid. Otherwise, we may still have a spec and
  // components, but they may not identify valid resources (for example, an
  // invalid port number, invalid characters in the scheme, etc.).
  bool is_valid_;

  // Identified components of the canonical spec.
  uri::Parsed parsed_;
};

} // End of sippet namespace

// Stream operator so URI can be used in assertion statements.
std::ostream& operator<<(std::ostream& out, const sippet::SipURI& uri);
std::ostream& operator<<(std::ostream& out, const sippet::TelURI& uri);

#endif // SIPPET_MESSAGE_URI_H_
