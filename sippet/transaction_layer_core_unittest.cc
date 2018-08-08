// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transaction_layer_core.h"

#include <memory>

#include "sippet/core.h"
#include "sippet/message/request.h"
#include "sippet/message/response.h"
#include "sippet/transport_layer.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace sippet {

namespace {

void HeadersToRaw(std::string* headers) {
  std::replace(headers->begin(), headers->end(), '\n', '\0');
  if (!headers->empty())
    *headers += '\0';
}

class MockTransportLayer : public TransportLayer {
 public:
  MOCK_METHOD2(Init, void(TransactionLayer* transaction_layer, Core* core));
  MOCK_METHOD0(Start, void());
  MOCK_METHOD0(Stop, void());
  MOCK_METHOD1(SendMessage, void(scoped_refptr<Message> message));
};

class MockCore : public Core {
 public:
  MOCK_METHOD1(OnIncomingRequest, void(scoped_refptr<Request> request));
  MOCK_METHOD1(OnIncomingResponse, void(scoped_refptr<Response> response));
  MOCK_METHOD1(OnTimedOut, void(const std::string& id));
  MOCK_METHOD2(OnTransportError, void(const std::string& id, int error));
};

class TransactionLayerCoreTest : public testing::Test {
 protected:
  TransactionLayerCoreTest() {
    TransactionConfig config;
    transaction_layer_core_ =
        new TransactionLayerCore(&transport_layer_, &core_, config);
  }

  ~TransactionLayerCoreTest() override {}

  MockCore core_;
  MockTransportLayer transport_layer_;
  scoped_refptr<TransactionLayerCore> transaction_layer_core_;
};

TEST_F(TransactionLayerCoreTest, StartSendsRequest) {
  std::string raw_headers =
      "REGISTER sip:registrar.biloxi.com SIP/2.0\n"
      "Via: SIP/2.0/UDP bobspc.biloxi.com:5060;branch=z9hG4bKnashds7\n"
      "Max-Forwards: 70\n"
      "To: Bob <sip:bob@biloxi.com>\n"
      "From: Bob <sip:bob@biloxi.com>;tag=456248\n"
      "Call-ID: 843817637684230@998sdasdh09\n"
      "CSeq: 1826 REGISTER\n"
      "Contact: <sip:bob@192.0.2.4>\n"
      "Expires: 7200\n"
      "Content-Length: 0\n";
  HeadersToRaw(&raw_headers);
  scoped_refptr<Message> message = Request::Parse(raw_headers);
  transaction_layer_core_->SendRequest(message->as_request());
}

}  // namespace

}  // namespace sippet