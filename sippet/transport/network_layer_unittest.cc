// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport/network_layer.h"
#include "sippet/transport/transaction_factory.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace sippet {

namespace {

class NetworkLayerDelegate : public NetworkLayer::Delegate {
 public:
  MOCK_METHOD2(OnChannelClosed, void(const EndPoint&, int));
  MOCK_METHOD1(OnIncomingMessage, void(Message*));
};

class TransactionFactoryImpl : public TransactionFactory {
 public:
  virtual ClientTransaction *CreateClientTransaction(
      const Method &method,
      const std::string &transaction_id,
      const scoped_refptr<Channel> &channel,
      TransactionDelegate *delegate) {
    return 0;
  }

  virtual ServerTransaction *CreateServerTransaction(
      const Method &method,
      const std::string &transaction_id,
      const scoped_refptr<Channel> &channel,
      TransactionDelegate *delegate) {
    return 0;
  }
};

} // End of empty namespace

class NetworkLayerTest : public testing::Test {
 public:
  void Finish() {
    MessageLoop::current()->RunUntilIdle();
  }

  void Initialize() {
    network_layer_ = new NetworkLayer(&delegate_, &transaction_factory_);
  }

  NetworkLayerDelegate delegate_;
  TransactionFactoryImpl transaction_factory_;
  scoped_refptr<NetworkLayer> network_layer_;
};

TEST_F(NetworkLayerTest, FirstTest) {
  Initialize();
  Finish();
}

} // End of sippet namespace