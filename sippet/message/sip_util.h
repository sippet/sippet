// Copyright (c) 2013-2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_SIP_UTIL_H_
#define SIPPET_SIP_UTIL_H_

#include <stddef.h>
#include <stdint.h>

#include "base/strings/string_tokenizer.h"

namespace sippet {

class SipUtil {
 public:
  // Parses the value of a Content-Type header.  The resulting mime_type and
  // charset values are normalized to lowercase.  The mime_type and charset
  // output values are only modified if the content_type_str contains a mime
  // type and charset value, respectively.  The boundary output value is
  // optional and will be assigned the (quoted) value of the boundary
  // parameter, if any.
  static void ParseContentType(const std::string& content_type_str,
                               std::string* mime_type,
                               std::string* charset,
                               bool* had_charset,
                               std::string* boundary);

  // Multiple occurances of some headers cannot be coalesced into a comma-
  // separated list.
  static bool IsNonCoalescingHeader(std::string::const_iterator name_begin,
                                    std::string::const_iterator name_end);
  static bool IsNonCoalescingHeader(const std::string& name) {
    return IsNonCoalescingHeader(name.begin(), name.end());
  }

  // Used to iterate over the name/value pairs of SIP headers.  To iterate
  // over the values in a multi-value header, use ValuesIterator.
  // See AssembleRawHeaders for joining line continuations (this iterator
  // does not expect any).
  class HeadersIterator {
   public:
    HeadersIterator(std::string::const_iterator headers_begin,
                    std::string::const_iterator headers_end,
                    const std::string& line_delimiter);
    ~HeadersIterator();

    // Advances the iterator to the next header, if any.  Returns true if there
    // is a next header.  Use name* and values* methods to access the resultant
    // header name and values.
    bool GetNext();

    // Iterates through the list of headers, starting with the current position
    // and looks for the specified header.  Note that the name _must_ be
    // lower cased.
    // If the header was found, the return value will be true and the current
    // position points to the header.  If the return value is false, the
    // current position will be at the end of the headers.
    bool AdvanceTo(const char* lowercase_name);

    void Reset() {
      lines_.Reset();
    }

    std::string::const_iterator name_begin() const {
      return name_begin_;
    }
    std::string::const_iterator name_end() const {
      return name_end_;
    }
    std::string name() const {
      return std::string(name_begin_, name_end_);
    }

    std::string::const_iterator values_begin() const {
      return values_begin_;
    }
    std::string::const_iterator values_end() const {
      return values_end_;
    }
    std::string values() const {
      return std::string(values_begin_, values_end_);
    }

   private:
    base::StringTokenizer lines_;
    std::string::const_iterator name_begin_;
    std::string::const_iterator name_end_;
    std::string::const_iterator values_begin_;
    std::string::const_iterator values_end_;
  };

  // Iterates over delimited values in an SIP header.  SIP LWS is
  // automatically trimmed from the resulting values.
  //
  // When using this class to iterate over response header values, be aware that
  // for some headers (e.g., Last-Modified), commas are not used as delimiters.
  // This iterator should be avoided for headers like that which are considered
  // non-coalescing (see IsNonCoalescingHeader).
  //
  // This iterator is careful to skip over delimiters found inside an SIP
  // quoted string.
  //
  class ValuesIterator {
   public:
    ValuesIterator(std::string::const_iterator values_begin,
                   std::string::const_iterator values_end,
                   char delimiter);
    ValuesIterator(const ValuesIterator& other);
    ~ValuesIterator();

    // Set the characters to regard as quotes.  By default, this includes both
    // single and double quotes.
    void set_quote_chars(const char* quotes) {
      values_.set_quote_chars(quotes);
    }

    // Advances the iterator to the next value, if any.  Returns true if there
    // is a next value.  Use value* methods to access the resultant value.
    bool GetNext();

    std::string::const_iterator value_begin() const {
      return value_begin_;
    }
    std::string::const_iterator value_end() const {
      return value_end_;
    }
    std::string value() const {
      return std::string(value_begin_, value_end_);
    }

   private:
    base::StringTokenizer values_;
    std::string::const_iterator value_begin_;
    std::string::const_iterator value_end_;
  };

  // Iterates over a delimited sequence of name-value pairs in an SIP header.
  // Each pair consists of a token (the name), an equals sign, and either a
  // token or quoted-string (the value). Arbitrary SIP LWS is permitted outside
  // of and between names, values, and delimiters.
  //
  // String iterators returned from this class' methods may be invalidated upon
  // calls to GetNext() or after the NameValuePairsIterator is destroyed.
  class NameValuePairsIterator {
   public:
    // Whether or not values are optional. Values::NOT_REQUIRED allows
    // e.g. name1=value1;name2;name3=value3, whereas Vaues::REQUIRED
    // will treat it as a parse error because name2 does not have a
    // corresponding equals sign.
    enum class Values { NOT_REQUIRED, REQUIRED };

    // Whether or not unmatched quotes should be considered a failure. By
    // default this class is pretty lenient and does a best effort to parse
    // values with mismatched quotes. When set to STRICT_QUOTES a value with
    // mismatched or otherwise invalid quotes is considered a parse error.
    enum class Quotes { STRICT_QUOTES, NOT_STRICT };

    NameValuePairsIterator(std::string::const_iterator begin,
                           std::string::const_iterator end,
                           char delimiter,
                           Values optional_values,
                           Quotes strict_quotes);

    // Treats values as not optional by default (Values::REQUIRED) and
    // treats quotes as not strict.
    NameValuePairsIterator(std::string::const_iterator begin,
                           std::string::const_iterator end,
                           char delimiter);

    NameValuePairsIterator(const NameValuePairsIterator& other);

    ~NameValuePairsIterator();

    // Advances the iterator to the next pair, if any.  Returns true if there
    // is a next pair.  Use name* and value* methods to access the resultant
    // value.
    bool GetNext();

    // Returns false if there was a parse error.
    bool valid() const { return valid_; }

    // The name of the current name-value pair.
    std::string::const_iterator name_begin() const { return name_begin_; }
    std::string::const_iterator name_end() const { return name_end_; }
    std::string name() const { return std::string(name_begin_, name_end_); }

    // The value of the current name-value pair.
    std::string::const_iterator value_begin() const {
      return value_is_quoted_ ? unquoted_value_.begin() : value_begin_;
    }
    std::string::const_iterator value_end() const {
      return value_is_quoted_ ? unquoted_value_.end() : value_end_;
    }
    std::string value() const {
      return value_is_quoted_ ? unquoted_value_ : std::string(value_begin_,
                                                              value_end_);
    }

    bool value_is_quoted() const { return value_is_quoted_; }

    // The value before unquoting (if any).
    std::string raw_value() const { return std::string(value_begin_,
                                                       value_end_); }

   private:
    bool IsQuote(char c) const;

    SipUtil::ValuesIterator props_;
    bool valid_;

    std::string::const_iterator name_begin_;
    std::string::const_iterator name_end_;

    std::string::const_iterator value_begin_;
    std::string::const_iterator value_end_;

    // Do not store iterators into this string. The NameValuePairsIterator
    // is copyable/assignable, and if copied the copy's iterators would point
    // into the original's unquoted_value_ member.
    std::string unquoted_value_;

    bool value_is_quoted_;

    // True if values are required for each name/value pair; false if a
    // name is permitted to appear without a corresponding value.
    bool values_optional_;

    // True if quotes values are required to be properly quoted; false if
    // mismatched quotes and other problems with quoted values should be more
    // or less gracefully treated as valid.
    bool strict_quotes_;
  };
};

}  // namespace sippet

#endif  // SIPPET_SIP_UTIL_H_
