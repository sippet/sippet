// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/channel_base.h"

#include "sippet/message/message.h"
#include "net/socket/socket_test_util.h"

#include "testing/gtest/include/gtest/gtest.h"

using namespace sippet;

scoped_refptr<Request> CreateTestRequest() {
  scoped_refptr<Request> request(
    new Request(Method::REGISTER, GURL("sip:registrar.biloxi.com")));

  scoped_ptr<Via> via(new Via);
  via->push_back(
    ViaParam(Protocol::UDP, net::HostPortPair("bobspc.biloxi.com",5060)));
  via->back().set_branch("z9hG4bKnashds7");
  request->push_back(via.PassAs<Header>());

  scoped_ptr<MaxForwards> maxfw(new MaxForwards(70));
  request->push_back(maxfw.PassAs<Header>());

  scoped_ptr<To> to(new To(GURL("sip:bob@biloxi.com"), "Bob"));
  request->push_back(to.PassAs<Header>());

  scoped_ptr<From> from(new From(GURL("sip:bob@biloxi.com"), "Bob"));
  from->set_tag("456248");
  request->push_back(from.PassAs<Header>());

  scoped_ptr<CallId> callid(new CallId("843817637684230@998sdasdh09"));
  request->push_back(callid.PassAs<Header>());

  scoped_ptr<Cseq> cseq(new Cseq(1826, Method::REGISTER));
  request->push_back(cseq.PassAs<Header>());

  scoped_ptr<Contact> contact(new Contact(GURL("sip:bob@192.0.2.4")));
  request->push_back(contact.PassAs<Header>());

  scoped_ptr<Expires> expires(new Expires(7200));
  request->push_back(expires.PassAs<Header>());

  scoped_ptr<ContentLength> content_length(new ContentLength(0));
  request->push_back(content_length.PassAs<Header>());

  return request;
}

TEST(ChannelBase, AsyncSend) {
  // The message is written as a single chunk.
  net::MockWrite writes[] = {
    net::MockWrite(net::ASYNC, 0,
       "REGISTER sip:registrar.biloxi.com SIP/2.0\r\n"
       "Via: SIP/2.0/UDP bobspc.biloxi.com:5060;branch=z9hG4bKnashds7\r\n"
       "Max-Forwards: 70\r\n"
       "To: \"Bob\" <sip:bob@biloxi.com>\r\n"
       "From: \"Bob\" <sip:bob@biloxi.com>;tag=456248\r\n"
       "Call-ID: 843817637684230@998sdasdh09\r\n"
       "CSeq: 1826 REGISTER\r\n"
       "Contact: <sip:bob@192.0.2.4>\r\n"
       "Expires: 7200\r\n"
       "Content-Length: 0\r\n"
       "\r\n"),
  };

  scoped_refptr<Request> request(CreateTestRequest());

  net::DeterministicSocketData data(NULL, 0, writes, ARRAYSIZE_UNSAFE(writes));
  data.set_connect_data(net::MockConnect(net::SYNCHRONOUS, net::OK));

  scoped_ptr<net::DeterministicMockTCPClientSocket> transport(
      new net::DeterministicMockTCPClientSocket(NULL, &data));
  data.set_socket(transport->AsWeakPtr());

  net::TestCompletionCallback callback;
  int rv = transport->Connect(callback.callback());
  rv = callback.GetResult(rv);
  ASSERT_EQ(net::OK, rv);

  scoped_refptr<ChannelBase> channel_base(
      new ChannelBase(transport->AsWeakPtr()));
  channel_base->Send(*request.get(), callback.callback());
 

}
