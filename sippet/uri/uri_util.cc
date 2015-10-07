// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/uri/uri_util.h"

#include "sippet/uri/uri.h"

#include "url/url_canon_internal.h"

#include "base/strings/string_piece.h"
#include "base/strings/string_util.h"

namespace sippet {
namespace uri {

namespace {

// This template converts a given character type to the corresponding
// StringPiece type.
template<typename CHAR> struct CharToStringPiece {
};
template<> struct CharToStringPiece<char> {
  typedef base::StringPiece Piece;
};
template<> struct CharToStringPiece<base::char16> {
  typedef base::StringPiece16 Piece;
};

// Given a string and a range inside the string, compares it to the given
// lower-case |compare_to| buffer.
template<typename CHAR>
inline bool DoCompareSchemeComponent(const CHAR* spec,
                                     const uri::Component& component,
                                     const char* compare_to) {
  if (!component.is_nonempty())
    return compare_to[0] == 0;  // When component is empty, match empty scheme.
  return base::LowerCaseEqualsASCII(
      &spec[component.begin],
      &spec[component.begin + component.len],
      compare_to);
}

template<typename CHAR>
bool DoCanonicalize(const CHAR* in_spec, int in_spec_len,
                    uri::CharsetConverter* charset_converter,
                    uri::CanonOutput* output,
                    uri::Parsed* output_parsed) {
  // Remove any whitespace from the middle of the relative URL, possibly
  // copying to the new buffer.
  url::RawCanonOutputT<CHAR> whitespace_buffer;
  int spec_len;
  const CHAR* spec = url::RemoveURLWhitespace(in_spec, in_spec_len,
                                              &whitespace_buffer, &spec_len);

  uri::Parsed parsed_input;
  uri::Component scheme;
  if (!uri::ExtractScheme(spec, spec_len, &scheme))
    return false;

  // This is the parsed version of the input URL, we have to canonicalize it
  // before storing it in our object.
  bool success = false;
  if (DoCompareSchemeComponent(spec, scheme, "sip") ||
      DoCompareSchemeComponent(spec, scheme, "sips")) {
    uri::ParseSipURI(spec, spec_len, &parsed_input);
    success = uri::CanonicalizeSipURI(spec, spec_len, parsed_input,
                                            charset_converter, output,
                                            output_parsed);
  } else if (DoCompareSchemeComponent(spec, scheme, "tel")) {
    uri::ParseTelURI(spec, spec_len, &parsed_input);
    success = uri::CanonicalizeTelURI(spec, spec_len, parsed_input,
                                            charset_converter, output,
                                            output_parsed);
  }
  return success;
}

template<typename CHAR>
void DoDecodeURIEscapeSequences(const CHAR* input, int length,
                                uri::CanonOutput* output) {
  for (int i = 0; i < length; i++) {
    if (input[i] == '%') {
      unsigned char ch;
      if (url::DecodeEscaped(input, &i, length, &ch)) {
        output->push_back(ch);
      } else {
        // Invalid escape sequence, copy the percent literal.
        output->push_back('%');
      }
    } else {
      // Regular non-escaped 8-bit character.
      output->push_back(input[i]);
    }
  }
}

}  // namespace

bool Canonicalize(const char* spec,
                  int spec_len,
                  uri::CharsetConverter* charset_converter,
                  uri::CanonOutput* output,
                  uri::Parsed* output_parsed) {
  return DoCanonicalize(spec, spec_len, charset_converter,
                        output, output_parsed);
}

bool Canonicalize(const base::char16* spec,
                  int spec_len,
                  uri::CharsetConverter* charset_converter,
                  uri::CanonOutput* output,
                  uri::Parsed* output_parsed) {
  return DoCanonicalize(spec, spec_len, charset_converter,
                        output, output_parsed);
}

void DecodeURIEscapeSequences(const char* input, int length,
                              uri::CanonOutput* output) {
  DoDecodeURIEscapeSequences(input, length, output);
}

}  // namespace uri
}  // namespace sippet
