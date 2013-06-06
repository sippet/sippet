/* 
 * Copyright (c) 2013, Guilherme Balena Versiani
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies, 
 * either expressed or implied, of the FreeBSD Project.
 */

#include "sippet/message/message.h"

#include "sippet/message/headers/accept.h"
#include "sippet/message/headers/accept_encoding.h"
#include "sippet/message/headers/accept_language.h"
#include "sippet/message/headers/alert_info.h"
#include "sippet/message/headers/allow.h"
#include "sippet/message/headers/authentication_info.h"
#include "sippet/message/headers/authorization.h"
#include "sippet/message/headers/call_id.h"
#include "sippet/message/headers/call_info.h"
#include "sippet/message/headers/contact.h"
#include "sippet/message/headers/content_disposition.h"
#include "sippet/message/headers/content_encoding.h"
#include "sippet/message/headers/content_language.h"
#include "sippet/message/headers/content_length.h"
#include "sippet/message/headers/content_type.h"
#include "sippet/message/headers/cseq.h"
#include "sippet/message/headers/date.h"
#include "sippet/message/headers/error_info.h"
#include "sippet/message/headers/expires.h"
#include "sippet/message/headers/from.h"
#include "sippet/message/headers/bits/has_multiple.h"
#include "sippet/message/headers/bits/has_parameters.h"
#include "sippet/message/headers/in_reply_to.h"
#include "sippet/message/headers/max_forwards.h"
#include "sippet/message/headers/mime_version.h"
#include "sippet/message/headers/min_expires.h"
#include "sippet/message/headers/organization.h"
#include "sippet/message/headers/priority.h"
#include "sippet/message/headers/proxy_authenticate.h"
#include "sippet/message/headers/proxy_authorization.h"
#include "sippet/message/headers/proxy_require.h"
#include "sippet/message/headers/record_route.h"
#include "sippet/message/headers/reply_to.h"
#include "sippet/message/headers/require.h"
#include "sippet/message/headers/retry_after.h"
#include "sippet/message/headers/route.h"
#include "sippet/message/headers/server.h"
#include "sippet/message/headers/subject.h"
#include "sippet/message/headers/supported.h"
#include "sippet/message/headers/timestamp.h"
#include "sippet/message/headers/to.h"
#include "sippet/message/headers/unsupported.h"
#include "sippet/message/headers/user_agent.h"
#include "sippet/message/headers/via.h"
#include "sippet/message/headers/warning.h"
#include "sippet/message/headers/www_authenticate.h"

#include "testing/gtest/include/gtest/gtest.h"

using namespace sippet;

TEST(HeaderTest, Method) {
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

  Method coerce("INVITE"); // coersion test
  EXPECT_EQ(Method::INVITE, coerce.type());
  EXPECT_STREQ("INVITE", coerce.str());
}

TEST(HeaderTest, Accept) {
  scoped_ptr<Accept> accept(new Accept);

  Header *h = accept.get();
  EXPECT_TRUE(isa<Accept>(h));

  accept->push_back(MediaRange("application","sdp"));
  accept->back().set_qvalue(1.0);
  accept->push_back(MediaRange("application","*"));

  EXPECT_FALSE(accept->empty());

  std::string buffer;
  raw_string_ostream os(buffer);
  accept->print(os);

  EXPECT_EQ("Accept: application/sdp;q=1.0, application/*", os.str());

  EXPECT_TRUE(accept->front().HasQvalue());
  EXPECT_EQ(1.0, accept->front().qvalue());
}

TEST(HeaderTest, AcceptEncoding) {
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

TEST(HeaderTest, AcceptLanguage) {
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

TEST(HeaderTest, AlertInfo) {
  scoped_ptr<AlertInfo> alert_info(new AlertInfo);
  alert_info->push_back(AlertParam("http://www.example.com/sounds/moo.wav"));

  std::string buffer;
  raw_string_ostream os(buffer);
  alert_info->print(os);

  EXPECT_EQ("Alert-Info: <http://www.example.com/sounds/moo.wav>", os.str());
}

TEST(HeaderTest, Allow) {
  scoped_ptr<Allow> allow(new Allow);
  allow->push_back(Method("INVITE"));
  allow->push_back(Method::ACK);
  allow->push_back(Method::BYE);

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

TEST(HeaderTest, AuthenticationInfo) {
  scoped_ptr<AuthenticationInfo> authentication_info(new AuthenticationInfo);
  authentication_info->set_nextnonce("47364c23432d2e131a5fb210812c");
  authentication_info->set_qop(AuthenticationInfo::auth);
  authentication_info->set_rspauth("xxx");
  authentication_info->set_cnonce("0a4f113b");
  authentication_info->set_nc(1);

  std::string buffer;
  raw_string_ostream os(buffer);
  authentication_info->print(os);

  EXPECT_EQ("Authentication-Info: nextnonce=\"47364c23432d2e131a5fb210812c\", qop=auth, rspauth=\"xxx\", cnonce=\"0a4f113b\", nc=00000001", os.str());
}

TEST(HeaderTest, Authorization) {
  scoped_ptr<Authorization> authorization(new Authorization(Authorization::Digest));
  authorization->set_username("Alice");
  authorization->set_realm("atlanta.com");
  authorization->set_nonce("84a4cc6f3082121f32b42a2187831a9e");
  authorization->set_response("7587245234b3434cc3412213e5f113a5432");

  std::string buffer;
  raw_string_ostream os(buffer);
  authorization->print(os);

  EXPECT_EQ("Authorization: Digest username=\"Alice\", realm=\"atlanta.com\", nonce=\"84a4cc6f3082121f32b42a2187831a9e\", response=\"7587245234b3434cc3412213e5f113a5432\"", os.str());
}

TEST(HeaderTest, CallId) {
  scoped_ptr<CallId> callid(new CallId("f81d4fae-7dec-11d0-a765-00a0c91e6bf6@biloxi.com"));

  std::string buffer;
  raw_string_ostream os(buffer);
  callid->print(os);

  EXPECT_EQ("Call-ID: f81d4fae-7dec-11d0-a765-00a0c91e6bf6@biloxi.com", os.str());
}

TEST(HeaderTest, CallInfo) {
  scoped_ptr<CallInfo> call_info(new CallInfo);

  call_info->push_back(Info("http://wwww.example.com/alice/photo.jpg"));
  call_info->back().set_purpose(Info::icon);
  call_info->push_back(Info("http://www.example.com/alice/"));
  call_info->back().set_purpose(Info::info);

  std::string buffer;
  raw_string_ostream os(buffer);
  call_info->print(os);

  EXPECT_EQ("Call-Info: <http://wwww.example.com/alice/photo.jpg>;purpose=icon, <http://www.example.com/alice/>;purpose=info", os.str());
}

TEST(HeaderTest, Contact) {
  scoped_ptr<Contact> contact(new Contact);

  Header *h = contact.get();
  EXPECT_TRUE(isa<Contact>(h));

  contact->push_back(ContactInfo("sip:foo@bar.com","John Doe"));
  contact->back().set_qvalue(1.0);
  contact->push_back(ContactInfo("sip:bar@foo.com"));
  contact->back().set_expires(300);

  EXPECT_FALSE(contact->empty());

  std::string buffer;
  raw_string_ostream os(buffer);
  contact->print(os);

  EXPECT_EQ("Contact: \"John Doe\" <sip:foo@bar.com>;q=1.0, <sip:bar@foo.com>;expires=300", os.str());
}

TEST(HeaderTest, ContentDisposition) {
  scoped_ptr<ContentDisposition> content_disposition(new ContentDisposition("attachment"));
  content_disposition->param_set("filename","smime.p7m");
  content_disposition->set_handling(ContentDisposition::required);

  std::string buffer;
  raw_string_ostream os(buffer);
  content_disposition->print(os);

  EXPECT_EQ("Content-Disposition: attachment;filename=smime.p7m;handling=required", os.str());
}

TEST(HeaderTest, ContentEncoding) {
  scoped_ptr<ContentEncoding> content_encoding(new ContentEncoding("gzip"));

  std::string buffer;
  raw_string_ostream os(buffer);
  content_encoding->print(os);

  EXPECT_EQ("Content-Encoding: gzip", os.str());
}

TEST(HeaderTest, ContentLanguage) {
  scoped_ptr<ContentLanguage> content_language(new ContentLanguage);
  content_language->push_back("en");
  content_language->push_back("pt-br");

  std::string buffer;
  raw_string_ostream os(buffer);
  content_language->print(os);

  EXPECT_EQ("Content-Language: en, pt-br", os.str());
}

TEST(HeaderTest, ContentLength) {
  scoped_ptr<ContentLength> content_length(new ContentLength(0));

  EXPECT_EQ(0, content_length->value());

  std::string buffer;
  raw_string_ostream os(buffer);
  content_length->print(os);

  EXPECT_EQ("Content-Length: 0", os.str());
}

TEST(HeaderTest, ContentType) {
  scoped_ptr<ContentType> content_type(new ContentType(MediaType("application","sdp")));

  std::string buffer;
  raw_string_ostream os(buffer);
  content_type->print(os);

  EXPECT_EQ("Content-Type: application/sdp", os.str());
}

TEST(HeaderTest, CSeq) {
  scoped_ptr<CSeq> cseq(new CSeq(1, Method::REGISTER));

  EXPECT_EQ(1, cseq->sequence());
  EXPECT_EQ(Method::REGISTER, cseq->method());

  std::string buffer;
  raw_string_ostream os(buffer);
  cseq->print(os);

  EXPECT_EQ("CSeq: 1 REGISTER", os.str());
}

TEST(HeaderTest, Date) {
  base::Time t(base::Time::FromJsTime(62123.4512345));
  scoped_ptr<Date> date(new Date(t));

  EXPECT_EQ(date->value(), t);

  std::string buffer;
  raw_string_ostream os(buffer);
  date->print(os);

  EXPECT_EQ("Date: Thu, 01 Jan 1970 00:01:02 GMT", os.str());
}

TEST(HeaderTest, ErrorInfo) {
  scoped_ptr<ErrorInfo> error_info(new ErrorInfo);
  error_info->push_back(ErrorUri("sip:not-in-service-recording@atlanta.com"));

  std::string buffer;
  raw_string_ostream os(buffer);
  error_info->print(os);

  EXPECT_EQ("Error-Info: <sip:not-in-service-recording@atlanta.com>", os.str());
}

TEST(HeaderTest, Expires) {
  scoped_ptr<Expires> error_info(new Expires(300));

  std::string buffer;
  raw_string_ostream os(buffer);
  error_info->print(os);

  EXPECT_EQ("Expires: 300", os.str());
}

TEST(HeaderTest, From) {
  scoped_ptr<From> from(new From("sip:agb@bell-telephone.com", "A. G. Bell"));
  from->set_tag("a48s");

  std::string buffer;
  raw_string_ostream os(buffer);
  from->print(os);

  EXPECT_EQ("From: \"A. G. Bell\" <sip:agb@bell-telephone.com>;tag=a48s", os.str());
}

TEST(HeaderTest, InReplyTo) {
  scoped_ptr<InReplyTo> in_reply_to(new InReplyTo);
  in_reply_to->push_back("70710@saturn.bell-tel.com");
  in_reply_to->push_back("17320@saturn.bell-tel.com");

  std::string buffer;
  raw_string_ostream os(buffer);
  in_reply_to->print(os);

  EXPECT_EQ("In-Reply-To: 70710@saturn.bell-tel.com, 17320@saturn.bell-tel.com", os.str());
}

TEST(HeaderTest, MaxForwards) {
  scoped_ptr<MaxForwards> max_forwards(new MaxForwards(70));

  EXPECT_EQ(70, max_forwards->value());

  std::string buffer;
  raw_string_ostream os(buffer);
  max_forwards->print(os);

  EXPECT_EQ("Max-Forwards: 70", os.str());
}

TEST(HeaderTest, MimeVersion) {
  scoped_ptr<MimeVersion> mime_version(new MimeVersion(1,0));

  EXPECT_EQ(1, mime_version->major());
  EXPECT_EQ(0, mime_version->minor());

  std::string buffer;
  raw_string_ostream os(buffer);
  mime_version->print(os);

  EXPECT_EQ("MIME-Version: 1.0", os.str());
}

TEST(HeaderTest, MinExpires) {
  scoped_ptr<MinExpires> min_expires(new MinExpires(5));

  EXPECT_EQ(5, min_expires->value());

  std::string buffer;
  raw_string_ostream os(buffer);
  min_expires->print(os);

  EXPECT_EQ("Min-Expires: 5", os.str());
}

TEST(HeaderTest, Organization) {
  scoped_ptr<Organization> organization(new Organization("Boxes by Bob"));

  EXPECT_EQ("Boxes by Bob", organization->value());

  std::string buffer;
  raw_string_ostream os(buffer);
  organization->print(os);

  EXPECT_EQ("Organization: Boxes by Bob", os.str());
}

TEST(HeaderTest, Priority) {
  scoped_ptr<Priority> priority(new Priority(Priority::emergency));

  EXPECT_EQ("emergency", priority->value());

  std::string buffer;
  raw_string_ostream os(buffer);
  priority->print(os);

  EXPECT_EQ("Priority: emergency", os.str());
}

TEST(HeaderTest, ProxyAuthenticate) {
  scoped_ptr<ProxyAuthenticate> proxy_authenticate(new ProxyAuthenticate(ProxyAuthenticate::Digest));
  proxy_authenticate->set_realm("atlanta.com");
  proxy_authenticate->set_domain("sip:ss1.carrier.com");
  proxy_authenticate->set_qop("auth");
  proxy_authenticate->set_nonce("f84f1cec41e6cbe5aea9c8e88d359");
  proxy_authenticate->set_opaque("");
  proxy_authenticate->set_stale(false);
  proxy_authenticate->set_algorithm(ProxyAuthenticate::MD5);

  std::string buffer;
  raw_string_ostream os(buffer);
  proxy_authenticate->print(os);

  EXPECT_EQ("Proxy-Authenticate: Digest realm=\"atlanta.com\", domain=\"sip:ss1.carrier.com\", qop=\"auth\", nonce=\"f84f1cec41e6cbe5aea9c8e88d359\", opaque=\"\", stale=false, algorithm=MD5", os.str());
}

TEST(HeaderTest, ProxyAuthorization) {
  scoped_ptr<ProxyAuthorization> proxy_authorization(new ProxyAuthorization(ProxyAuthorization::Digest));
  proxy_authorization->set_username("Alice");
  proxy_authorization->set_realm("atlanta.com");
  proxy_authorization->set_nonce("c60f3082ee1212b402a21831ae");
  proxy_authorization->set_response("245f23415f11432b3434341c022");

  std::string buffer;
  raw_string_ostream os(buffer);
  proxy_authorization->print(os);

  EXPECT_EQ("Proxy-Authorization: Digest username=\"Alice\", realm=\"atlanta.com\", nonce=\"c60f3082ee1212b402a21831ae\", response=\"245f23415f11432b3434341c022\"", os.str());   
}

TEST(HeaderTest, ProxyRequire) {
  scoped_ptr<ProxyRequire> proxy_require(new ProxyRequire);
  proxy_require->push_back("foo");

  std::string buffer;
  raw_string_ostream os(buffer);
  proxy_require->print(os);

  EXPECT_EQ("Proxy-Require: foo", os.str());
}

TEST(HeaderTest, RecordRoute) {
  scoped_ptr<RecordRoute> record_route(new RecordRoute(RouteParam("sip:p2.example.com;lr")));
  record_route->push_back(RouteParam("sip:p1.example.com;lr"));

  std::string buffer;
  raw_string_ostream os(buffer);
  record_route->print(os);

  EXPECT_EQ("Record-Route: <sip:p2.example.com;lr>, <sip:p1.example.com;lr>", os.str());
}

TEST(HeaderTest, ReplyTo) {
  scoped_ptr<ReplyTo> reply_to(new ReplyTo("sip:bob@biloxi.com","Bob"));

  std::string buffer;
  raw_string_ostream os(buffer);
  reply_to->print(os);

  EXPECT_EQ("Reply-To: \"Bob\" <sip:bob@biloxi.com>", os.str());
}

TEST(HeaderTest, Require) {
  scoped_ptr<Require> require(new Require("100rel"));

  std::string buffer;
  raw_string_ostream os(buffer);
  require->print(os);

  EXPECT_EQ("Require: 100rel", os.str());
}

TEST(HeaderTest, RetryAfter) {
  scoped_ptr<RetryAfter> retry_after(new RetryAfter(300));

  std::string buffer;
  raw_string_ostream os(buffer);
  retry_after->print(os);

  EXPECT_EQ("Retry-After: 300", os.str());
}

TEST(HeaderTest, Route) {
  scoped_ptr<Route> route(new Route(RouteParam("sip:alice@atlanta.com")));

  std::string buffer;
  raw_string_ostream os(buffer);
  route->print(os);

  EXPECT_EQ("Route: <sip:alice@atlanta.com>", os.str());
}

TEST(HeaderTest, Subject) {
  scoped_ptr<Subject> subject(new Subject("Need more boxes"));

  std::string buffer;
  raw_string_ostream os(buffer);
  subject->print(os);

  EXPECT_EQ("Subject: Need more boxes", os.str());
}

TEST(HeaderTest, Supported) {
  scoped_ptr<Supported> supported(new Supported("100rel"));

  std::string buffer;
  raw_string_ostream os(buffer);
  supported->print(os);

  EXPECT_EQ("Supported: 100rel", os.str());
}

TEST(HeaderTest, Timestamp) {
  scoped_ptr<Timestamp> timestamp(new Timestamp(100, 2.2345));

  std::string buffer;
  raw_string_ostream os(buffer);
  timestamp->print(os);

  EXPECT_EQ("Timestamp: 100 2.2345", os.str());
}

TEST(HeaderTest, To) {
  scoped_ptr<To> to(new To("sip:operator@cs.columbia.edu","The Operator"));
  to->set_tag("287447");

  std::string buffer;
  raw_string_ostream os(buffer);
  to->print(os);

  EXPECT_EQ("To: \"The Operator\" <sip:operator@cs.columbia.edu>;tag=287447", os.str());
}

TEST(HeaderTest, Unsupported) {
  scoped_ptr<Unsupported> unsupported(new Unsupported("foo"));

  std::string buffer;
  raw_string_ostream os(buffer);
  unsupported->print(os);

  EXPECT_EQ("Unsupported: foo", os.str());
}

TEST(HeaderTest, UserAgent) {
  scoped_ptr<UserAgent> user_agent(new UserAgent("Softphone Beta1.5"));

  std::string buffer;
  raw_string_ostream os(buffer);
  user_agent->print(os);

  EXPECT_EQ("User-Agent: Softphone Beta1.5", os.str());
}

// TODO: Via
// TODO: Warning

TEST(HeaderTest, WwwAuthenticate) {
  scoped_ptr<WwwAuthenticate> www_authenticate(new WwwAuthenticate(WwwAuthenticate::Digest));
  www_authenticate->set_realm("atlanta.com");
  www_authenticate->set_domain("sip:boxesbybob.com");
  www_authenticate->set_qop("auth");
  www_authenticate->set_nonce("f84f1cec41e6cbe5aea9c8e88d359");
  www_authenticate->set_opaque("");
  www_authenticate->set_stale(false);
  www_authenticate->set_algorithm(WwwAuthenticate::MD5);

  std::string buffer;
  raw_string_ostream os(buffer);
  www_authenticate->print(os);

  EXPECT_EQ("WWW-Authenticate: Digest realm=\"atlanta.com\", domain=\"sip:boxesbybob.com\", qop=\"auth\", nonce=\"f84f1cec41e6cbe5aea9c8e88d359\", opaque=\"\", stale=false, algorithm=MD5", os.str());
}