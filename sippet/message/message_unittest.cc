// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/message.h"

#include <string>

#include "base/memory/ptr_util.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace sippet {

namespace {

struct TestRequestData {
  const char* raw_headers;
  const char* expected_headers;
  bool expected_valid;
  const char* expected_method;
  const char* expected_request_uri;
  SipVersion expected_version;
};

struct TestResponseData {
  const char* raw_headers;
  const char* expected_headers;
  bool expected_valid;
  SipVersion expected_version;
  int expected_response_code;
  const char* expected_status_text;
};

class SipMessagesTest : public testing::Test {
};

class CommonSipRequestsTest
    : public SipMessagesTest,
      public testing::WithParamInterface<TestRequestData> {
};

class CommonSipResponsesTest
    : public SipMessagesTest,
      public testing::WithParamInterface<TestResponseData> {
};

// Transform "normal"-looking headers (\n-separated) to the appropriate
// input format for ParseRawHeaders (\0-separated).
void HeadersToRaw(std::string* headers) {
  std::replace(headers->begin(), headers->end(), '\n', '\0');
  if (!headers->empty())
    *headers += '\0';
}

// Returns a simple text serialization of the given
// |Message|. This is used by tests to verify that a
// |Message| matches an expectation string.
//
//  * One line per header, written as:
//        HEADER_NAME: HEADER_VALUE\n
//  * The original case of header names is preserved.
//  * Whitespace around head names/values is stripped.
//  * Repeated headers are not aggregated.
//  * Headers are listed in their original order.
std::string ToSimpleString(const scoped_refptr<Message>& parsed) {
  std::string result = parsed->GetStartLine() + "\n";

  size_t iter = 0;
  std::string name;
  std::string value;
  while (parsed->EnumerateHeaderLines(&iter, &name, &value)) {
    std::string new_line = name + ": " + value + "\n";

    // Verify that |name| and |value| do not contain '\n' (if they did
    // it would make this serialized format ambiguous).
    if (std::count(new_line.begin(), new_line.end(), '\n') != 1) {
      ADD_FAILURE() << "Unexpected characters in the header name or value: "
                    << new_line;
      return result;
    }

    result += new_line;
  }

  return result;
}

TEST_P(CommonSipRequestsTest, TestCommon) {
  const TestRequestData test = GetParam();

  std::string raw_headers(test.raw_headers);
  HeadersToRaw(&raw_headers);

  scoped_refptr<Message> parsed(Message::Parse(raw_headers));
  if (!parsed) {
    EXPECT_FALSE(test.expected_valid);
  } else {
    EXPECT_TRUE(test.expected_valid);

    std::string expected_headers(test.expected_headers);
    std::string headers = ToSimpleString(parsed);
 
    // Transform to readable output format (so it's easier to see diffs).
    std::replace(headers.begin(), headers.end(), ' ', '_');
    std::replace(headers.begin(), headers.end(), '\n', '\\');
    std::replace(expected_headers.begin(), expected_headers.end(), ' ', '_');
    std::replace(expected_headers.begin(), expected_headers.end(), '\n', '\\');
 
    EXPECT_EQ(expected_headers, headers);
 
    EXPECT_EQ(test.expected_method, parsed->request_method());
    EXPECT_EQ(test.expected_request_uri, parsed->request_uri().spec());
    EXPECT_TRUE(test.expected_version == parsed->GetSipVersion());
  }
}

TEST_P(CommonSipResponsesTest, TestCommon) {
  const TestResponseData test = GetParam();

  std::string raw_headers(test.raw_headers);
  HeadersToRaw(&raw_headers);

  scoped_refptr<Message> parsed(Message::Parse(raw_headers));
  if (!parsed) {
    EXPECT_FALSE(test.expected_valid);
  } else {
    EXPECT_TRUE(test.expected_valid);

    std::string expected_headers(test.expected_headers);
    std::string headers = ToSimpleString(parsed);
 
    // Transform to readable output format (so it's easier to see diffs).
    std::replace(headers.begin(), headers.end(), ' ', '_');
    std::replace(headers.begin(), headers.end(), '\n', '\\');
    std::replace(expected_headers.begin(), expected_headers.end(), ' ', '_');
    std::replace(expected_headers.begin(), expected_headers.end(), '\n', '\\');
 
    EXPECT_EQ(expected_headers, headers);
 
    EXPECT_TRUE(test.expected_version == parsed->GetSipVersion());
    EXPECT_EQ(test.expected_response_code, parsed->response_code());
    EXPECT_EQ(test.expected_status_text, parsed->GetStatusText());
  }
}

TestRequestData request_headers_tests[] = {
    {// Normalize whitespace.
     "INVITE   sip:user@example.com  SIP/2.0 \n"
     "Content-TYPE  : text/html; charset=utf-8  \n",

     "INVITE sip:user@example.com SIP/2.0\n"
     "Content-TYPE: text/html; charset=utf-8\n",

     true, "INVITE", "sip:user@example.com", SipVersion(2, 0)},
    {// Normalize the method name.
     "InViTe sip:user@example.com  SIP/2.0 \n"
     "Content-TYPE  : text/html; charset=utf-8  \n",

     "INVITE sip:user@example.com SIP/2.0\n"
     "Content-TYPE: text/html; charset=utf-8\n",

     true, "INVITE", "sip:user@example.com", SipVersion(2, 0)},
    {// Accept weird method names.
     "!interesting-Method0123456789_*+`.%indeed'~ "
     "sip:1_unusual.URI~(to-be!sure)&isn't+it$/crazy?,/;;*"
     ":&it+has=1,weird!*pas$wo~d_too.(doesn't-it)"
     "@example.com SIP/2.0\n",

     "!INTERESTING-METHOD0123456789_*+`.%INDEED'~ "
     "sip:1_unusual.URI~(to-be!sure)&isn't+it$/crazy?,/;;*"
     ":&it+has=1,weird!*pas$wo~d_too.(doesn't-it)"
     "@example.com SIP/2.0\n",

     true, "!INTERESTING-METHOD0123456789_*+`.%INDEED'~",
     "sip:1_unusual.URI~(to-be!sure)&isn't+it$/crazy?,/;;*"
     ":&it+has=1,weird!*pas$wo~d_too.(doesn't-it)"
     "@example.com", SipVersion(2, 0)},
    {// Accept semicolon in Request-URI.
     "OPTIONS sip:user;par=u%40example.net@example.com SIP/2.0\n",

     "OPTIONS sip:user;par=u%40example.net@example.com SIP/2.0\n",

     true, "OPTIONS", "sip:user;par=u%40example.net@example.com",
     SipVersion(2, 0)},
    {// Malformed SIP Request-URI.
     "INVITE sip:user@example.com; lr SIP/2.0\n",

     nullptr,

     false, nullptr, nullptr, SipVersion()},
    {// Unknown Protocol Version.
     "OPTIONS sip:t.watson@example.org SIP/7.0\n",

     nullptr,

     false, nullptr, nullptr, SipVersion()},
};

INSTANTIATE_TEST_CASE_P(SipRequestHeaders,
                        CommonSipRequestsTest,
                        testing::ValuesIn(request_headers_tests));

TestResponseData response_headers_tests[] = {
    {// Normalize whitespace.
     "SIP/2.0    202   Accepted  \n"
     "Content-TYPE  : text/html; charset=utf-8  \n"
     "Expires: 7200 \n"
     "Accept-Language:   en \n",

     "SIP/2.0 202 Accepted\n"
     "Content-TYPE: text/html; charset=utf-8\n"
     "Expires: 7200\n"
     "Accept-Language: en\n",

     true, SipVersion(2, 0), 202, "Accepted"},
    {// Normalize leading whitespace.
     "SIP/2.0    202   Accepted  \n"
     // Starts with space -- will be skipped as invalid.
     "  Content-TYPE  : text/html; charset=utf-8  \n"
     "Expires: 7200 \n"
     "Accept-Language:   en \n",

     "SIP/2.0 202 Accepted\n"
     "Expires: 7200\n"
     "Accept-Language: en\n",

     true, SipVersion(2, 0), 202, "Accepted"},
    {// Keep whitespace within status text.
     "SIP/2.0 404 Not   found  \n",

     "SIP/2.0 404 Not   found\n",

     true, SipVersion(2, 0), 404, "Not   found"},
    {// Normalize blank headers.
     "SIP/2.0 200 OK\n"
     "Header1 :          \n"
     "Header2: \n"
     "Header3:\n"
     "Header4\n"
     "Header5    :\n",

     "SIP/2.0 200 OK\n"
     "Header1: \n"
     "Header2: \n"
     "Header3: \n"
     "Header5: \n",

     true, SipVersion(2, 0), 200, "OK"},
    {// Normalize SIP/ case and do not add missing status text.
     "sIp/2.0 201\n"
     "Content-TYPE: text/html; charset=utf-8\n",

     "SIP/2.0 201\n"
     "Content-TYPE: text/html; charset=utf-8\n",

     true, SipVersion(2, 0), 201, ""},
    {// Normalize headers that start with a colon.
     "SIP/2.0    202   Accepted  \n"
     "foo: bar\n"
     ": a \n"
     " : b\n"
     "baz: blat \n",

     "SIP/2.0 202 Accepted\n"
     "foo: bar\n"
     "baz: blat\n",

     true, SipVersion(2, 0), 202, "Accepted"},
    {// Normalize headers that end with a colon.
     "SIP/2.0    202   Accepted  \n"
     "foo:   \n"
     "bar:\n"
     "baz: blat \n"
     "zip:\n",

     "SIP/2.0 202 Accepted\n"
     "foo: \n"
     "bar: \n"
     "baz: blat\n"
     "zip: \n",

     true, SipVersion(2, 0), 202, "Accepted"},
    {// Expand compact form headers.
     "SIP/2.0    202   Accepted  \n"
     "i:f81d4fae-7dec-11d0-a765-00a0c91e6bf6@192.0.2.4\n"
     "M:<sips:bob@192.0.2.4>;expires=60\n"
     "e: tar\n"
     "l:   173 \n"
     "c:  text/html; charset=ISO-8859-4 \n"
     "f: <sip:c8oqz84zk7z@privacy.org>;tag=hyh8 \n"
     "s: Tech Support \n"
     "k: 100rel\n"
     "t: sip:+12125551212@server.phone2net.com \n"
     "v: SIP/2.0/UDP erlang.bell-telephone.com:5060 ;branch=z9hG4bK87asdks7\n",

     "SIP/2.0 202 Accepted\n"
     "Call-ID: f81d4fae-7dec-11d0-a765-00a0c91e6bf6@192.0.2.4\n"
     "Contact: <sips:bob@192.0.2.4>;expires=60\n"
     "Content-Encoding: tar\n"
     "Content-Length: 173\n"
     "Content-Type: text/html; charset=ISO-8859-4\n"
     "From: <sip:c8oqz84zk7z@privacy.org>;tag=hyh8\n"
     "Subject: Tech Support\n"
     "Supported: 100rel\n"
     "To: sip:+12125551212@server.phone2net.com\n"
     "Via: SIP/2.0/UDP erlang.bell-telephone.com:5060 ;branch=z9hG4bK87asdks7\n",

     true, SipVersion(2, 0), 202, "Accepted"},
    {// Reject bad status line.
     "SCREWED_UP_START_LINE\n"
     "Content-TYPE: text/html; charset=utf-8\n",

     nullptr,

     false, SipVersion(), 0, nullptr},
    {// Reject bad version number.
     "SIP/1.0 202 Accepted\n"
     "Content-TYPE: text/html; charset=utf-8\n",

     nullptr,

     false, SipVersion(), 0, nullptr},
    {// Reject bad status code.
     "SIP/2.0 -1 Accepted\n"
     "Content-TYPE: text/html; charset=utf-8\n",

     nullptr,

     false, SipVersion(), 0, nullptr},
    {// Reject bad status code (2).
     "SIP/2.0 18 Accepted\n"
     "Content-TYPE: text/html; charset=utf-8\n",

     nullptr,

     false, SipVersion(), 0, nullptr},
    {// Reject bad status code (3).
     "SIP/2.0 4294967301 better not break the receiver\n"
     "Content-TYPE: text/html; charset=utf-8\n",

     nullptr,

     false, SipVersion(), 0, nullptr},
};

INSTANTIATE_TEST_CASE_P(SipResponseHeaders,
                        CommonSipResponsesTest,
                        testing::ValuesIn(response_headers_tests));

}  // namespace

}  // namespace sippet
