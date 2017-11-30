// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/chrome/chrome_datagram_writer.h"

#include <string>

#include "base/run_loop.h"
#include "net/socket/socket_test_util.h"
#include "sippet/message/message.h"

#include "testing/gtest/include/gtest/gtest.h"

using sippet::Request;
using sippet::Via;
using sippet::ViaParam;
using sippet::MaxForwards;
using sippet::To;
using sippet::From;
using sippet::CallId;
using sippet::Cseq;
using sippet::Contact;
using sippet::Expires;
using sippet::ContentLength;
using sippet::ChromeDatagramWriter;
using sippet::Method;
using sippet::Protocol;

class DatagramChannelTest : public testing::Test {
 public:
  void Finish() {
    base::RunLoop().RunUntilIdle();
    EXPECT_TRUE(data_->AllReadDataConsumed());
    EXPECT_TRUE(data_->AllWriteDataConsumed());
  }

  void Initialize(net::MockWrite* writes, size_t writes_count) {
    data_.reset(
      new net::SequencedSocketData(nullptr, 0, writes, writes_count));
    data_->set_connect_data(net::MockConnect(net::SYNCHRONOUS, 0));
    wrapped_socket_ = new net::MockTCPClientSocket(
        address_list_, net_log_.net_log(), data_.get());
    writer_.reset(new ChromeDatagramWriter(wrapped_socket_));
    wrapped_socket_->Connect(callback_.callback());
  }

  static scoped_refptr<Request> CreateRegisterRequest() {
    scoped_refptr<Request> request(
      new Request(Method::REGISTER, GURL("sip:registrar.biloxi.com")));
    std::unique_ptr<Via> via(new Via);
    via->push_back(ViaParam(Protocol::UDP,
        net::HostPortPair("bobspc.biloxi.com", 5060)));
    via->back().set_branch("z9hG4bKnashds7");
    request->push_back(std::move(via));
    std::unique_ptr<MaxForwards> maxfw(new MaxForwards(70));
    request->push_back(std::move(maxfw));
    std::unique_ptr<To> to(new To(GURL("sip:bob@biloxi.com"), "Bob"));
    request->push_back(std::move(to));
    std::unique_ptr<From> from(new From(GURL("sip:bob@biloxi.com"), "Bob"));
    from->set_tag("456248");
    request->push_back(std::move(from));
    std::unique_ptr<CallId> callid(new CallId("843817637684230@998sdasdh09"));
    request->push_back(std::move(callid));
    std::unique_ptr<Cseq> cseq(new Cseq(1826, Method::REGISTER));
    request->push_back(std::move(cseq));
    std::unique_ptr<Contact> contact(new Contact(GURL("sip:bob@192.0.2.4")));
    request->push_back(std::move(contact));
    std::unique_ptr<Expires> expires(new Expires(7200));
    request->push_back(std::move(expires));
    std::unique_ptr<ContentLength> content_length(new ContentLength(0));
    request->push_back(std::move(content_length));
    return request;
  }

  int WriteMessage(const net::CompletionCallback &callback) {
    scoped_refptr<Request> request(CreateRegisterRequest());

    std::string data(request->ToString());
    scoped_refptr<net::IOBuffer> buf(new net::IOBuffer(data.size()));
    memcpy(buf->data(), data.data(), data.size());

    return writer_->Write(buf.get(), data.size(), callback);
  }

  net::AddressList address_list_;
  net::MockTCPClientSocket* wrapped_socket_;
  std::unique_ptr<net::SequencedSocketData> data_;
  std::unique_ptr<ChromeDatagramWriter> writer_;
  const net::NetLogWithSource net_log_;
  net::TestCompletionCallback callback_;
};

static char RegisterRequest[] =
  "REGISTER sip:registrar.biloxi.com SIP/2.0\r\n"
  "v: SIP/2.0/UDP bobspc.biloxi.com:5060;rport;branch=z9hG4bKnashds7\r\n"
  "Max-Forwards: 70\r\n"
  "t: \"Bob\" <sip:bob@biloxi.com>\r\n"
  "f: \"Bob\" <sip:bob@biloxi.com>;tag=456248\r\n"
  "i: 843817637684230@998sdasdh09\r\n"
  "CSeq: 1826 REGISTER\r\n"
  "m: <sip:bob@192.0.2.4>\r\n"
  "Expires: 7200\r\n"
  "l: 0\r\n"
  "\r\n";

TEST_F(DatagramChannelTest, SyncSend) {
  // Covering the normal case when the message is sent synchronously.
  net::MockWrite writes[] = {
    net::MockWrite(net::SYNCHRONOUS, 0, RegisterRequest),
  };

  Initialize(writes, arraysize(writes));

  int rv = WriteMessage(callback_.callback());
  ASSERT_EQ(net::OK, rv);

  Finish();
}

TEST_F(DatagramChannelTest, OverlappedSend) {
  // This test covers the case where two requests are enqueued and sent
  // asynchrously. Messages are always truncated.
  net::MockWrite writes[] = {
    net::MockWrite(net::ASYNC, 0,
      "REGISTER sip:registrar.biloxi.com SIP/2.0\r\n"
      "v: SIP/2.0/UDP bobspc.biloxi.com:5060;rport;branch=z9hG4bKnashds7\r\n"
      "Max-Forwards: 70\r\n"),
    net::MockWrite(net::ASYNC, 1,
      "REGISTER sip:registrar.biloxi.com SIP/2.0\r\n"
      "v: SIP/2.0/UDP bobspc.biloxi.com:5060;rport;branch=z9hG4bKnashds7\r\n"
      "Max-Forwards: 70\r\n"),
  };

  Initialize(writes, arraysize(writes));

  int rv = WriteMessage(callback_.callback());
  ASSERT_EQ(net::ERR_IO_PENDING, rv);

  // Send the same message again, should be enqueued.
  rv = WriteMessage(callback_.callback());
  ASSERT_EQ(net::ERR_IO_PENDING, rv);

  // First message is sent.
  rv = callback_.WaitForResult();
  ASSERT_EQ(net::OK, rv);

  // Second message is sent.
  rv = callback_.WaitForResult();
  ASSERT_EQ(net::OK, rv);

  Finish();
}

TEST_F(DatagramChannelTest, SyncSendError) {
  // Synchronous error while sending data.
  net::MockWrite writes[] = {
    net::MockWrite(net::SYNCHRONOUS, net::ERR_CONNECTION_ABORTED),
  };

  Initialize(writes, arraysize(writes));

  int rv = WriteMessage(callback_.callback());
  ASSERT_EQ(net::ERR_CONNECTION_ABORTED, rv);

  Finish();
}

TEST_F(DatagramChannelTest, AsyncSendError) {
  // This test simulates the case when an error occurs while sending
  // data, asynchronously.
  net::MockWrite writes[] = {
    net::MockWrite(net::ASYNC, 0,
       "REGISTER sip:registrar.biloxi.com SIP/2.0\r\n"
       "v: SIP/2.0/UDP bobspc.biloxi.com:5060;rport;branch=z9hG4bKnashds7\r\n"
       "Max-Forwards: 70\r\n"),
    net::MockWrite(net::ASYNC, net::ERR_CONNECTION_CLOSED, 1),
  };

  Initialize(writes, arraysize(writes));

  int rv = WriteMessage(callback_.callback());
  ASSERT_EQ(net::ERR_IO_PENDING, rv);

  // Write a second message, this one will be reseted
  rv = WriteMessage(callback_.callback());
  ASSERT_EQ(net::ERR_IO_PENDING, rv);

  // The connection will be reset while sending the message.
  rv = callback_.WaitForResult();
  ASSERT_EQ(net::ERR_CONNECTION_CLOSED, rv);

  Finish();
}

TEST_F(DatagramChannelTest, AsyncConnReset) {
  // This test simulates the case when the connection is reset by the peer
  // and the channel tries to send more data. Considering zero bytes sent
  // as a connection reset.
  net::MockWrite writes[] = {
    net::MockWrite(net::ASYNC, 0,
       "REGISTER sip:registrar.biloxi.com SIP/2.0\r\n"
       "v: SIP/2.0/UDP bobspc.biloxi.com:5060;rport;branch=z9hG4bKnashds7\r\n"
       "Max-Forwards: 70\r\n"),
    net::MockWrite(net::ASYNC, 1, ""),
  };

  Initialize(writes, arraysize(writes));

  int rv = WriteMessage(callback_.callback());
  ASSERT_EQ(net::ERR_IO_PENDING, rv);

  // Write a second message, as the first message will be truncated
  rv = WriteMessage(callback_.callback());
  ASSERT_EQ(net::ERR_IO_PENDING, rv);

  // There will be a result = 0 while sending data.
  rv = callback_.WaitForResult();
  ASSERT_EQ(net::ERR_CONNECTION_RESET, rv);

  Finish();
}
