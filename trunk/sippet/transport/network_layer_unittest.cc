// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/transport_test_util.h"

namespace sippet {

class NetworkLayerTest : public testing::Test {
 public:
  void Finish() {
    MessageLoop::current()->RunUntilIdle();
    EXPECT_TRUE(data_provider_->at_events_end());
    EXPECT_TRUE(data_->at_write_eof());
    EXPECT_TRUE(data_->at_read_eof());
  }

  void Initialize(net::MockRead* reads = NULL, size_t reads_count = 0,
                  net::MockWrite* writes = NULL, size_t writes_count = 0,
                  MockEvent* events = NULL, size_t events_count = 0,
                  const char *branches[] = NULL, size_t branches_count = 0) {
    NetworkSettings settings;
    if (branches != NULL) {
      branch_factory_.reset(new MockBranchFactory(branches, branches_count));
      settings.set_branch_factory(branch_factory_.get());
    }
    data_provider_.reset(new StaticDataProvider(events, events_count));
    transaction_factory_.reset(new MockTransactionFactory(data_provider_.get()));
    settings.set_transaction_factory(transaction_factory_.get());
    delegate_.reset(new StaticNetworkLayerDelegate(data_provider_.get()));
    network_layer_ = new NetworkLayer(delegate_.get(), settings);
    data_.reset(new net::DeterministicSocketData(reads, reads_count,
                                                 writes, writes_count));
    data_->set_connect_data(net::MockConnect(net::ASYNC, 0));
    socket_factory_.reset(new net::DeterministicMockClientSocketFactory);
    socket_factory_->AddSocketDataProvider(data_.get());
    channel_factory_.reset(new MockChannelFactory(socket_factory_.get()));
    network_layer_->RegisterChannelFactory(Protocol::TCP, channel_factory_.get());
  }

  static scoped_refptr<Request> CreateRegisterRequest() {
    scoped_refptr<Request> request(
      new Request(Method::REGISTER, GURL("sip:192.0.4.42;transport=TCP")));
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

  scoped_ptr<DataProvider> data_provider_;
  scoped_ptr<net::DeterministicSocketData> data_;
  scoped_ptr<net::DeterministicMockClientSocketFactory> socket_factory_;
  scoped_ptr<MockChannelFactory> channel_factory_;
  scoped_ptr<NetworkLayer::Delegate> delegate_;
  scoped_ptr<MockTransactionFactory> transaction_factory_;
  scoped_refptr<NetworkLayer> network_layer_;
  scoped_ptr<MockBranchFactory> branch_factory_;
  net::TestCompletionCallback callback_;
};

TEST_F(NetworkLayerTest, StaticFunctions) {
  Initialize();

  std::string branch = network_layer_->CreateBranch();
  EXPECT_TRUE(StartsWithASCII(branch, NetworkLayer::kMagicCookie, true));

  scoped_refptr<Channel> channel;
  channel_factory_->CreateChannel(EndPoint("192.0.2.34", 321, Protocol::TCP),
                                  network_layer_,
                                  &channel);

  scoped_refptr<Request> client_request =
    new Request(Method::INVITE, GURL("sip:foo@bar.com"));
  network_layer_->StampClientTopmostVia(client_request, channel);
  EXPECT_TRUE(StartsWithASCII(client_request->ToString(),
    "INVITE sip:foo@bar.com SIP/2.0\r\n"
    "v: SIP/2.0/TCP 192.0.2.33:123;rport;branch=z9", true));

  scoped_refptr<Request> empty_via_request =
    new Request(Method::INVITE, GURL("sip:bar@foo.com"));
  network_layer_->StampServerTopmostVia(empty_via_request, channel);
  EXPECT_TRUE(StartsWithASCII(empty_via_request->ToString(),
    "INVITE sip:bar@foo.com SIP/2.0\r\n"
    "v: SIP/2.0/TCP 192.0.2.34:321;rport", true));

  scoped_refptr<Request> single_via_request =
    new Request(Method::INVITE, GURL("sip:foobar@foo.com"));
  scoped_ptr<Via> via(new Via);
  net::HostPortPair hostport("192.168.0.1", 7001);
  via->push_back(ViaParam(Protocol::TCP, hostport));
  single_via_request->push_front(via.PassAs<Header>());
  network_layer_->StampServerTopmostVia(single_via_request, channel);
  EXPECT_TRUE(StartsWithASCII(single_via_request->ToString(),
    "INVITE sip:foobar@foo.com SIP/2.0\r\n"
    "v: SIP/2.0/TCP 192.168.0.1:7001;received=192.0.2.34;rport=321", true));

  EndPoint single_via_request_endpoint =
    NetworkLayer::GetMessageEndPoint(single_via_request);
  EXPECT_EQ(EndPoint("foo.com",5060,Protocol::UDP),
    single_via_request_endpoint);

  single_via_request->set_request_uri(GURL("sip:foobar@foo.com;transport=TCP"));
  single_via_request_endpoint =
    NetworkLayer::GetMessageEndPoint(single_via_request);
  EXPECT_EQ(EndPoint("foo.com",5060,Protocol::TCP),
    single_via_request_endpoint);

  scoped_refptr<Response> single_via_response = new Response(200);
  via.reset(new Via);
  via->push_back(ViaParam(Protocol::TCP, net::HostPortPair("192.168.0.1", 7001)));
  single_via_response->push_front(via.PassAs<Header>());
  EndPoint single_via_response_endpoint =
    NetworkLayer::GetMessageEndPoint(single_via_response);
  EXPECT_EQ(EndPoint("192.168.0.1",7001,Protocol::TCP),
    single_via_response_endpoint);

  single_via_response = new Response(200);
  via.reset(new Via);
  via->push_back(ViaParam(Protocol::TCP, net::HostPortPair("192.168.0.1", 7001)));
  via->front().set_received("189.187.200.23");
  single_via_response->push_front(via.PassAs<Header>());
  single_via_response_endpoint =
    NetworkLayer::GetMessageEndPoint(single_via_response);
  EXPECT_EQ(EndPoint("189.187.200.23",7001,Protocol::TCP),
    single_via_response_endpoint);

  single_via_response = new Response(200);
  via.reset(new Via);
  via->push_back(ViaParam(Protocol::TCP, net::HostPortPair("192.168.0.1", 7001)));
  via->front().set_received("189.187.200.23");
  via->front().set_rport(5002);
  single_via_response->push_front(via.PassAs<Header>());
  single_via_response_endpoint =
    NetworkLayer::GetMessageEndPoint(single_via_response);
  EXPECT_EQ(EndPoint("189.187.200.23",5002,Protocol::TCP),
    single_via_response_endpoint);

  Finish();
}

TEST_F(NetworkLayerTest, OutgoingRequest) {
  const char *branches[] = {
    "z9hG4bKnashds7"
  };

  net::MockRead expected_reads[] = {
    net::MockRead(net::ASYNC, 1,
      "SIP/2.0 200 OK\r\n"
      "v: SIP/2.0/TCP 192.0.2.33:123;rport=123;branch=z9hG4bKnashds7\r\n"
      "Max-Forwards: 70\r\n"
      "t: \"Bob\" <sip:bob@biloxi.com>\r\n"
      "f: \"Bob\" <sip:bob@biloxi.com>;tag=456248\r\n"
      "i: 843817637684230@998sdasdh09\r\n"
      "CSeq: 1826 REGISTER\r\n"
      "m: <sip:bob@192.0.2.4>\r\n"
      "Expires: 7200\r\n"
      "l: 0\r\n"
      "\r\n"),
    net::MockRead(net::ASYNC, 2, "\r\n")
  };

  net::MockWrite expected_writes[] = {
    net::MockWrite(net::SYNCHRONOUS, 0,
      "REGISTER sip:192.0.4.42;transport=TCP SIP/2.0\r\n"
      "v: SIP/2.0/TCP 192.0.2.33:123;rport;branch=z9hG4bKnashds7\r\n"
      "Max-Forwards: 70\r\n"
      "t: \"Bob\" <sip:bob@biloxi.com>\r\n"
      "f: \"Bob\" <sip:bob@biloxi.com>;tag=456248\r\n"
      "i: 843817637684230@998sdasdh09\r\n"
      "CSeq: 1826 REGISTER\r\n"
      "m: <sip:bob@192.0.2.4>\r\n"
      "Expires: 7200\r\n"
      "l: 0\r\n"
      "\r\n"),
  };

  std::string tid;
  MockEvent expected_events[] = {
    ExpectStartTransaction("^REGISTER sip:192.0.4.42.*", &tid),
    ExpectIncomingResponse("^SIP/2.0 200 OK.*", &tid),
    ExpectIncomingMessage("^SIP/2.0 200 OK.*"),
    ExpectTransactionClose(&tid),
  };

  Initialize(expected_reads, ARRAYSIZE_UNSAFE(expected_reads),
             expected_writes, ARRAYSIZE_UNSAFE(expected_writes),
             expected_events, ARRAYSIZE_UNSAFE(expected_events),
             branches, ARRAYSIZE_UNSAFE(branches));

  int rv = network_layer_->Send(CreateRegisterRequest(),
    callback_.callback());
  EXPECT_EQ(net::ERR_IO_PENDING, rv);

  data_->RunFor(1);
  
  rv = callback_.WaitForResult();
  EXPECT_EQ(net::OK, rv);

  data_->RunFor(1);

  // The mock transaction doesn't terminate automatically
  // when the response arrives, so we have to close it
  // explicitly.
  MockClientTransaction *client_transaction =
    transaction_factory_->client_transaction(0);
  client_transaction->Terminate();

  Finish();
}

} // End of sippet namespace
