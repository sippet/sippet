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
#include "sippet/message/request.h"
#include "sippet/message/response.h"

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
#include "sippet/message/headers/generic.h"

#include "testing/gtest/include/gtest/gtest.h"

using namespace sippet;

TEST(SimpleMessages, Message1) {
  const char message_string[] =
    "REGISTER sip:registrar.biloxi.com SIP/2.0\r\n"
    "Via: SIP/2.0/UDP bobspc.biloxi.com:5060;branch=z9hG4bKnashds7\r\n"
    "Max-Forwards: 70\r\n"
    "To: Bob <sip:bob@biloxi.com>\r\n"
    "From: Bob <sip:bob@biloxi.com>;tag=456248\r\n"
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
  // TODO:
  //EXPECT_EQ("UDP", via->front().protocol());
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

  // TODO:
  //CallId *callid = request->get<CallId>();
  //EXPECT_EQ("843817637684230@998sdasdh09", callid->value());

  Cseq *cseq = request->get<Cseq>();
  EXPECT_EQ(1826, cseq->sequence());
  EXPECT_EQ(Method::REGISTER, cseq->method());

  // TODO:
  //Contact *contact = request->get<Contact>();
  //ASSERT_FALSE(!contact->empty());
  //EXPECT_EQ(GURL("sip:bob@192.0.2.4"), contact->front().address());

  Expires *expires = request->get<Expires>();
  EXPECT_FALSE(0 == expires);
  EXPECT_EQ(7200, expires->value());

  ContentLength *clen = request->get<ContentLength>();
  EXPECT_FALSE(0 == clen);
  EXPECT_EQ(0, clen->value());
}

