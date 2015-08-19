// Copyright (c) 2013-2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include "sippet/message/message.h"
#include "sippet/uri/uri.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace sippet {

namespace {

struct name_is : public std::unary_function<const Header&, bool> {
  std::string name_;
  name_is(const char *name)
    : name_(name) {
  }
  bool operator() (const Header& h) {
    return name_ == h.name();
  }
};

void ExpectSipURIHaving(const GURL& url,
                        const char *uri,
                        const char *username,
                        const char *host) {
  ASSERT_TRUE(url.SchemeIs("sip"));
  EXPECT_EQ(GURL(uri), url);
  SipURI sip_uri(url);
  EXPECT_EQ(username, sip_uri.username());
  EXPECT_EQ(host, sip_uri.host());
}


} // empty namespace

TEST(SimpleMessages, Message1) {
  const char message_string[] =
    "REGISTER sip:registrar.biloxi.com SIP/2.0\r\n"
    "v: SIP/2.0/UDP bobspc.biloxi.com:5060;branch=z9hG4bKnashds7\r\n"
    "Max-Forwards: 70\r\n"
    "t: Bob <sip:bob@biloxi.com>\r\n"
    "f: Bob <sip:bob@biloxi.com>;tag=456248\r\n"
    "Call-ID: 843817637684230@998sdasdh09\r\n"
    "CSeq: 1826 REGISTER\r\n"
    "Contact: <sip:bob@192.0.2.4>\r\n"
    "Expires: 7200\r\n"
    "Content-Length: 0\r\n"
    "\r\n";

  scoped_refptr<Message> message = Message::Parse(message_string);
  ASSERT_TRUE(isa<Request>(message));

  scoped_refptr<Request> request = dyn_cast<Request>(message);
  EXPECT_EQ(Method::REGISTER, request->method());
  EXPECT_EQ(GURL("sip:registrar.biloxi.com"), request->request_uri());
  EXPECT_EQ(Version(2,0), request->version());

  Message::iterator via_it = request->find_first<Via>();
  EXPECT_NE(request->end(), via_it);
  Via *via = dyn_cast<Via>(via_it);
  EXPECT_FALSE(via->empty());
  EXPECT_EQ(Protocol::UDP, via->front().protocol());
  EXPECT_TRUE(net::HostPortPair::FromString("bobspc.biloxi.com:5060")
    .Equals(via->front().sent_by()));
  EXPECT_EQ("z9hG4bKnashds7", via->front().branch());

  Message::iterator maxfw_it = request->find_first<MaxForwards>();
  EXPECT_NE(request->end(), maxfw_it);
  MaxForwards *maxfw = dyn_cast<MaxForwards>(maxfw_it);
  EXPECT_EQ(70, maxfw->value());

  To *to = request->get<To>();
  EXPECT_EQ("Bob", to->display_name());
  EXPECT_EQ(GURL("sip:bob@biloxi.com"), to->address());

  From *from = request->get<From>();
  EXPECT_EQ("Bob", from->display_name());
  EXPECT_EQ(GURL("sip:bob@biloxi.com"), from->address());
  EXPECT_EQ("456248", from->tag());

  CallId *callid = request->get<CallId>();
  EXPECT_EQ("843817637684230@998sdasdh09", callid->value());

  Cseq *cseq = request->get<Cseq>();
  EXPECT_EQ(1826, cseq->sequence());
  EXPECT_EQ(Method::REGISTER, cseq->method());

  Contact *contact = request->get<Contact>();
  ASSERT_FALSE(contact->empty());
  EXPECT_EQ(GURL("sip:bob@192.0.2.4"), contact->front().address());

  Expires *expires = request->get<Expires>();
  EXPECT_FALSE(0 == expires);
  EXPECT_EQ(7200, expires->value());

  ContentLength *clen = request->get<ContentLength>();
  EXPECT_FALSE(0 == clen);
  EXPECT_EQ(0, clen->value());
}

TEST(SimpleMessages, TortureMessage1) {
  const char message_string[] =
    "INVITE sip:vivekg@chair-dnrc.example.com;unknownparam SIP/2.0\r\n"
    "TO :\r\n"
    "  sip:vivekg@chair-dnrc.example.com ;   tag    = 1918181833n\r\n"
    "from   : \"J Rosenberg \\\\\\\"\"       <sip:jdrosen@example.com>\r\n"
    "  ;\r\n"
    "  tag = 98asjd8\r\n"
    "MaX-fOrWaRdS: 0068\r\n"
    "Call-ID: wsinv.ndaksdj@192.0.2.1\r\n"
    "Content-Length   : 150\r\n"
    "cseq: 0009\r\n"
    "  INVITE\r\n"
    "Via  : SIP  /   2.0\r\n"
    "  /UDP\r\n"
    "    192.0.2.2;branch=390skdjuw\r\n"
    "s :\r\n"
    "NewFangledHeader:   newfangled value\r\n"
    "  continued newfangled value\r\n"
    "UnknownHeaderWithUnusualValue: ;;,,;;,;\r\n"
    "Content-Type: application/sdp\r\n"
    "Route:\r\n"
    "  <sip:services.example.com;lr;unknownwith=value;unknown-no-value>\r\n"
    "v:  SIP  / 2.0  / TCP     spindle.example.com   ;\r\n"
    "  branch  =   z9hG4bK9ikj8  ,\r\n"
    "  SIP  /    2.0   / UDP  192.168.255.111   ; branch=\r\n"
    "  z9hG4bK30239\r\n"
    "m:\"Quoted string \\\"\\\"\" <sip:jdrosen@example.com> ; newparam =\r\n"
    "      newvalue ;\r\n"
    "  secondparam ; q = 0.33\r\n"
    "\r\n";

  scoped_refptr<Message> message = Message::Parse(message_string);
  ASSERT_TRUE(isa<Request>(message));

  scoped_refptr<Request> request = dyn_cast<Request>(message);
  EXPECT_EQ(Method::INVITE, request->method());
  EXPECT_EQ(GURL("sip:vivekg@chair-dnrc.example.com;unknownparam"), request->request_uri());
  EXPECT_EQ(Version(2,0), request->version());

  To *to = request->get<To>();
  EXPECT_EQ("", to->display_name());
  EXPECT_EQ(GURL("sip:vivekg@chair-dnrc.example.com"), to->address());
  EXPECT_EQ("1918181833n", to->tag());

  From *from = request->get<From>();
  EXPECT_EQ("J Rosenberg \\\"", from->display_name());
  EXPECT_EQ(GURL("sip:jdrosen@example.com"), from->address());
  EXPECT_EQ("98asjd8", from->tag());

  MaxForwards *maxfw = request->get<MaxForwards>();
  EXPECT_EQ(68, maxfw->value());

  ContentLength *clen = request->get<ContentLength>();
  EXPECT_EQ(150, clen->value());

  Cseq *cseq = request->get<Cseq>();
  EXPECT_EQ(9, cseq->sequence());
  EXPECT_EQ(Method::INVITE, cseq->method());

  Via *via = request->get<Via>();
  EXPECT_EQ(Version(2,0), via->front().version());
  EXPECT_EQ(Protocol::UDP, via->front().protocol());
  EXPECT_EQ("192.0.2.2", via->front().sent_by().host());
  EXPECT_EQ(5060, via->front().sent_by().port());
  EXPECT_EQ("390skdjuw", via->front().branch());

  Subject *subject = request->get<Subject>();
  EXPECT_EQ("", subject->value());
}

TEST(SimpleMessages, TortureMessage2) {
  const char message_string[] =
    "!interesting-Method0123456789_*+`.%indeed'~ "
      "sip:1_unusual.URI~(to-be!sure)"
      "&isn't+it$/crazy?,/;;*:"
      "&it+has=1,weird!*pas$wo~d_too.(doesn't-it)"
      "@example.com SIP/2.0\r\n"
    "Via: SIP/2.0/TCP host1.example.com;branch=z9hG4bK-.!%66*_+`'~\r\n"
    "To: \"BEL:\\\x07 NUL:\\\x00 DEL:\\\x7f\" "
      "<sip:1_unusual.URI~(to-be!sure)&isn't+it$/crazy?,/;;*@example.com>\r\n"
    "From: token1~` token2'+_ token3*%!.- <sip:mundane@example.com>"
      ";fromParam''~+*_!.-%=\"\xD1\x80\xD0\xB0\xD0\xB1\xD0\xBE\xD1\x82\xD0\xB0\xD1\x8E\xD1\x89\xD0\xB8\xD0\xB9\""
      ";tag=_token~1'+`*%!-.\r\n"
    "Call-ID: intmeth.word%ZK-!.*_+'@word`~)(><:\\/\"][?}{\r\n"
    "CSeq: 139122385 !interesting-Method0123456789_*+`.%indeed'~\r\n"
    "Max-Forwards: 255\r\n"
    "extensionHeader-!.%*+_`'~: \xEF\xBB\xBF\xE5\xA4\xA7\xE5\x81\x9C\xE9\x9B\xBB\r\n"
    "Content-Length: 0\r\n"
    "\r\n";

  scoped_refptr<Message> message = Message::Parse(
    std::string(message_string, arraysize(message_string)-1));
  ASSERT_TRUE(isa<Request>(message));

  Request *request = dyn_cast<Request>(message).get();
  EXPECT_TRUE(base::LowerCaseEqualsASCII(request->method().str(),
    "!interesting-method0123456789_*+`.%indeed'~"));
  EXPECT_EQ(GURL("sip:1_unusual.URI~(to-be!sure)"
                   "&isn't+it$/crazy?,/;;*:"
                   "&it+has=1,weird!*pas$wo~d_too.(doesn't-it)"
                   "@example.com"),
            request->request_uri());

  Via *via = request->get<Via>();
  ASSERT_TRUE(via);
  ASSERT_FALSE(via->empty());
  EXPECT_EQ("z9hG4bK-.!%66*_+`'~", via->front().branch());

  To *to = request->get<To>();
  ASSERT_TRUE(to);
  const char to_name[] = "BEL:\x07 NUL:\x00 DEL:\x7f";
  EXPECT_EQ(std::string(to_name, arraysize(to_name)-1),
      to->display_name());
  EXPECT_EQ(GURL("sip:1_unusual.URI~(to-be!sure)"
                 "&isn't+it$/crazy?,/;;*@example.com"),
            to->address());

  From *from = request->get<From>();
  ASSERT_TRUE(from);
  const char from_name[] = "token1~` token2'+_ token3*%!.-";
  EXPECT_EQ(std::string(from_name, arraysize(from_name)-1),
      from->display_name());
  has_parameters::const_param_iterator i =
      from->param_find("fromParam''~+*_!.-%");
  ASSERT_NE(i, from->param_end());
  EXPECT_EQ("\xD1\x80\xD0\xB0\xD0\xB1\xD0\xBE\xD1\x82\xD0\xB0\xD1\x8E\xD1\x89\xD0\xB8\xD0\xB9",
      i->second);
  EXPECT_EQ("_token~1'+`*%!-.", from->tag());

  CallId *call_id = request->get<CallId>();
  ASSERT_TRUE(call_id);
  EXPECT_EQ("intmeth.word%ZK-!.*_+'@word`~)(><:\\/\"][?}{", call_id->value());

  Cseq *cseq = request->get<Cseq>();
  ASSERT_TRUE(cseq);
  EXPECT_TRUE(base::LowerCaseEqualsASCII(cseq->method().str(),
    "!interesting-method0123456789_*+`.%indeed'~"));
  EXPECT_EQ(139122385, cseq->sequence());

  MaxForwards *max_forwards = request->get<MaxForwards>();
  ASSERT_TRUE(max_forwards);
  EXPECT_EQ(255, max_forwards->value());

  Message::iterator j =
      std::find_if(request->begin(), request->end(),
          name_is("extensionHeader-!.%*+_`'~"));
  ASSERT_NE(request->end(), j);
  ASSERT_TRUE(isa<Generic>(j));
  Generic *g = dyn_cast<Generic>(j);
  ASSERT_EQ("\xEF\xBB\xBF\xE5\xA4\xA7\xE5\x81\x9C\xE9\x9B\xBB",
      g->header_value());
}

TEST(SimpleMessages, EscapedUris) {
  const char message_string[] =
    "INVITE sip:sips%3Auser%40example.com@example.net SIP/2.0\r\n"
    "To: sip:%75se%72@example.com\r\n"
    "From: <sip:I%20have%20spaces@example.net>;tag=938\r\n"
    "Contact: <sip:cal%6Cer@host5.example.net;%6C%72;n%61me=v%61lue%25%34%31>\r\n"
    "\r\n";

  scoped_refptr<Message> message = Message::Parse(message_string);
  ASSERT_TRUE(isa<Request>(message));

  Request *request = dyn_cast<Request>(message).get();
  ExpectSipURIHaving(request->request_uri(),
      "sip:sips%3Auser%40example.com@example.net",
      "sips:user@example.com",
      "example.net");

  To *to = request->get<To>();
  ASSERT_TRUE(to);
  ExpectSipURIHaving(to->address(),
      "sip:%75se%72@example.com",
      "user",
      "example.com");

  From *from = request->get<From>();
  ASSERT_TRUE(from);
  ExpectSipURIHaving(from->address(),
      "sip:I%20have%20spaces@example.net",
      "I have spaces",
      "example.net");

  Contact *contact = request->get<Contact>();
  ASSERT_TRUE(contact);
  ASSERT_FALSE(contact->empty());
  ExpectSipURIHaving(contact->front().address(),
      "sip:cal%6Cer@host5.example.net;%6C%72;n%61me=v%61lue%25%34%31",
      "caller",
      "host5.example.net");
}

TEST(SimpleMessages, Bye) {
  // The function IsStatusLine (from Message::Parse) was wrong: it was
  // interpreting the request below as a response. The heuristic
  // considered that the word "SIP" had to start on the first 4 characters
  // of the message.
  const char message_string[] =
    "BYE sip:1.1.1.1:1;ob SIP/2.0\r\n"
    "Via: SIP/2.0/UDP 2.2.2.2:2;branch=z9hG4bK5913.5c3b2a91.0\r\n"
    "From: <sip:12345@1.1.1.1:1>;tag=as1e74befe\r\n"
    "To: <sip:54321@3.3.3.3:3>;tag=NlgWLM60\r\n"
    "Call-ID: FPN7NjRQZmVTJm1E6mII\r\n"
    "CSeq: 1 BYE\r\n"
    "Max-Forwards: 70\r\n"
    "Content-Length: 0\r\n";

  scoped_refptr<Message> message = Message::Parse(message_string);
  ASSERT_TRUE(isa<Request>(message));
}

TEST(Headers, Contact) {
  struct {
    const char *input;
    const char *address;
    const char *display_name;
  } cases[] = {
    { "Contact: <sip:bob@192.0.2.4>", "sip:bob@192.0.2.4", "" },
    { "Contact: sip:bob@192.0.2.4", "sip:bob@192.0.2.4", "" },
    { "Contact: \"\" <sip:bob@192.0.2.4>", "sip:bob@192.0.2.4", "" },
    { "Contact: \"foo\\\"bar\" <sip:bob@192.0.2.4>", "sip:bob@192.0.2.4", "foo\"bar" },
    { "Contact: \"\x4f\x60\x59\x7d\" <sip:bob@192.0.2.4>", "sip:bob@192.0.2.4", "\x4f\x60\x59\x7d" },
    { "Contact: \x4f\x60\x59\x7d <sip:bob@192.0.2.4>", "sip:bob@192.0.2.4", "\x4f\x60\x59\x7d" },
    // No LWS between Display Name
    { "Contact: caller<sip:caller@example.com>;tag=323", "sip:caller@example.com", "caller" },
    // Some torture tests
    { "Contact: < sip:bob@192.0.2.4 >", "sip:bob@192.0.2.4", "" },
  };

  for (size_t i = 0; i < arraysize(cases); ++i) {
    scoped_ptr<Header> header(Header::Parse(cases[i].input));
    ASSERT_TRUE(isa<Contact>(header));
    Contact *contact = dyn_cast<Contact>(header);
    ASSERT_FALSE(contact->empty());
    EXPECT_EQ(GURL(cases[i].address), contact->front().address());
    EXPECT_EQ(cases[i].display_name, contact->front().display_name());
  }
}

TEST(Headers, Via) {
  struct {
    const char *input;
    Version version;
    Protocol::Type protocol;
    const char *host;
    int port;
    const char *hostport;
    const char *branch;
  } cases[] = {
    { "v: SIP/2.0/UDP bobspc.biloxi.com:5060;branch=z9hG4bKnashds7", Version(2,0),
      Protocol::UDP, "bobspc.biloxi.com", 5060, "bobspc.biloxi.com:5060", "z9hG4bKnashds7" },
    { "v: sip/2.0/udp 127.0.0.1    ; branch=z9hG4bKnashds7", Version(2,0),
      Protocol::UDP, "127.0.0.1", 5060, "127.0.0.1:5060", "z9hG4bKnashds7" },
    { "v: SiP/2.0/TLS hostname; branch = \"z9hG4bKnashds7\"", Version(2,0),
      Protocol::TLS, "hostname", 5061, "hostname:5060", "z9hG4bKnashds7" },
    { "v: SIP/2.0/TLS [::1]; branch = \"z9hG4bKnashds7\"", Version(2,0),
      Protocol::TLS, "::1", 5061, "[::1]:5060", "z9hG4bKnashds7" },
    { "v: SIP/3.0/TLS [::1]; branch = \"z9hG4bKnashds7\"", Version(3,0),
      Protocol::TLS, "::1", 5061, "[::1]:5060", "z9hG4bKnashds7" },
  };

  for (size_t i = 0; i < arraysize(cases); ++i) {
    scoped_ptr<Header> header(Header::Parse(cases[i].input));
    ASSERT_TRUE(isa<Via>(header));
    Via *via = dyn_cast<Via>(header);
    ASSERT_FALSE(via->empty());
    EXPECT_EQ(cases[i].version, via->front().version());
    EXPECT_EQ(cases[i].protocol, via->front().protocol());
    EXPECT_EQ(cases[i].host, via->front().sent_by().host());
    EXPECT_EQ(cases[i].port, via->front().sent_by().port());
  }
}

} // namespace sippet
