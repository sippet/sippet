// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/chrome/chrome_stream_writer.h"

#include "sippet/message/message.h"
#include "net/socket/socket_test_util.h"

#include "testing/gtest/include/gtest/gtest.h"

using namespace sippet;

class StreamChannelTest : public testing::Test {
 public:
  void Finish() {
    base::MessageLoop::current()->RunUntilIdle();
    EXPECT_TRUE(data_->at_read_eof());
    EXPECT_TRUE(data_->at_write_eof());
  }

  void Initialize(net::MockWrite* writes, size_t writes_count) {
    data_.reset(
      new net::DeterministicSocketData(NULL, 0, writes, writes_count));
    data_->set_connect_data(net::MockConnect(net::SYNCHRONOUS, 0));
    if (writes_count) {
      data_->StopAfter(writes_count);
    }
    wrapped_socket_ =
        new net::DeterministicMockTCPClientSocket(net_log_.net_log(), data_.get());
    data_->set_socket(wrapped_socket_);
    writer_.reset(new ChromeStreamWriter(wrapped_socket_));
    wrapped_socket_->Connect(callback_.callback());
  }

  static scoped_refptr<Request> CreateRegisterRequest() {
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

  int WriteMessage(const net::CompletionCallback &callback) {
    scoped_refptr<Request> request(CreateRegisterRequest());

    std::string data(request->ToString());
    scoped_refptr<net::IOBuffer> buf(new net::IOBuffer(data.size()));
    memcpy(buf->data(), data.data(), data.size());

    return writer_->Write(buf, data.size(), callback);
  }

  net::DeterministicMockTCPClientSocket* wrapped_socket_;
  scoped_ptr<net::DeterministicSocketData> data_;
  scoped_ptr<ChromeStreamWriter> writer_;
  net::BoundNetLog net_log_;
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

TEST_F(StreamChannelTest, SyncSend) {
  // Covering the normal case when the message is sent synchronously.
  net::MockWrite writes[] = {
    net::MockWrite(net::SYNCHRONOUS, 0, RegisterRequest),
  };

  Initialize(writes, arraysize(writes));
  
  int rv = WriteMessage(callback_.callback());
  ASSERT_EQ(net::OK, rv);

  Finish();
}

TEST_F(StreamChannelTest, AsyncSend) {
  // During congestion or due to system limits, the message can be written
  // in two separated data blocks.
  net::MockWrite writes[] = {
    net::MockWrite(net::ASYNC, 0,
       "REGISTER sip:registrar.biloxi.com SIP/2.0\r\n"
       "v: SIP/2.0/UDP bobspc.biloxi.com:5060;rport;branch=z9hG4bKnashds7\r\n"
       "Max-Forwards: 70\r\n"),
    net::MockWrite(net::ASYNC, 1,
       "t: \"Bob\" <sip:bob@biloxi.com>\r\n"
       "f: \"Bob\" <sip:bob@biloxi.com>;tag=456248\r\n"
       "i: 843817637684230@998sdasdh09\r\n"
       "CSeq: 1826 REGISTER\r\n"
       "m: <sip:bob@192.0.2.4>\r\n"
       "Expires: 7200\r\n"
       "l: 0\r\n"
       "\r\n"),
  };

  Initialize(writes, arraysize(writes));

  int rv = WriteMessage(callback_.callback());
  ASSERT_EQ(net::ERR_IO_PENDING, rv);

  // For the first time, just a part of the message is sent,
  wrapped_socket_->CompleteWrite();
  // and the second part comes after.
  data_->RunFor(1);
  ASSERT_FALSE(callback_.have_result());

  // Now the second part completes.
  wrapped_socket_->CompleteWrite();
  data_->RunFor(1);
  ASSERT_TRUE(callback_.have_result());

  // The result must be OK now.
  rv = callback_.WaitForResult();
  ASSERT_EQ(net::OK, rv);

  Finish();
}

TEST_F(StreamChannelTest, OverlappedSend) {
  // This test covers the case where two requests are enqueue and sent
  // asynchrously.
  net::MockWrite writes[] = {
    net::MockWrite(net::ASYNC, 0, RegisterRequest),
    net::MockWrite(net::ASYNC, 1, RegisterRequest),
  };

  Initialize(writes, arraysize(writes));

  int rv = WriteMessage(callback_.callback());
  ASSERT_EQ(net::ERR_IO_PENDING, rv);

  // Send the same message again, should be enqueued.
  rv = WriteMessage(callback_.callback());
  ASSERT_EQ(net::ERR_IO_PENDING, rv);

  // First message is sent.
  wrapped_socket_->CompleteWrite();
  data_->RunFor(1);
  ASSERT_TRUE(callback_.have_result());
  rv = callback_.WaitForResult();
  ASSERT_EQ(net::OK, rv);

  // Second message is sent.
  wrapped_socket_->CompleteWrite();
  data_->RunFor(1);
  ASSERT_TRUE(callback_.have_result());
  rv = callback_.WaitForResult();
  ASSERT_EQ(net::OK, rv);

  Finish();
}

TEST_F(StreamChannelTest, SyncSendError) {
  // Synchronous error while sending data.
  net::MockWrite writes[] = {
    net::MockWrite(net::SYNCHRONOUS, net::ERR_CONNECTION_ABORTED),
  };

  Initialize(writes, arraysize(writes));

  int rv = WriteMessage(callback_.callback());
  ASSERT_EQ(net::ERR_CONNECTION_ABORTED, rv);

  Finish();
}

TEST_F(StreamChannelTest, AsyncSendError) {
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

  // The connection will be reset while sending the message.
  wrapped_socket_->CompleteWrite();
  wrapped_socket_->CompleteWrite();
  data_->RunFor(2); // there will be a second write attempt
  ASSERT_TRUE(callback_.have_result());
  rv = callback_.WaitForResult();
  ASSERT_EQ(net::ERR_CONNECTION_CLOSED, rv);

  Finish();
}

TEST_F(StreamChannelTest, AsyncConnReset) {
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

  // There will be a result = 0 while sending data.
  wrapped_socket_->CompleteWrite();
  wrapped_socket_->CompleteWrite();
  data_->RunFor(2); // there will be a second write attempt
  ASSERT_TRUE(callback_.have_result());
  rv = callback_.WaitForResult();
  ASSERT_EQ(net::ERR_CONNECTION_RESET, rv);

  Finish();
}
