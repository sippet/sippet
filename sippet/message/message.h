// Copyright (c) 2013-2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_MESSAGE_H_
#define SIPPET_MESSAGE_MESSAGE_H_

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <unordered_set>
#include <vector>

#include "base/strings/string_piece.h"
#include "base/time/time.h"
#include "url/gurl.h"
#include "sippet/message/sip_version.h"
#include "sippet/message/sip_util.h"

namespace base {
class Time;
}

namespace sippet {

class Message {
 public:
  Message();
  ~Message();

  // Parses the given raw_headers.  raw_headers should be formatted thus: each
  // line is \0-terminated, and it's terminated by an empty line (ie, 2 \0s in
  // a row).  (Note that line continuations should have already been joined;
  // see SipUtil::AssembleRawMessage)
  bool Parse(const std::string& raw_headers);

  // Removes all instances of a particular header.
  void RemoveHeader(const std::string& name);

  // Removes all instances of particular headers.
  void RemoveMessage(const std::unordered_set<std::string>& header_names);

  // Removes a particular header line. The header name is compared
  // case-insensitively.
  void RemoveHeaderLine(const std::string& name, const std::string& value);

  // Adds a particular header.  |header| has to be a single header without any
  // EOL termination, just [<header-name>: <header-values>]
  // If a header with the same name is already stored, the two headers are not
  // merged together by this method; the one provided is simply put at the
  // end of the list.
  void AddHeader(const std::string& header);

  // Fetch the "normalized" value of a single header, where all values for the
  // header name are separated by commas.  See the GetNormalizedMessage for
  // format details.  Returns false if this header wasn't found.
  //
  // NOTE: Do not make any assumptions about the encoding of this output
  // string.  It may be non-ASCII, and the encoding used by the server is not
  // necessarily known to us.  Do not assume that this output is UTF-8!
  bool GetNormalizedHeader(const std::string& name, std::string* value) const;

  // Returns the normalized start line.
  std::string GetStartLine() const;

  // Enumerate the "lines" of the response headers.  This skips over the status
  // line.  Use GetStatusLine if you are interested in that.  Note that this
  // method returns the un-coalesced response header lines, so if a response
  // header appears on multiple lines, then it will appear multiple times in
  // this enumeration (in the order the header lines were received from the
  // server).  Also, a given header might have an empty value.  Initialize a
  // 'size_t' variable to 0 and pass it by address to EnumerateHeaderLines.
  // Call EnumerateHeaderLines repeatedly until it returns false.  The
  // out-params 'name' and 'value' are set upon success.
  bool EnumerateHeaderLines(size_t* iter,
                            std::string* name,
                            std::string* value) const;

  // Enumerate the values of the specified header.   If you are only interested
  // in the first header, then you can pass nullptr for the 'iter' parameter.
  // Otherwise, to iterate across all values for the specified header,
  // initialize a 'size_t' variable to 0 and pass it by address to
  // EnumerateHeader. Note that a header might have an empty value. Call
  // EnumerateHeader repeatedly until it returns false.
  //
  // Unless a header is explicitly marked as non-coalescing (see
  // SipUtil::IsNonCoalescingHeader), headers that contain
  // comma-separated lists are treated "as if" they had been sent as
  // distinct headers. That is, a header of "Foo: a, b, c" would
  // enumerate into distinct values of "a", "b", and "c". This is also
  // true for headers that occur multiple times in a response; unless
  // they are marked non-coalescing, "Foo: a, b" followed by "Foo: c"
  // will enumerate to "a", "b", "c". Commas inside quoted strings are ignored,
  // for example a header of 'Foo: "a, b", "c"' would enumerate as '"a, b"',
  // '"c"'.
  //
  // This can cause issues for headers that might have commas in fields that
  // aren't quoted strings, for example a header of "Foo: <a, b>, <c>" would
  // enumerate as '<a', 'b>', '<c>', rather than as '<a, b>', '<c>'.
  //
  // To handle cases such as this, use GetNormalizedHeader to return the full
  // concatenated header, and then parse manually.
  bool EnumerateHeader(size_t* iter,
                       const base::StringPiece& name,
                       std::string* value) const;

  // Returns true if the response contains the specified header-value pair.
  // Both name and value are compared case insensitively.
  bool HasHeaderValue(const base::StringPiece& name,
                      const base::StringPiece& value) const;

  // Returns true if the response contains the specified header.
  // The name is compared case insensitively.
  bool HasHeader(const base::StringPiece& name) const;

  // Get the mime type and charset values in lower case form from the headers.
  // Empty strings are returned if the values are not present.
  void GetMimeTypeAndCharset(std::string* mime_type,
                             std::string* charset) const;

  // Get the mime type in lower case from the headers.  If there's no mime
  // type, returns false.
  bool GetMimeType(std::string* mime_type) const;

  // Get the charset in lower case from the headers.  If there's no charset,
  // returns false.
  bool GetCharset(std::string* charset) const;

  // Extracts the time value of a particular header.  This method looks for the
  // first matching header value and parses its value as a SIP-date.
  bool GetTimeValuedHeader(const std::string& name, base::Time* result) const;

  // Extracts the value of the Content-Length header or returns -1 if there is
  // no such header in the response.
  int64_t GetContentLength() const;

  // Extracts the value of the specified header or returns -1 if there is no
  // such header in the response.
  int64_t GetInt64HeaderValue(const std::string& header) const;

  // Extracts the value of the From header or returns false.
  bool GetFrom(std::string* display_name,
               GURL* address,
               SipUtil::NameValuePairsIterator* parameters) const;

  // Extracts the value of the To header or returns false.
  bool GetTo(std::string* display_name,
             GURL* address,
             SipUtil::NameValuePairsIterator* parameters) const;

  // Extracts the value of the Reply-To header or returns false.
  bool GetReplyTo(std::string* display_name,
                  GURL* address,
                  SipUtil::NameValuePairsIterator* parameters) const;

  // Extracts the values of the Contact header. The 'iter' parameter works like
  // the |EnumerateHeader| function.
  bool EnumerateContacts(size_t* iter,
                         std::string* display_name,
                         GURL* address,
                         SipUtil::NameValuePairsIterator* parameters) const;

  // Extracts the values of the Route header. The 'iter' parameter works like
  // the |EnumerateHeader| function.
  bool EnumerateRoutes(size_t* iter,
                       std::string* display_name,
                       GURL* address,
                       SipUtil::NameValuePairsIterator* parameters) const;

  // Extracts the values of the Record-Route header. The 'iter' parameter works
  // like the |EnumerateHeader| function.
  bool EnumerateRecordRoutes(
      size_t* iter,
      std::string* display_name,
      GURL* address,
      SipUtil::NameValuePairsIterator* parameters) const;

  // Enumerates Contact-like headers.
  bool EnumerateContactLikeMessage(
      size_t* iter,
      const base::StringPiece& name,
      std::string* display_name,
      GURL* address,
      SipUtil::NameValuePairsIterator* parameters) const;

  // Extracts the value of the CSeq header or returns -1.
  int64_t GetCSeq(std::string* method) const;

  // Returns whether the message is a request.
  bool IsRequest() const { return !request_method_.empty(); }

  // Returns whether the message is a response.
  bool IsResponse() const { return !IsRequest(); }

  // Returns the SIP request method normalized in uppercase.  This is empty if
  // the request method could not be parsed.
  const std::string& request_method() const { return request_method_; }

  // Returns the SIP request URI.  This is empty if the request URI could not
  // be parsed.
  const GURL& request_uri() const { return request_uri_; };

  // Returns the SIP response code.  This is -1 if the response code text could
  // not be parsed.
  int response_code() const { return response_code_; }

  // Get the SIP status text of the normalized status line.
  std::string GetStatusText() const;

  // Get the SIP version of the normalized status line.
  SipVersion GetSipVersion() const { return sip_version_; }

  // Returns the raw header string.
  const std::string& raw_headers() const { return raw_headers_; }

 private:
  using HeaderSet = std::unordered_set<std::string>;

  // The members of this structure point into raw_headers_.
  struct ParsedHeader;
  typedef std::vector<ParsedHeader> HeaderList;

  // Helper function for ParseStartLine.
  // Tries to extract the "SIP/X.Y" from a status line formatted like:
  //    SIP/2.0 200 OK
  // with line_begin and end pointing at the begin and end of this line.  If the
  // status line is malformed, returns SipVersion(0,0).
  static SipVersion ParseVersion(std::string::const_iterator line_begin,
                                 std::string::const_iterator line_end);

  // Tries to extract the start line from a header block, either a status line
  // or a request line.
  // Output will be a normalized version of this.
  bool ParseStartLine(std::string::const_iterator line_begin,
                      std::string::const_iterator line_end);

  // Tries to extract the request line from a header block.
  bool ParseRequestLine(std::string::const_iterator line_begin,
                        std::string::const_iterator line_end);

  // Tries to extract the status line from a header block.
  bool ParseStatusLine(std::string::const_iterator line_begin,
                       std::string::const_iterator line_end);

  // Find the header in our list (case-insensitive) starting with parsed_ at
  // index |from|.  Returns string::npos if not found.
  size_t FindHeader(size_t from, const base::StringPiece& search) const;

  // Add a header->value pair to our list.  If we already have header in our
  // list, append the value to it.
  void AddHeader(std::string::const_iterator name_begin,
                 std::string::const_iterator name_end,
                 std::string::const_iterator values_begin,
                 std::string::const_iterator values_end);

  // Add to parsed_ given the fields of a ParsedHeader object.
  void AddToParsed(std::string::const_iterator name_begin,
                   std::string::const_iterator name_end,
                   std::string::const_iterator value_begin,
                   std::string::const_iterator value_end);

  // Replaces the current headers with the merged version of |raw_headers| and
  // the current headers without the headers in |headers_to_remove|. Note that
  // |headers_to_remove| are removed from the current headers (before the
  // merge), not after the merge.
  void MergeWithMessage(const std::string& raw_headers,
                        const HeaderSet& headers_to_remove);

  // We keep a list of ParsedHeader objects.  These tell us where to locate the
  // header-value pairs within raw_headers_.
  HeaderList parsed_;

  // The raw_headers_ consists of the normalized status line (terminated with a
  // null byte) and then followed by the raw null-terminated headers from the
  // input that was passed to our constructor.
  std::string raw_headers_;

  // This is the parsed SIP response code.
  int response_code_;

  // This is the parsed SIP request method.
  std::string request_method_;

  // This is the parsed SIP Request-URI.
  GURL request_uri_;

  // The normalized sip version.
  SipVersion sip_version_;

  DISALLOW_COPY_AND_ASSIGN(Message);
};

}  // namespace sippet

#endif // SIPPET_MESSAGE_MESSAGE_H_
