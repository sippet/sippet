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
     "Content-TYPE  : application/sdp; charset=utf-8  \n",

     "INVITE sip:user@example.com SIP/2.0\n"
     "Content-TYPE: application/sdp; charset=utf-8\n",

     true, "INVITE", "sip:user@example.com", SipVersion(2, 0)},
    {// Normalize the method name.
     "InViTe sip:user@example.com  SIP/2.0 \n"
     "Content-TYPE  : application/sdp; charset=utf-8  \n",

     "INVITE sip:user@example.com SIP/2.0\n"
     "Content-TYPE: application/sdp; charset=utf-8\n",

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
     "Content-TYPE  : application/sdp; charset=utf-8  \n"
     "Expires: 7200 \n"
     "Accept-Language:   en \n",

     "SIP/2.0 202 Accepted\n"
     "Content-TYPE: application/sdp; charset=utf-8\n"
     "Expires: 7200\n"
     "Accept-Language: en\n",

     true, SipVersion(2, 0), 202, "Accepted"},
    {// Normalize leading whitespace.
     "SIP/2.0    202   Accepted  \n"
     // Starts with space -- will be skipped as invalid.
     "  Content-TYPE  : application/sdp; charset=utf-8  \n"
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
     "Content-TYPE: application/sdp; charset=utf-8\n",

     "SIP/2.0 201\n"
     "Content-TYPE: application/sdp; charset=utf-8\n",

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
     "c:  application/sdp; charset=ISO-8859-4 \n"
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
     "Content-Type: application/sdp; charset=ISO-8859-4\n"
     "From: <sip:c8oqz84zk7z@privacy.org>;tag=hyh8\n"
     "Subject: Tech Support\n"
     "Supported: 100rel\n"
     "To: <sip:+12125551212@server.phone2net.com>\n"
     "Via: SIP/2.0/UDP erlang.bell-telephone.com:5060 ;branch=z9hG4bK87asdks7\n",

     true, SipVersion(2, 0), 202, "Accepted"},
    {// Normalize contact-like headers.
     "SIP/2.0 202 Accepted  \n"
     "M:Mr.    John    <sips:bob@192.0.2.4>;expires=60\n"
     "Contact: \"Foo, Bar;\" <sips:bob@192.0.2.8> ; expires=60, "
       "sip:crazy@stuff ;tag=abcd\n"
     "f:sip:c8oqz84zk7z@privacy.org;tag=hyh8 \n"
     "t: \"\" <sip:+12125551212@server.phone2net.com>\n",

     "SIP/2.0 202 Accepted\n"
     "Contact: \"Mr. John\" <sips:bob@192.0.2.4>;expires=60\n"
     "Contact: \"Foo, Bar;\" <sips:bob@192.0.2.8>;expires=60, "
       "<sip:crazy@stuff>;tag=abcd\n"
     "From: <sip:c8oqz84zk7z@privacy.org>;tag=hyh8\n"
     "To: <sip:+12125551212@server.phone2net.com>\n",

     true, SipVersion(2, 0), 202, "Accepted"},
    {// Reject bad status line.
     "SCREWED_UP_START_LINE\n"
     "Content-TYPE: application/sdp; charset=utf-8\n",

     nullptr,

     false, SipVersion(), 0, nullptr},
    {// Reject bad version number.
     "SIP/1.0 202 Accepted\n"
     "Content-TYPE: application/sdp; charset=utf-8\n",

     nullptr,

     false, SipVersion(), 0, nullptr},
    {// Reject bad status code.
     "SIP/2.0 -1 Accepted\n"
     "Content-TYPE: application/sdp; charset=utf-8\n",

     nullptr,

     false, SipVersion(), 0, nullptr},
    {// Reject bad status code (2).
     "SIP/2.0 18 Accepted\n"
     "Content-TYPE: application/sdp; charset=utf-8\n",

     nullptr,

     false, SipVersion(), 0, nullptr},
    {// Reject bad status code (3).
     "SIP/2.0 4294967301 better not break the receiver\n"
     "Content-TYPE: application/sdp; charset=utf-8\n",

     nullptr,

     false, SipVersion(), 0, nullptr},
};

INSTANTIATE_TEST_CASE_P(SipResponseHeaders,
                        CommonSipResponsesTest,
                        testing::ValuesIn(response_headers_tests));

TEST(SipResponseHeadersTest, EnumerateHeader_Coalesced) {
  // Ensure that commas in quoted strings are not regarded as value separators.
  // Ensure that whitespace following a value is trimmed properly.
  std::string headers =
      "SIP/2.0 200 OK\n"
      "Via: SIP/2.0/UDP server10.biloxi.com"
        ";branch=z9hG4bKnashds8;received=192.0.2.3, "
        "SIP/2.0/UDP bigbox3.site3.atlanta.com"
        " ;branch=z9hG4bK77ef4c2312983.1;  received=192.0.2.2\n"
      "Via: SIP/2.0/UDP pc33.atlanta.com"
        ";branch=z9hG4bK776asdhds ;received=192.0.2.1\n";
  HeadersToRaw(&headers);
  scoped_refptr<Message> parsed(Message::Parse(headers));

  size_t iter = 0;
  std::string value;
  EXPECT_TRUE(parsed->EnumerateHeader(&iter, "via", &value));
  EXPECT_EQ("SIP/2.0/UDP server10.biloxi.com"
      ";branch=z9hG4bKnashds8;received=192.0.2.3",
      value);
  EXPECT_TRUE(parsed->EnumerateHeader(&iter, "via", &value));
  EXPECT_EQ("SIP/2.0/UDP bigbox3.site3.atlanta.com"
      " ;branch=z9hG4bK77ef4c2312983.1;  received=192.0.2.2",
      value);
  EXPECT_TRUE(parsed->EnumerateHeader(&iter, "via", &value));
  EXPECT_EQ("SIP/2.0/UDP pc33.atlanta.com"
      ";branch=z9hG4bK776asdhds ;received=192.0.2.1",
      value);
  EXPECT_FALSE(parsed->EnumerateHeader(&iter, "via", &value));
}

TEST(SipResponseHeadersTest, EnumerateHeader_Challenge) {
  // Even though WWW-Authenticate has commas, it should not be treated as
  // coalesced values.
  std::string headers =
      "SIP/2.0 401 OK\n"
      "WWW-Authenticate:Digest realm=foobar,  nonce=x, domain=y\n"
      "WWW-Authenticate:Basic realm=quatar\n";
  HeadersToRaw(&headers);
  scoped_refptr<Message> parsed(Message::Parse(headers));

  size_t iter = 0;
  std::string value;
  EXPECT_TRUE(parsed->EnumerateHeader(&iter, "WWW-Authenticate", &value));
  EXPECT_EQ("Digest realm=foobar,  nonce=x, domain=y", value);
  EXPECT_TRUE(parsed->EnumerateHeader(&iter, "WWW-Authenticate", &value));
  EXPECT_EQ("Basic realm=quatar", value);
  EXPECT_FALSE(parsed->EnumerateHeader(&iter, "WWW-Authenticate", &value));
}

TEST(SipResponseHeadersTest, EnumerateHeader_DateValued) {
  // The comma in a date valued header should not be treated as a
  // field-value separator.
  std::string headers =
      "SIP/2.0 200 OK\n"
      "Date: Tue, 07 Aug 2007 23:10:55 GMT\n";
  HeadersToRaw(&headers);
  scoped_refptr<Message> parsed(Message::Parse(headers));

  std::string value;
  EXPECT_TRUE(parsed->EnumerateHeader(NULL, "date", &value));
  EXPECT_EQ("Tue, 07 Aug 2007 23:10:55 GMT", value);
}

struct TimeValuedHeaderTestData {
  const char* raw_headers;
  const char* expected_date;
};

class TimeValuedHeaderTest
    : public SipMessagesTest,
      public testing::WithParamInterface<TimeValuedHeaderTestData> {
};

TimeValuedHeaderTestData time_valued_header_tests[] = {
    {// When the timezone is missing, GMT is a good guess as its what RFC2616
     // requires.
     "SIP/2.0 200 OK\n"
     "Date: Tue, 07 Aug 2007 23:10:55\n",

     "Tue, 07 Aug 2007 23:10:55 GMT"},
    {// If GMT is missing but an RFC822-conforming one is present, use that.
     "SIP/2.0 200 OK\n"
     "Date: Tue, 07 Aug 2007 19:10:55 EDT\n",

     "Tue, 07 Aug 2007 23:10:55 GMT"},
};

TEST_P(TimeValuedHeaderTest, DefaultDateToGMT) {
  const TimeValuedHeaderTestData test = GetParam();

  std::string raw_headers(test.raw_headers);
  HeadersToRaw(&raw_headers);

  scoped_refptr<Message> parsed(Message::Parse(raw_headers));
  base::Time expected_value;
  ASSERT_TRUE(base::Time::FromString(test.expected_date,
                                     &expected_value));

  base::Time value;
  EXPECT_TRUE(parsed->GetTimeValuedHeader("date", &value));
  EXPECT_EQ(expected_value, value);
}

INSTANTIATE_TEST_CASE_P(SipTimeValuedHeaders,
                        TimeValuedHeaderTest,
                        testing::ValuesIn(time_valued_header_tests));

TEST(SipResponseHeadersTest, GetExpiresValue10) {
  std::string headers =
      "SIP/2.0 200 OK\n"
      "Expires: 10\n";
  HeadersToRaw(&headers);
  scoped_refptr<Message> parsed(Message::Parse(headers));
  base::TimeDelta age;
  ASSERT_TRUE(parsed->GetExpiresValue(&age));
  EXPECT_EQ(10, age.InSeconds());
}

TEST(SipResponseHeadersTest, GetExpiresValue0) {
  std::string headers =
      "SIP/2.0 200 OK\n"
      "Expires: 0\n";
  HeadersToRaw(&headers);
  scoped_refptr<Message> parsed(Message::Parse(headers));
  base::TimeDelta age;
  ASSERT_TRUE(parsed->GetExpiresValue(&age));
  EXPECT_EQ(0, age.InSeconds());
}

TEST(SipResponseHeadersTest, GetExpiresValueBogus) {
  std::string headers =
      "SIP/2.0 200 OK\n"
      "Expires: donkey\n";
  HeadersToRaw(&headers);
  scoped_refptr<Message> parsed(Message::Parse(headers));
  base::TimeDelta age;
  ASSERT_FALSE(parsed->GetExpiresValue(&age));
}

TEST(SipResponseHeadersTest, GetExpiresValueNegative) {
  std::string headers =
      "SIP/2.0 200 OK\n"
      "Expires: -10\n";
  HeadersToRaw(&headers);
  scoped_refptr<Message> parsed(Message::Parse(headers));
  base::TimeDelta age;
  ASSERT_FALSE(parsed->GetExpiresValue(&age));
}

TEST(SipResponseHeadersTest, GetExpiresValueLeadingPlus) {
  std::string headers =
      "SIP/2.0 200 OK\n"
      "Expires: +10\n";
  HeadersToRaw(&headers);
  scoped_refptr<Message> parsed(Message::Parse(headers));
  base::TimeDelta age;
  ASSERT_FALSE(parsed->GetExpiresValue(&age));
}

TEST(SipResponseHeadersTest, GetExpiresValueOverflow) {
  std::string headers =
      "SIP/2.0 200 OK\n"
      "Expires: 999999999999999999999999999999999999999999\n";
  HeadersToRaw(&headers);
  scoped_refptr<Message> parsed(Message::Parse(headers));
  base::TimeDelta age;
  ASSERT_TRUE(parsed->GetExpiresValue(&age));

  // Should have saturated to 2^32 - 1.
  EXPECT_EQ(static_cast<int64_t>(0xFFFFFFFFL), age.InSeconds());
}

struct ContentTypeTestData {
  const std::string raw_headers;
  const std::string mime_type;
  const bool has_mimetype;
  const std::string charset;
  const bool has_charset;
  const std::string all_content_type;
};

class ContentTypeTest
    : public SipMessagesTest,
      public ::testing::WithParamInterface<ContentTypeTestData> {
};

TEST_P(ContentTypeTest, GetMimeType) {
  const ContentTypeTestData test = GetParam();

  std::string headers(test.raw_headers);
  HeadersToRaw(&headers);
  scoped_refptr<Message> parsed(Message::Parse(headers));

  std::string value;
  EXPECT_EQ(test.has_mimetype, parsed->GetMimeType(&value));
  EXPECT_EQ(test.mime_type, value);
  value.clear();
  EXPECT_EQ(test.has_charset, parsed->GetCharset(&value));
  EXPECT_EQ(test.charset, value);
  EXPECT_TRUE(parsed->GetNormalizedHeader("content-type", &value));
  EXPECT_EQ(test.all_content_type, value);
}

const ContentTypeTestData mimetype_tests[] = {
  { "SIP/2.0 200 OK\n"
    "Content-type: application/sdp\n",
    "application/sdp", true,
    "", false,
    "application/sdp" },
  // Multiple content-type headers should give us the last one.
  { "SIP/2.0 200 OK\n"
    "Content-type: application/sdp\n"
    "Content-type: application/sdp\n",
    "application/sdp", true,
    "", false,
    "application/sdp, application/sdp" },
  { "SIP/2.0 200 OK\n"
    "Content-type: text/plain\n"
    "Content-type: application/sdp\n"
    "Content-type: text/plain\n"
    "Content-type: application/sdp\n",
    "application/sdp", true,
    "", false,
    "text/plain, application/sdp, text/plain, application/sdp" },
  // Test charset parsing.
  { "SIP/2.0 200 OK\n"
    "Content-type: application/sdp\n"
    "Content-type: application/sdp; charset=ISO-8859-1\n",
    "application/sdp", true,
    "iso-8859-1", true,
    "application/sdp, application/sdp; charset=ISO-8859-1" },
  // Test charset in double quotes.
  { "SIP/2.0 200 OK\n"
    "Content-type: application/sdp\n"
    "Content-type: application/sdp; charset=\"ISO-8859-1\"\n",
    "application/sdp", true,
    "iso-8859-1", true,
    "application/sdp, application/sdp; charset=\"ISO-8859-1\"" },
  // If there are multiple matching content-type headers, we carry
  // over the charset value.
  { "SIP/2.0 200 OK\n"
    "Content-type: application/sdp;charset=utf-8\n"
    "Content-type: application/sdp\n",
    "application/sdp", true,
    "utf-8", true,
    "application/sdp;charset=utf-8, application/sdp" },
  // Test single quotes.
  { "SIP/2.0 200 OK\n"
    "Content-type: application/sdp;charset='utf-8'\n"
    "Content-type: application/sdp\n",
    "application/sdp", true,
    "utf-8", true,
    "application/sdp;charset='utf-8', application/sdp" },
  // Last charset wins if matching content-type.
  { "SIP/2.0 200 OK\n"
    "Content-type: application/sdp;charset=utf-8\n"
    "Content-type: application/sdp;charset=iso-8859-1\n",
    "application/sdp", true,
    "iso-8859-1", true,
    "application/sdp;charset=utf-8, application/sdp;charset=iso-8859-1" },
  // Charset is ignored if the content types change.
  { "SIP/2.0 200 OK\n"
    "Content-type: text/plain;charset=utf-8\n"
    "Content-type: application/sdp\n",
    "application/sdp", true,
    "", false,
    "text/plain;charset=utf-8, application/sdp" },
  // Empty content-type.
  { "SIP/2.0 200 OK\n"
    "Content-type: \n",
    "", false,
    "", false,
    "" },
  // Emtpy charset.
  { "SIP/2.0 200 OK\n"
    "Content-type: application/sdp;charset=\n",
    "application/sdp", true,
    "", false,
    "application/sdp;charset=" },
  // Multiple charsets, last one wins.
  { "SIP/2.0 200 OK\n"
    "Content-type: application/sdp;charset=utf-8; charset=iso-8859-1\n",
    "application/sdp", true,
    "iso-8859-1", true,
    "application/sdp;charset=utf-8; charset=iso-8859-1" },
  // Multiple params.
  { "SIP/2.0 200 OK\n"
    "Content-type: application/sdp; foo=utf-8; charset=iso-8859-1\n",
    "application/sdp", true,
    "iso-8859-1", true,
    "application/sdp; foo=utf-8; charset=iso-8859-1" },
  { "SIP/2.0 200 OK\n"
    "Content-type: application/sdp ; charset=utf-8 ; bar=iso-8859-1\n",
    "application/sdp", true,
    "utf-8", true,
    "application/sdp ; charset=utf-8 ; bar=iso-8859-1" },
  // Comma embeded in quotes.
  { "SIP/2.0 200 OK\n"
    "Content-type: application/sdp ; charset='utf-8,text/plain' ;\n",
    "application/sdp", true,
    "utf-8,text/plain", true,
    "application/sdp ; charset='utf-8,text/plain' ;" },
  // Charset with leading spaces.
  { "SIP/2.0 200 OK\n"
    "Content-type: application/sdp ; charset= 'utf-8' ;\n",
    "application/sdp", true,
    "utf-8", true,
    "application/sdp ; charset= 'utf-8' ;" },
  // Media type comments in mime-type.
  { "SIP/2.0 200 OK\n"
    "Content-type: application/sdp (html)\n",
    "application/sdp", true,
    "", false,
   "application/sdp (html)" },
  // Incomplete charset= param.
  { "SIP/2.0 200 OK\n"
    "Content-type: application/sdp; char=\n",
    "application/sdp", true,
    "", false,
    "application/sdp; char=" },
  // Invalid media type: no slash.
  { "SIP/2.0 200 OK\n"
    "Content-type: texthtml\n",
    "", false,
    "", false,
    "texthtml" },
  // Invalid media type: "*/*".
  { "SIP/2.0 200 OK\n"
    "Content-type: */*\n",
    "", false,
    "", false,
    "*/*" },
};

INSTANTIATE_TEST_CASE_P(SipResponseHeaders,
                        ContentTypeTest,
                        testing::ValuesIn(mimetype_tests));

struct EnumerateHeaderTestData {
  const char* headers;
  const char* expected_lines;
};

class EnumerateHeaderLinesTest
    : public SipMessagesTest,
      public ::testing::WithParamInterface<EnumerateHeaderTestData> {
};

TEST_P(EnumerateHeaderLinesTest, EnumerateHeaderLines) {
  const EnumerateHeaderTestData test = GetParam();

  std::string headers(test.headers);
  HeadersToRaw(&headers);
  scoped_refptr<Message> parsed(Message::Parse(headers));

  std::string name, value, lines;

  size_t iter = 0;
  while (parsed->EnumerateHeaderLines(&iter, &name, &value)) {
    lines.append(name);
    lines.append(": ");
    lines.append(value);
    lines.append("\n");
  }

  EXPECT_EQ(std::string(test.expected_lines), lines);
}

const EnumerateHeaderTestData enumerate_header_tests[] = {
  { "SIP/2.0 200 OK\n",

    ""
  },
  { "SIP/2.0 200 OK\n"
    "Foo: 1\n",

    "Foo: 1\n"
  },
  { "SIP/2.0 200 OK\n"
    "Foo: 1\n"
    "Bar: 2\n"
    "Foo: 3\n",

    "Foo: 1\nBar: 2\nFoo: 3\n"
  },
  { "SIP/2.0 200 OK\n"
    "Foo: 1, 2, 3\n",

    "Foo: 1, 2, 3\n"
  },
};

INSTANTIATE_TEST_CASE_P(SipResponseHeaders,
                        EnumerateHeaderLinesTest,
                        testing::ValuesIn(enumerate_header_tests));

struct ContentLengthTestData {
  const char* headers;
  int64_t expected_len;
};

class GetContentLengthTest
    : public SipMessagesTest,
      public ::testing::WithParamInterface<ContentLengthTestData> {
};

TEST_P(GetContentLengthTest, GetContentLength) {
  const ContentLengthTestData test = GetParam();

  std::string headers(test.headers);
  HeadersToRaw(&headers);
  scoped_refptr<Message> parsed(Message::Parse(headers));

  EXPECT_EQ(test.expected_len, parsed->GetContentLength());
}

const ContentLengthTestData content_length_tests[] = {
    {"SIP/2.0 200 OK\n", -1},
    {"SIP/2.0 200 OK\n"
     "Content-Length: 10\n",
     10},
    {"SIP/2.0 200 OK\n"
     "Content-Length: \n",
     -1},
    {"SIP/2.0 200 OK\n"
     "Content-Length: abc\n",
     -1},
    {"SIP/2.0 200 OK\n"
     "Content-Length: -10\n",
     -1},
    {"SIP/2.0 200 OK\n"
     "Content-Length:  +10\n",
     -1},
    {"SIP/2.0 200 OK\n"
     "Content-Length: 23xb5\n",
     -1},
    {"SIP/2.0 200 OK\n"
     "Content-Length: 0xA\n",
     -1},
    {"SIP/2.0 200 OK\n"
     "L: 010\n",
     10},
    // Content-Length too big, will overflow an int64_t.
    {"SIP/2.0 200 OK\n"
     "Content-Length: 40000000000000000000\n",
     -1},
    {"SIP/2.0 200 OK\n"
     "l:       10\n",
     10},
    {"SIP/2.0 200 OK\n"
     "Content-Length: 10  \n",
     10},
    {"SIP/2.0 200 OK\n"
     "Content-Length: \t10\n",
     10},
    {"SIP/2.0 200 OK\n"
     "Content-Length: \v10\n",
     -1},
    {"SIP/2.0 200 OK\n"
     "Content-Length: \f10\n",
     -1},
    {"SIP/2.0 200 OK\n"
     "cOnTeNt-LENgth: 33\n",
     33},
    {"SIP/2.0 200 OK\n"
     "Content-Length: 34\r\n",
     -1},
};

INSTANTIATE_TEST_CASE_P(SipResponseHeaders,
                        GetContentLengthTest,
                        testing::ValuesIn(content_length_tests));

struct AddHeaderTestData {
  const char* orig_headers;
  const char* new_header;
  const char* expected_headers;
};

class AddHeaderTest
    : public SipMessagesTest,
      public ::testing::WithParamInterface<AddHeaderTestData> {
};

TEST_P(AddHeaderTest, AddHeader) {
  const AddHeaderTestData test = GetParam();

  std::string orig_headers(test.orig_headers);
  HeadersToRaw(&orig_headers);
  scoped_refptr<Message> parsed(Message::Parse(orig_headers));

  std::string new_header(test.new_header);
  parsed->AddHeader(new_header);

  EXPECT_EQ(std::string(test.expected_headers), ToSimpleString(parsed));
}

const AddHeaderTestData add_header_tests[] = {
  { "SIP/2.0 200 OK\n"
    "Contact: <sip:alice@pc33.atlanta.com>\n"
    "CSeq: 314159 INVITE\n",

    "Content-Length: 450",

    "SIP/2.0 200 OK\n"
    "Contact: <sip:alice@pc33.atlanta.com>\n"
    "CSeq: 314159 INVITE\n"
    "Content-Length: 450\n"
  },
  { "SIP/2.0 200 OK\n"
    "Contact: <sip:alice@pc33.atlanta.com>\n"
    "CSeq: 314159 INVITE    \n",

    "Content-Length: 450  ",

    "SIP/2.0 200 OK\n"
    "Contact: <sip:alice@pc33.atlanta.com>\n"
    "CSeq: 314159 INVITE\n"
    "Content-Length: 450\n"
  },
};

INSTANTIATE_TEST_CASE_P(SipResponseHeaders,
                        AddHeaderTest,
                        testing::ValuesIn(add_header_tests));

struct RemoveHeaderTestData {
  const char* orig_headers;
  const char* to_remove;
  const char* expected_headers;
};

class RemoveHeaderTest
    : public SipMessagesTest,
      public ::testing::WithParamInterface<RemoveHeaderTestData> {
};

TEST_P(RemoveHeaderTest, RemoveHeader) {
  const RemoveHeaderTestData test = GetParam();

  std::string orig_headers(test.orig_headers);
  HeadersToRaw(&orig_headers);
  scoped_refptr<Message> parsed(Message::Parse(orig_headers));

  std::string name(test.to_remove);
  parsed->RemoveHeader(name);

  EXPECT_EQ(std::string(test.expected_headers), ToSimpleString(parsed));
}

const RemoveHeaderTestData remove_header_tests[] = {
  { "SIP/2.0 200 OK\n"
    "Contact: <sip:alice@pc33.atlanta.com>\n"
    "CSeq: 314159 INVITE\n"
    "Content-Length: 450\n",

    "content-length",

    "SIP/2.0 200 OK\n"
    "Contact: <sip:alice@pc33.atlanta.com>\n"
    "CSeq: 314159 INVITE\n"
  },
  { "SIP/2.0 200 OK\n"
    "Contact: <sip:alice@pc33.atlanta.com>  \n"
    "Content-Length  : 450  \n"
    "CSeq: 314159 INVITE\n",

    "content-length",

    "SIP/2.0 200 OK\n"
    "Contact: <sip:alice@pc33.atlanta.com>\n"
    "CSeq: 314159 INVITE\n"
  },
};

INSTANTIATE_TEST_CASE_P(SipResponseHeaders,
                        RemoveHeaderTest,
                        testing::ValuesIn(remove_header_tests));

struct RemoveHeadersTestData {
  const char* orig_headers;
  const char* to_remove[2];
  const char* expected_headers;
};

class RemoveHeadersTest
    : public SipMessagesTest,
      public ::testing::WithParamInterface<RemoveHeadersTestData> {};

TEST_P(RemoveHeadersTest, RemoveHeaders) {
  const RemoveHeadersTestData test = GetParam();

  std::string orig_headers(test.orig_headers);
  HeadersToRaw(&orig_headers);
  scoped_refptr<Message> parsed(Message::Parse(orig_headers));

  std::unordered_set<std::string> to_remove;
  for (auto* header : test.to_remove) {
    if (header)
      to_remove.insert(header);
  }
  parsed->RemoveHeaders(to_remove);

  EXPECT_EQ(std::string(test.expected_headers), ToSimpleString(parsed));
}

const RemoveHeadersTestData remove_headers_tests[] = {
    {"SIP/2.0 200 OK\n"
     "Contact: <sip:alice@pc33.atlanta.com>\n"
     "CSeq: 314159 INVITE\n"
     "Content-Length: 450\n",

     {"Content-Length", "CSEQ"},

     "SIP/2.0 200 OK\n"
     "Contact: <sip:alice@pc33.atlanta.com>\n"},

    {"SIP/2.0 200 OK\n"
     "Contact: <sip:alice@pc33.atlanta.com>\n"
     "Content-Length: 450\n",

     {"foo", "bar"},

     "SIP/2.0 200 OK\n"
     "Contact: <sip:alice@pc33.atlanta.com>\n"
     "Content-Length: 450\n"},

    {"SIP/2.0 404 Kinda not OK\n"
     "Contact: <sip:alice@pc33.atlanta.com>  \n",

     {},

     "SIP/2.0 404 Kinda not OK\n"
     "Contact: <sip:alice@pc33.atlanta.com>\n"},
};

INSTANTIATE_TEST_CASE_P(SipResponseHeaders,
                        RemoveHeadersTest,
                        testing::ValuesIn(remove_headers_tests));

struct ReplaceStatusTestData {
  const char* orig_headers;
  const char* new_status;
  const char* expected_headers;
};

class ReplaceStatusTest
    : public SipMessagesTest,
      public ::testing::WithParamInterface<ReplaceStatusTestData> {
};

TEST_P(ReplaceStatusTest, ReplaceStatus) {
  const ReplaceStatusTestData test = GetParam();

  std::string orig_headers(test.orig_headers);
  HeadersToRaw(&orig_headers);
  scoped_refptr<Message> parsed(Message::Parse(orig_headers));

  std::string name(test.new_status);
  parsed->ReplaceStartLine(name);

  EXPECT_EQ(std::string(test.expected_headers), ToSimpleString(parsed));
}

const ReplaceStatusTestData replace_status_tests[] = {
  { "SIP/2.0 206 Partial Content\n"
    "Contact: <sip:alice@pc33.atlanta.com>\n"
    "CSeq: 314159 INVITE\n"
    "Content-Length: 450\n",

    "SIP/2.0 200 OK",

    "SIP/2.0 200 OK\n"
    "Contact: <sip:alice@pc33.atlanta.com>\n"
    "CSeq: 314159 INVITE\n"
    "Content-Length: 450\n"
  },
  { "SIP/2.0 200 OK\n"
    "Contact: <sip:alice@pc33.atlanta.com>\n",

    "SIP/2.0 304 Not Modified",

    "SIP/2.0 304 Not Modified\n"
    "Contact: <sip:alice@pc33.atlanta.com>\n"
  },
};

INSTANTIATE_TEST_CASE_P(SipResponseHeaders,
                        ReplaceStatusTest,
                        testing::ValuesIn(replace_status_tests));

TEST(SipRequestHeadersTest, SetViaReceived) {
  std::string headers =
      "INVITE sip:bob@Biloxi.com SIP/2.0\n"
      "Via: SIP/2.0/UDP bobspc.biloxi.com:5060\n";
  HeadersToRaw(&headers);
  scoped_refptr<Message> parsed(Message::Parse(headers));
  parsed->SetViaReceived("192.0.2.4");
  EXPECT_EQ(
      "INVITE sip:bob@Biloxi.com SIP/2.0\n"
      "Via: SIP/2.0/UDP bobspc.biloxi.com:5060;received=192.0.2.4\n",
      ToSimpleString(parsed));
}

struct EnumerateContactLikeTestData {
  const char* raw_headers;
  const char* expected_display_name;
  const char* expected_address;
  std::unordered_map<std::string, std::string> expected_parameters;
};

EnumerateContactLikeTestData contact_like_tests[] = {
    {"SIP/2.0 200 OK\n"
     "Contact: sip:caller@u1.example.com\n",

     "", "sip:caller@u1.example.com",
    },
    {"SIP/2.0 200 OK\n"
     "Contact: sip:caller@u1.example.com;foo=bar\n",

     "", "sip:caller@u1.example.com", {{"foo", "bar"}},
    },
    {"SIP/2.0 200 OK\n"
     "Contact: sip:caller@u1.example.com;foo=\"bar\"\n",

     "", "sip:caller@u1.example.com", {{"foo", "bar"}},
    },
    {"SIP/2.0 200 OK\n"
     "Contact: sip:caller@u1.example.com;foo\n",

     "", "sip:caller@u1.example.com", {{"foo", ""}},
    },
    {"SIP/2.0 200 OK\n"
     "Contact: sip:caller@u1.example.com;foo=\"bar;baz\"\n",

     "", "sip:caller@u1.example.com", {{"foo", "bar;baz"}},
    },
    {"SIP/2.0 200 OK\n"
     "Contact: sip:caller@u1.example.com;foo=\"bar\\\"baz\"\n",

     "", "sip:caller@u1.example.com", {{"foo", "bar\"baz"}},
    },
    {"SIP/2.0 200 OK\n"
     "Contact: \"Mr, Magoo\" <sip:caller@u1.example.com>\n",

     "Mr, Magoo", "sip:caller@u1.example.com",
    },
};

class EnumerateContactLikeTest
    : public SipMessagesTest,
      public ::testing::WithParamInterface<EnumerateContactLikeTestData> {
};

TEST_P(EnumerateContactLikeTest, ReadValue) {
  const EnumerateContactLikeTestData test = GetParam();

  std::string orig_headers(test.raw_headers);
  HeadersToRaw(&orig_headers);
  scoped_refptr<Message> parsed(Message::Parse(orig_headers));

  std::string display_name;
  GURL address;
  std::unordered_map<std::string, std::string> parameters;
  parsed->EnumerateContactLikeHeader(nullptr, "contact", &display_name,
      &address, &parameters);

  EXPECT_EQ(std::string(test.expected_display_name), display_name);
  EXPECT_EQ(std::string(test.expected_address), address.spec());
  EXPECT_EQ(test.expected_parameters, parameters);
}

INSTANTIATE_TEST_CASE_P(SipResponseHeaders,
                        EnumerateContactLikeTest,
                        testing::ValuesIn(contact_like_tests));

struct CSeqTestData {
  const char* raw_headers;
  int64_t expected_sequence;
  const char *expected_method;
};

CSeqTestData cseq_tests[] = {
    {"SIP/2.0 200 OK\n"
     "CSeq: 314159 INVITE\n",

     314159, "INVITE",
    },
    {"SIP/2.0 200 OK\n"
     "CSeq: 0009 INVITE\n",

     9, "INVITE",
    },
    {"SIP/2.0 200 OK\n"
     "CSeq: 36893488147419103232 REGISTER\n",

     -1,
    },
    {"SIP/2.0 200 OK\n"
     "CSeq: 123\n",

     -1,
    },
    {"SIP/2.0 200 OK\n"
     "CSeq:\n",

     -1,
    },
};

class CSeqTest
    : public SipMessagesTest,
      public ::testing::WithParamInterface<CSeqTestData> {
};

TEST_P(CSeqTest, ReadValue) {
  const CSeqTestData test = GetParam();

  std::string orig_headers(test.raw_headers);
  HeadersToRaw(&orig_headers);
  scoped_refptr<Message> parsed(Message::Parse(orig_headers));

  std::string method;
  int64_t sequence = parsed->GetCSeq(&method);

  EXPECT_EQ(test.expected_sequence, sequence);
  if (sequence >= 0) {
    EXPECT_EQ(test.expected_method, method);
  }
}

INSTANTIATE_TEST_CASE_P(SipResponseHeaders,
                        CSeqTest,
                        testing::ValuesIn(cseq_tests));

}  // namespace

}  // namespace sippet
