// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/transport_test_util.h"

namespace sippet {

class NetworkLayerTest : public testing::Test {
 public:
  void Finish() {
    MessageLoop::current()->RunUntilIdle();
    EXPECT_TRUE(data_->at_read_eof());
    EXPECT_TRUE(data_->at_write_eof());
  }

  void Initialize(net::MockRead* reads = NULL, size_t reads_count = 0,
                  net::MockWrite* writes = NULL, size_t writes_count = 0) {
    network_layer_ = new NetworkLayer(&delegate_, &transaction_factory_);
    data_.reset(new net::DeterministicSocketData(reads, reads_count,
                                                 writes, writes_count));
    data_->set_connect_data(net::MockConnect(net::SYNCHRONOUS, 0));
    if (writes_count) {
      data_->StopAfter(writes_count);
    }
    socket_factory_.reset(new net::MockClientSocketFactory);
    channel_factory_.reset(new MockChannelFactory(socket_factory_.get()));
  }

  scoped_ptr<net::DeterministicSocketData> data_;
  scoped_ptr<net::MockClientSocketFactory> socket_factory_;
  scoped_ptr<MockChannelFactory> channel_factory_;
  StaticNetworkLayerDelegate delegate_;
  MockTransactionFactory transaction_factory_;
  scoped_refptr<NetworkLayer> network_layer_;
};

TEST_F(NetworkLayerTest, StaticFunctions) {
  Initialize();

  std::string branch = NetworkLayer::CreateBranch();
  EXPECT_TRUE(StartsWithASCII(branch, NetworkLayer::kMagicCookie, true));

  scoped_refptr<Channel> channel;
  channel_factory_->CreateChannel(EndPoint("192.0.2.34", 321, Protocol::UDP),
                                  network_layer_,
                                  &channel);

  scoped_refptr<Request> client_request =
    new Request(Method::INVITE, GURL("sip:foo@bar.com"));
  NetworkLayer::StampClientTopmostVia(client_request, channel);
  EXPECT_TRUE(StartsWithASCII(client_request->ToString(),
    "INVITE sip:foo@bar.com SIP/2.0\r\n"
    "v: SIP/2.0/UDP 192.0.2.33:123;rport;branch=z9", true));

  scoped_refptr<Request> empty_via_request =
    new Request(Method::INVITE, GURL("sip:bar@foo.com"));
  NetworkLayer::StampServerTopmostVia(empty_via_request, channel);
  EXPECT_TRUE(StartsWithASCII(empty_via_request->ToString(),
    "INVITE sip:bar@foo.com SIP/2.0\r\n"
    "v: SIP/2.0/UDP 192.0.2.34:321;rport", true));

  scoped_refptr<Request> single_via_request =
    new Request(Method::INVITE, GURL("sip:foobar@foo.com"));
  scoped_ptr<Via> via(new Via);
  net::HostPortPair hostport("192.168.0.1", 7001);
  via->push_back(ViaParam(Protocol::UDP, hostport));
  single_via_request->push_front(via.PassAs<Header>());
  NetworkLayer::StampServerTopmostVia(single_via_request, channel);
  EXPECT_TRUE(StartsWithASCII(single_via_request->ToString(),
    "INVITE sip:foobar@foo.com SIP/2.0\r\n"
    "v: SIP/2.0/UDP 192.168.0.1:7001;received=192.0.2.34;rport=321", true));

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
  via->push_back(ViaParam(Protocol::UDP, net::HostPortPair("192.168.0.1", 7001)));
  single_via_response->push_front(via.PassAs<Header>());
  EndPoint single_via_response_endpoint =
    NetworkLayer::GetMessageEndPoint(single_via_response);
  EXPECT_EQ(EndPoint("192.168.0.1",7001,Protocol::UDP),
    single_via_response_endpoint);

  single_via_response = new Response(200);
  via.reset(new Via);
  via->push_back(ViaParam(Protocol::UDP, net::HostPortPair("192.168.0.1", 7001)));
  via->front().set_received("189.187.200.23");
  single_via_response->push_front(via.PassAs<Header>());
  single_via_response_endpoint =
    NetworkLayer::GetMessageEndPoint(single_via_response);
  EXPECT_EQ(EndPoint("189.187.200.23",7001,Protocol::UDP),
    single_via_response_endpoint);

  single_via_response = new Response(200);
  via.reset(new Via);
  via->push_back(ViaParam(Protocol::UDP, net::HostPortPair("192.168.0.1", 7001)));
  via->front().set_received("189.187.200.23");
  via->front().set_rport(5002);
  single_via_response->push_front(via.PassAs<Header>());
  single_via_response_endpoint =
    NetworkLayer::GetMessageEndPoint(single_via_response);
  EXPECT_EQ(EndPoint("189.187.200.23",5002,Protocol::UDP),
    single_via_response_endpoint);

  Finish();
}

} // End of sippet namespace