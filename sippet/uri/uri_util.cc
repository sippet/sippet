// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/uri/uri_util.h"
#include "sippet/uri/uri_canon.h"

namespace sippet {
namespace uri_util {

namespace {

// Given a string and a range inside the string, compares it to the given
// lower-case |compare_to| buffer.
template<typename CHAR>
inline bool DoCompareSchemeComponent(const CHAR* spec,
                                     const uri_parse::Component& component,
                                     const char* compare_to) {
  if (!component.is_nonempty())
    return compare_to[0] == 0;  // When component is empty, match empty scheme.
  return uri_util::LowerCaseEqualsASCII(&spec[component.begin],
                                        &spec[component.end()],
                                        compare_to);
}

template<typename CHAR>
bool DoCanonicalize(const CHAR* in_spec, int in_spec_len,
                    uri_canon::CharsetConverter* charset_converter,
                    uri_canon::CanonOutput* output,
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
    success = uri_canon::CanonicalizeSipURI(spec, spec_len, parsed_input,
                                            charset_converter, output,
                                            output_parsed);
  } else if (DoCompareSchemeComponent(spec, scheme, "tel")) {
    uri_parse::ParseTelURI(spec, spec_len, &parsed_input);
    success = uri_canon::CanonicalizeTelURI(spec, spec_len, parsed_input,
                                            charset_converter, output,
                                            output_parsed);
  }
  return success;
}

} // End of empty namespace

bool Canonicalize(const char* spec,
                  int spec_len,
                  uri_canon::CharsetConverter* charset_converter,
                  uri_canon::CanonOutput* output,
                  uri_parse::Parsed* output_parsed) {
  return DoCanonicalize(spec, spec_len, charset_converter,
                        output, output_parsed);
}

bool Canonicalize(const char16* spec,
                  int spec_len,
                  uri_canon::CharsetConverter* charset_converter,
                  uri_canon::CanonOutput* output,
                  uri_parse::Parsed* output_parsed) {
  return DoCanonicalize(spec, spec_len, charset_converter,
                        output, output_parsed);
}

} // End of uri_util namespace
} // End of sippet namespace
