// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/message.h"
#include "sippet/uri/uri.h"
#include "net/base/ip_endpoint.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace sippet {

class HeaderTest : public testing::Test {
 public:
  void HasExpectedRequestUri(const char *request_header,
                             const char *expected_request_uri) {
    scoped_refptr<Request> request(dyn_cast<Request>(Message::Parse(
      request_header)));

    ASSERT_TRUE(request.get());
    ASSERT_TRUE(request->request_uri().is_valid());
    ASSERT_TRUE(request->request_uri().SchemeIs("sip"));

    SipURI uri(request->request_uri());
    if (!expected_request_uri) {
      ASSERT_FALSE(uri.is_valid());
    } else {
      ASSERT_TRUE(uri.is_valid());
      EXPECT_EQ(GURL(expected_request_uri), request->request_uri());
    }
  }

  void HasViaReceived(const char *via_header,
                      const char *expected_received_parameter) {
    scoped_ptr<Header> header(Header::Parse(via_header));
    Via *via = dyn_cast<Via>(header);

    ASSERT_FALSE(via->empty());
    ASSERT_EQ(expected_received_parameter, via->front().received());
  }
};

TEST_F(HeaderTest, Method) {
  Method null;
  EXPECT_EQ(Method::Unknown, null.type());
  EXPECT_STREQ("", null.str());

  Method invite(Method::INVITE);
  EXPECT_EQ(Method::INVITE, invite.type());
  EXPECT_STREQ("INVITE", invite.str());

  Method foobar("FOOBAR");
  EXPECT_EQ(Method::Unknown, foobar.type());
  EXPECT_STREQ("FOOBAR", foobar.str());

  foobar = invite;
  EXPECT_EQ(Method::INVITE, foobar.type());

  foobar = null;
  EXPECT_EQ(Method::Unknown, foobar.type());

  Method coerce("INVITE");  // coersion test
  EXPECT_EQ(Method::INVITE, coerce.type());
  EXPECT_STREQ("INVITE", coerce.str());
}

TEST_F(HeaderTest, Accept) {
  scoped_ptr<Accept> accept(new Accept);

  Header *h = accept.get();
  EXPECT_TRUE(isa<Accept>(h));

  accept->push_back(MediaRange("application", "sdp"));
  accept->back().set_qvalue(1.0);
  accept->push_back(MediaRange("application", "*"));

  EXPECT_FALSE(accept->empty());

  std::string buffer;
  raw_string_ostream os(buffer);
  accept->print(os);

  EXPECT_EQ("Accept: application/sdp;q=1.0, application/*", os.str());

  EXPECT_TRUE(accept->front().HasQvalue());
  EXPECT_EQ(1.0, accept->front().qvalue());
}

TEST_F(HeaderTest, AcceptEncoding) {
  scoped_ptr<AcceptEncoding> accept_encoding(new AcceptEncoding);

  Header *h = accept_encoding.get();
  EXPECT_TRUE(isa<AcceptEncoding>(h));

  accept_encoding->push_back(Encoding("gzip"));
  accept_encoding->back().set_qvalue(0.1);
  accept_encoding->push_back(Encoding("7zip"));

  EXPECT_FALSE(accept_encoding->empty());

  std::string buffer;
  raw_string_ostream os(buffer);
  accept_encoding->print(os);

  EXPECT_EQ("Accept-Encoding: gzip;q=0.1, 7zip", os.str());
}

TEST_F(HeaderTest, AcceptLanguage) {
  scoped_ptr<AcceptLanguage> accept_language(new AcceptLanguage);

  Header *h = accept_language.get();
  EXPECT_TRUE(isa<AcceptLanguage>(h));

  accept_language->push_back(LanguageRange("en"));
  accept_language->back().set_qvalue(0.9);
  accept_language->push_back(LanguageRange("pt-br"));

  EXPECT_FALSE(accept_language->empty());

  std::string buffer;
  raw_string_ostream os(buffer);
  accept_language->print(os);

  EXPECT_EQ("Accept-Language: en;q=0.9, pt-br", os.str());

  EXPECT_TRUE(accept_language->front().HasQvalue());
  EXPECT_EQ(0.9, accept_language->front().qvalue());
}

TEST_F(HeaderTest, AlertInfo) {
  scoped_ptr<AlertInfo> alert_info(new AlertInfo);
  alert_info->push_back(AlertParam(
      GURL("http://www.example.com/sounds/moo.wav")));

  Header *h = alert_info.get();
  EXPECT_TRUE(isa<AlertInfo>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  alert_info->print(os);

  EXPECT_EQ("Alert-Info: <http://www.example.com/sounds/moo.wav>", os.str());
}

TEST_F(HeaderTest, Allow) {
  scoped_ptr<Allow> allow(new Allow);
  allow->push_back(Method("INVITE"));
  allow->push_back(Method::ACK);
  allow->push_back(Method::BYE);

  Header *h = allow.get();
  EXPECT_TRUE(isa<Allow>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  allow->print(os);

  EXPECT_EQ("Allow: INVITE, ACK, BYE", os.str());

  Method myarray[] = {
    Method::REGISTER,
    Method::CANCEL,
    Method("Invite"),
    Method::ACK,
    Method::BYE,
  };

  allow->assign(myarray, myarray+5);
  buffer.clear();

  allow->print(os);
  EXPECT_EQ("Allow: REGISTER, CANCEL, INVITE, ACK, BYE", os.str());
}

TEST_F(HeaderTest, AuthenticationInfo) {
  scoped_ptr<AuthenticationInfo> authentication_info(new AuthenticationInfo);
  authentication_info->set_nextnonce("47364c23432d2e131a5fb210812c");
  authentication_info->set_qop(AuthenticationInfo::auth);
  authentication_info->set_rspauth("xxx");
  authentication_info->set_cnonce("0a4f113b");
  authentication_info->set_nc(1);

  Header *h = authentication_info.get();
  EXPECT_TRUE(isa<AuthenticationInfo>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  authentication_info->print(os);

  EXPECT_EQ("Authentication-Info: "
      "nextnonce=\"47364c23432d2e131a5fb210812c\", "
      "qop=auth, rspauth=\"xxx\", cnonce=\"0a4f113b\", "
      "nc=00000001", os.str());
}

TEST_F(HeaderTest, Authorization) {
  scoped_ptr<Authorization> authorization(
      new Authorization(Authorization::Digest));
  authorization->set_username("Alice");
  authorization->set_realm("atlanta.com");
  authorization->set_nonce("84a4cc6f3082121f32b42a2187831a9e");
  authorization->set_response("7587245234b3434cc3412213e5f113a5432");

  Header *h = authorization.get();
  EXPECT_TRUE(isa<Authorization>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  authorization->print(os);

  EXPECT_EQ("Authorization: Digest username=\"Alice\", "
      "realm=\"atlanta.com\", nonce=\"84a4cc6f3082121f32b42a2187831a9e\", "
      "response=\"7587245234b3434cc3412213e5f113a5432\"", os.str());
}

TEST_F(HeaderTest, CallId) {
  scoped_ptr<CallId> callid(
      new CallId("f81d4fae-7dec-11d0-a765-00a0c91e6bf6@biloxi.com"));

  Header *h = callid.get();
  EXPECT_TRUE(isa<CallId>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  callid->print(os);

  EXPECT_EQ("i: f81d4fae-7dec-11d0-a765-00a0c91e6bf6@biloxi.com", os.str());
}

TEST_F(HeaderTest, CallInfo) {
  scoped_ptr<CallInfo> call_info(new CallInfo);
  call_info->push_back(Info(GURL("http://wwww.example.com/alice/photo.jpg")));
  call_info->back().set_purpose(Info::icon);
  call_info->push_back(Info(GURL("http://www.example.com/alice/")));
  call_info->back().set_purpose(Info::info);

  Header *h = call_info.get();
  EXPECT_TRUE(isa<CallInfo>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  call_info->print(os);

  EXPECT_EQ("Call-Info: "
      "<http://wwww.example.com/alice/photo.jpg>;purpose=icon, "
      "<http://www.example.com/alice/>;purpose=info", os.str());
}

TEST_F(HeaderTest, Contact) {
  scoped_ptr<Contact> contact(new Contact);

  Header *h = contact.get();
  EXPECT_TRUE(isa<Contact>(h));

  contact->push_back(ContactInfo(GURL("sip:foo@bar.com"), "John Doe"));
  contact->back().set_qvalue(1.0);
  contact->push_back(ContactInfo(GURL("sip:bar@foo.com")));
  contact->back().set_expires(300);

  EXPECT_FALSE(contact->empty());

  std::string buffer;
  raw_string_ostream os(buffer);
  contact->print(os);

  EXPECT_EQ("m: \"John Doe\" <sip:foo@bar.com>;q=1.0, "
      "<sip:bar@foo.com>;expires=300", os.str());
}

TEST_F(HeaderTest, ContentDisposition) {
  scoped_ptr<ContentDisposition> content_disposition(
      new ContentDisposition("attachment"));
  content_disposition->param_set("filename", "smime.p7m");
  content_disposition->set_handling(ContentDisposition::required);

  Header *h = content_disposition.get();
  EXPECT_TRUE(isa<ContentDisposition>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  content_disposition->print(os);

  EXPECT_EQ("Content-Disposition: "
      "attachment;filename=smime.p7m;handling=required", os.str());
}

TEST_F(HeaderTest, ContentEncoding) {
  scoped_ptr<ContentEncoding> content_encoding(new ContentEncoding("gzip"));

  Header *h = content_encoding.get();
  EXPECT_TRUE(isa<ContentEncoding>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  content_encoding->print(os);

  EXPECT_EQ("e: gzip", os.str());
}

TEST_F(HeaderTest, ContentLanguage) {
  scoped_ptr<ContentLanguage> content_language(new ContentLanguage);
  content_language->push_back("en");
  content_language->push_back("pt-br");

  Header *h = content_language.get();
  EXPECT_TRUE(isa<ContentLanguage>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  content_language->print(os);

  EXPECT_EQ("Content-Language: en, pt-br", os.str());
}

TEST_F(HeaderTest, ContentLength) {
  scoped_ptr<ContentLength> content_length(new ContentLength(0));

  EXPECT_EQ(0, content_length->value());

  Header *h = content_length.get();
  EXPECT_TRUE(isa<ContentLength>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  content_length->print(os);

  EXPECT_EQ("l: 0", os.str());
}

TEST_F(HeaderTest, ContentType) {
  scoped_ptr<ContentType> content_type(
      new ContentType(MediaType("application", "sdp")));

  Header *h = content_type.get();
  EXPECT_TRUE(isa<ContentType>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  content_type->print(os);

  EXPECT_EQ("c: application/sdp", os.str());
}

TEST_F(HeaderTest, Cseq) {
  scoped_ptr<Cseq> cseq(new Cseq(1, Method::REGISTER));

  EXPECT_EQ(1, cseq->sequence());
  EXPECT_EQ(Method::REGISTER, cseq->method());

  Header *h = cseq.get();
  EXPECT_TRUE(isa<Cseq>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  cseq->print(os);

  EXPECT_EQ("CSeq: 1 REGISTER", os.str());
}

TEST_F(HeaderTest, Date) {
  base::Time t(base::Time::FromJsTime(62123.4512345));
  scoped_ptr<Date> date(new Date(t));

  EXPECT_EQ(date->value(), t);

  Header *h = date.get();
  EXPECT_TRUE(isa<Date>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  date->print(os);

  EXPECT_EQ("Date: Thu, 01 Jan 1970 00:01:02 GMT", os.str());
}

TEST_F(HeaderTest, ErrorInfo) {
  scoped_ptr<ErrorInfo> error_info(new ErrorInfo);
  error_info->push_back(ErrorUri(
      GURL("sip:not-in-service-recording@atlanta.com")));

  Header *h = error_info.get();
  EXPECT_TRUE(isa<ErrorInfo>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  error_info->print(os);

  EXPECT_EQ("Error-Info: <sip:not-in-service-recording@atlanta.com>", os.str());
}

TEST_F(HeaderTest, Expires) {
  scoped_ptr<Expires> expires(new Expires(300));

  Header *h = expires.get();
  EXPECT_TRUE(isa<Expires>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  expires->print(os);

  EXPECT_EQ("Expires: 300", os.str());
}

TEST_F(HeaderTest, From) {
  scoped_ptr<From> from(
      new From(GURL("sip:agb@bell-telephone.com"), "A. G. Bell"));
  from->set_tag("a48s");

  Header *h = from.get();
  EXPECT_TRUE(isa<From>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  from->print(os);

  EXPECT_EQ("f: \"A. G. Bell\" <sip:agb@bell-telephone.com>;tag=a48s",
      os.str());
}

TEST_F(HeaderTest, InReplyTo) {
  scoped_ptr<InReplyTo> in_reply_to(new InReplyTo);
  in_reply_to->push_back("70710@saturn.bell-tel.com");
  in_reply_to->push_back("17320@saturn.bell-tel.com");

  Header *h = in_reply_to.get();
  EXPECT_TRUE(isa<InReplyTo>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  in_reply_to->print(os);

  EXPECT_EQ("In-Reply-To: 70710@saturn.bell-tel.com, "
      "17320@saturn.bell-tel.com", os.str());
}

TEST_F(HeaderTest, MaxForwards) {
  scoped_ptr<MaxForwards> max_forwards(new MaxForwards(70));

  EXPECT_EQ(70, max_forwards->value());

  Header *h = max_forwards.get();
  EXPECT_TRUE(isa<MaxForwards>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  max_forwards->print(os);

  EXPECT_EQ("Max-Forwards: 70", os.str());
}

TEST_F(HeaderTest, MimeVersion) {
  scoped_ptr<MimeVersion> mime_version(new MimeVersion(1, 0));

  EXPECT_EQ(1, mime_version->major());
  EXPECT_EQ(0, mime_version->minor());

  Header *h = mime_version.get();
  EXPECT_TRUE(isa<MimeVersion>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  mime_version->print(os);

  EXPECT_EQ("MIME-Version: 1.0", os.str());
}

TEST_F(HeaderTest, MinExpires) {
  scoped_ptr<MinExpires> min_expires(new MinExpires(5));

  EXPECT_EQ(5, min_expires->value());

  Header *h = min_expires.get();
  EXPECT_TRUE(isa<MinExpires>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  min_expires->print(os);

  EXPECT_EQ("Min-Expires: 5", os.str());
}

TEST_F(HeaderTest, Organization) {
  scoped_ptr<Organization> organization(new Organization("Boxes by Bob"));

  EXPECT_EQ("Boxes by Bob", organization->value());

  Header *h = organization.get();
  EXPECT_TRUE(isa<Organization>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  organization->print(os);

  EXPECT_EQ("Organization: Boxes by Bob", os.str());
}

TEST_F(HeaderTest, Priority) {
  scoped_ptr<Priority> priority(new Priority(Priority::emergency));

  EXPECT_EQ("emergency", priority->value());

  Header *h = priority.get();
  EXPECT_TRUE(isa<Priority>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  priority->print(os);

  EXPECT_EQ("Priority: emergency", os.str());
}

TEST_F(HeaderTest, ProxyAuthenticate) {
  scoped_ptr<ProxyAuthenticate> proxy_authenticate(
      new ProxyAuthenticate(ProxyAuthenticate::Digest));
  proxy_authenticate->set_realm("atlanta.com");
  proxy_authenticate->set_domain("sip:ss1.carrier.com");
  proxy_authenticate->set_qop("auth");
  proxy_authenticate->set_nonce("f84f1cec41e6cbe5aea9c8e88d359");
  proxy_authenticate->set_opaque("");
  proxy_authenticate->set_stale(false);
  proxy_authenticate->set_algorithm(ProxyAuthenticate::MD5);

  Header *h = proxy_authenticate.get();
  EXPECT_TRUE(isa<ProxyAuthenticate>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  proxy_authenticate->print(os);

  EXPECT_EQ("Proxy-Authenticate: Digest realm=\"atlanta.com\", "
      "domain=\"sip:ss1.carrier.com\", qop=\"auth\", "
      "nonce=\"f84f1cec41e6cbe5aea9c8e88d359\", opaque=\"\", "
      "stale=false, algorithm=MD5", os.str());
}

TEST_F(HeaderTest, ProxyAuthorization) {
  scoped_ptr<ProxyAuthorization> proxy_authorization(
      new ProxyAuthorization(ProxyAuthorization::Digest));
  proxy_authorization->set_username("Alice");
  proxy_authorization->set_realm("atlanta.com");
  proxy_authorization->set_nonce("c60f3082ee1212b402a21831ae");
  proxy_authorization->set_response("245f23415f11432b3434341c022");

  Header *h = proxy_authorization.get();
  EXPECT_TRUE(isa<ProxyAuthorization>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  proxy_authorization->print(os);

  EXPECT_EQ("Proxy-Authorization: Digest username=\"Alice\", "
      "realm=\"atlanta.com\", nonce=\"c60f3082ee1212b402a21831ae\", "
      "response=\"245f23415f11432b3434341c022\"", os.str());
}

TEST_F(HeaderTest, ProxyRequire) {
  scoped_ptr<ProxyRequire> proxy_require(new ProxyRequire);
  proxy_require->push_back("foo");

  Header *h = proxy_require.get();
  EXPECT_TRUE(isa<ProxyRequire>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  proxy_require->print(os);

  EXPECT_EQ("Proxy-Require: foo", os.str());
}

TEST_F(HeaderTest, RecordRoute) {
  scoped_ptr<RecordRoute> record_route(
      new RecordRoute(RouteParam(GURL("sip:p2.example.com;lr"))));
  record_route->push_back(RouteParam(GURL("sip:p1.example.com;lr")));

  Header *h = record_route.get();
  EXPECT_TRUE(isa<RecordRoute>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  record_route->print(os);

  EXPECT_EQ("Record-Route: <sip:p2.example.com;lr>, "
      "<sip:p1.example.com;lr>", os.str());
}

TEST_F(HeaderTest, ReplyTo) {
  scoped_ptr<ReplyTo> reply_to(
      new ReplyTo(GURL("sip:bob@biloxi.com"), "Bob"));

  Header *h = reply_to.get();
  EXPECT_TRUE(isa<ReplyTo>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  reply_to->print(os);

  EXPECT_EQ("Reply-To: \"Bob\" <sip:bob@biloxi.com>", os.str());
}

TEST_F(HeaderTest, Require) {
  scoped_ptr<Require> require(new Require("100rel"));

  Header *h = require.get();
  EXPECT_TRUE(isa<Require>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  require->print(os);

  EXPECT_EQ("Require: 100rel", os.str());
}

TEST_F(HeaderTest, RetryAfter) {
  scoped_ptr<RetryAfter> retry_after(new RetryAfter(300));

  Header *h = retry_after.get();
  EXPECT_TRUE(isa<RetryAfter>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  retry_after->print(os);

  EXPECT_EQ("Retry-After: 300", os.str());
}

TEST_F(HeaderTest, Route) {
  scoped_ptr<Route> route(new Route(RouteParam(GURL("sip:alice@atlanta.com"))));

  Header *h = route.get();
  EXPECT_TRUE(isa<Route>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  route->print(os);

  EXPECT_EQ("Route: <sip:alice@atlanta.com>", os.str());
}

TEST_F(HeaderTest, Subject) {
  scoped_ptr<Subject> subject(new Subject("Need more boxes"));

  Header *h = subject.get();
  EXPECT_TRUE(isa<Subject>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  subject->print(os);

  EXPECT_EQ("s: Need more boxes", os.str());
}

TEST_F(HeaderTest, Supported) {
  scoped_ptr<Supported> supported(new Supported("100rel"));

  Header *h = supported.get();
  EXPECT_TRUE(isa<Supported>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  supported->print(os);

  EXPECT_EQ("k: 100rel", os.str());
}

TEST_F(HeaderTest, Timestamp) {
  scoped_ptr<Timestamp> timestamp(new Timestamp(100, 2.2345));

  Header *h = timestamp.get();
  EXPECT_TRUE(isa<Timestamp>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  timestamp->print(os);

  EXPECT_EQ("Timestamp: 100 2.2345", os.str());
}

TEST_F(HeaderTest, To) {
  scoped_ptr<To> to(
      new To(GURL("sip:operator@cs.columbia.edu"), "The Operator"));
  to->set_tag("287447");

  Header *h = to.get();
  EXPECT_TRUE(isa<To>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  to->print(os);

  EXPECT_EQ("t: \"The Operator\" "
      "<sip:operator@cs.columbia.edu>;tag=287447", os.str());
}

TEST_F(HeaderTest, Unsupported) {
  scoped_ptr<Unsupported> unsupported(new Unsupported("foo"));

  Header *h = unsupported.get();
  EXPECT_TRUE(isa<Unsupported>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  unsupported->print(os);

  EXPECT_EQ("Unsupported: foo", os.str());
}

TEST_F(HeaderTest, UserAgent) {
  scoped_ptr<UserAgent> user_agent(new UserAgent("Softphone Beta1.5"));

  Header *h = user_agent.get();
  EXPECT_TRUE(isa<UserAgent>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  user_agent->print(os);

  EXPECT_EQ("User-Agent: Softphone Beta1.5", os.str());
}

TEST_F(HeaderTest, Via) {
  scoped_ptr<Via> via(new Via);
  via->push_back(ViaParam(Protocol::UDP,
      net::HostPortPair("pc33.atlanta.com", 0)));
  via->back().set_branch("z9hG4bK776asdhds");

  Header *h = via.get();
  EXPECT_TRUE(isa<Via>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  via->print(os);

  EXPECT_EQ("v: SIP/2.0/UDP "
      "pc33.atlanta.com;rport;branch=z9hG4bK776asdhds", os.str());
}

TEST_F(HeaderTest, Warning) {
  scoped_ptr<Warning> warning(new Warning);
  warning->push_back(
      WarnParam(370, "devnull", "Choose a bigger pipe"));
  warning->push_back(
      WarnParam(307, "isi.edu", "Session parameter 'foo' not understood"));

  Header *h = warning.get();
  EXPECT_TRUE(isa<Warning>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  warning->print(os);

  EXPECT_EQ("Warning: 370 devnull \"Choose a bigger pipe\", "
      "307 isi.edu \"Session parameter 'foo' not understood\"", os.str());
}

TEST_F(HeaderTest, WwwAuthenticate) {
  scoped_ptr<WwwAuthenticate> www_authenticate(
      new WwwAuthenticate(WwwAuthenticate::Digest));
  www_authenticate->set_realm("atlanta.com");
  www_authenticate->set_domain("sip:boxesbybob.com");
  www_authenticate->set_qop("auth");
  www_authenticate->set_nonce("f84f1cec41e6cbe5aea9c8e88d359");
  www_authenticate->set_opaque("");
  www_authenticate->set_stale(false);
  www_authenticate->set_algorithm(WwwAuthenticate::MD5);

  Header *h = www_authenticate.get();
  EXPECT_TRUE(isa<WwwAuthenticate>(h));

  std::string buffer;
  raw_string_ostream os(buffer);
  www_authenticate->print(os);

  EXPECT_EQ("WWW-Authenticate: Digest realm=\"atlanta.com\", "
      "domain=\"sip:boxesbybob.com\", qop=\"auth\", "
      "nonce=\"f84f1cec41e6cbe5aea9c8e88d359\", opaque=\"\", "
      "stale=false, algorithm=MD5", os.str());
}

TEST_F(HeaderTest, TortureIpv6Good) {
  scoped_refptr<Request> request(dyn_cast<Request>(Message::Parse(
    "REGISTER sip:[2001:db8::10] SIP/2.0\r\n"
    "To: sip:user@[2001:db8::10]\r\n"
    "From: sip:user@[2001:db8::10];tag=81x2\r\n"
    "Via: SIP/2.0/UDP [2001:db8::9:1];branch=z9hG4bKas3-111\r\n"
    "Contact: \"Caller\" <sip:caller@[2001:db8::1]>\r\n"
    "Route: <sip:[2001:db8::2]>\r\n"
    "Record-Route: <sip:[2001:db8::3]>\r\n"
    "\r\n")));

  EXPECT_EQ(GURL("sip:[2001:db8::10]"), request->request_uri());

  To* to = request->get<To>();
  ASSERT_TRUE(to);
  EXPECT_EQ(GURL("sip:user@[2001:db8::10]"), to->address());

  From* from = request->get<From>();
  ASSERT_TRUE(from);
  EXPECT_EQ(GURL("sip:user@[2001:db8::10]"), to->address());

  Via* via = request->get<Via>();
  ASSERT_TRUE(via);
  ASSERT_FALSE(via->empty());
  // parser always assume default port
  EXPECT_TRUE(net::HostPortPair("2001:db8::9:1", 5060)
    .Equals(via->front().sent_by()));

  Contact* contact = request->get<Contact>();
  ASSERT_TRUE(contact);
  ASSERT_FALSE(contact->empty());
  EXPECT_EQ(GURL("sip:caller@[2001:db8::1]"), contact->front().address());

  Route* route = request->get<Route>();
  ASSERT_TRUE(route);
  ASSERT_FALSE(route->empty());
  EXPECT_EQ(GURL("sip:[2001:db8::2]"), route->front().address());

  RecordRoute* record_route = request->get<RecordRoute>();
  ASSERT_TRUE(record_route);
  ASSERT_FALSE(record_route->empty());
  EXPECT_EQ(GURL("sip:[2001:db8::3]"), record_route->front().address());
}

TEST_F(HeaderTest, TortureIpv6Bad) {
  HasExpectedRequestUri(
    "REGISTER sip:2001:db8::10 SIP/2.0\r\n"
    "\r\n",
    nullptr);
}

TEST_F(HeaderTest, TorturePortAmbiguous) {
  HasExpectedRequestUri(
    "REGISTER sip:[2001:db8::10:5070] SIP/2.0\r\n"
    "\r\n",
    "sip:[2001:db8::10:5070]");
}

TEST_F(HeaderTest, TorturePortUnambiguous) {
  HasExpectedRequestUri(
    "REGISTER sip:[2001:db8::10]:5070 SIP/2.0\r\n"
    "\r\n",
    "sip:[2001:db8::10]:5070");
}

TEST_F(HeaderTest, TortureSemicolonInRequestUri) {
  HasExpectedRequestUri(
    "OPTIONS sip:user;par=u%40example.net@example.com SIP/2.0\r\n"
    "\r\n",
    "sip:user;par=u%40example.net@example.com");
}

TEST_F(HeaderTest, TortureViaReceivedDelims) {
  // Implementations must follow the Robustness Principle [RFC1122] and be
  // liberal in accepting a "received" parameter with or without the
  // delimiting "[" and "]" tokens.
  HasViaReceived(
    "Via: SIP/2.0/UDP [2001:db8::9:1];received=[2001:db8::9:255]",
    "2001:db8::9:255");
  HasViaReceived(
    "Via: SIP/2.0/UDP [2001:db8::9:1];received=2001:db8::9:255",
    "2001:db8::9:255");

  // When sending a request, implementations must not put the delimiting "["
  // and "]" tokens.
  scoped_ptr<Via> via(new Via(ViaParam(Protocol::UDP,
    net::HostPortPair("2001:db8::9:1", 0))));
  via->front().set_received("[2001:db8::9:255]");
  EXPECT_EQ("v: SIP/2.0/UDP [2001:db8::9:1];rport;received=2001:db8::9:255",
    via->ToString());
}

}  // namespace sippet
