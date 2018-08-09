// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transaction_layer.h"

#include <memory>

#include "base/threading/thread_task_runner_handle.h"
#include "base/run_loop.h"
#include "net/url_request/url_request_test_util.h"
#include "sippet/core.h"
#include "sippet/message/request.h"
#include "sippet/message/response.h"
#include "sippet/transport_layer.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/gmock/include/gmock/gmock.h"

using namespace net;
using testing::_;

namespace sippet {

namespace {

void HeadersToRaw(std::string* headers) {
  std::replace(headers->begin(), headers->end(), '\n', '\0');
  if (!headers->empty())
    *headers += '\0';
}

void SetTimedOutAndQuitLoop(const base::WeakPtr<bool> timed_out,
                            const base::Closure& quit_loop_func) {
  if (timed_out) {
    *timed_out = true;
    quit_loop_func.Run();
  }
}

bool RunLoopWithTimeout(base::RunLoop* run_loop, int64_t milliseconds) {
  bool timed_out = false;
  base::WeakPtrFactory<bool> timed_out_weak_factory(&timed_out);
  base::ThreadTaskRunnerHandle::Get()->PostDelayedTask(
      FROM_HERE,
      base::Bind(&SetTimedOutAndQuitLoop, timed_out_weak_factory.GetWeakPtr(),
                 run_loop->QuitClosure()),
      base::TimeDelta::FromMilliseconds(milliseconds));
  run_loop->Run();
  return !timed_out;
}

class MockCore : public Core {
 public:
  MOCK_METHOD1(OnIncomingRequest, void(scoped_refptr<Request> request));
  MOCK_METHOD1(OnIncomingResponse, void(scoped_refptr<Response> response));
  MOCK_METHOD1(OnTimedOut, void(const std::string& id));
  MOCK_METHOD2(OnTransportError, void(const std::string& id, int error));
};

class TransactionLayerTest : public testing::Test,
                             public TransportLayer {
 public:
  class MockConnection : public TransportLayer::Connection {
   public:
    MockConnection()
        : expected_sent_messages_(0) {}

    MOCK_METHOD0(GetTransportProtocol, std::string());
    MOCK_METHOD0(GetDestination, net::HostPortPair());

    void SendMessage(scoped_refptr<Message> message) {
      DCHECK(message);

      sent_messages_.push_back(message);

      if (sent_messages_.size() == expected_sent_messages_) {
        run_loop_quit_func_.Run();
      }
    }

    size_t expected_sent_messages_;
    base::Closure run_loop_quit_func_;
    std::vector<scoped_refptr<Message>> sent_messages_;

   private:
    ~MockConnection() override {}
  };

  TransactionLayerTest()
      : connection_(new MockConnection),
        init_called_(false) {
    transaction_layer_ = TransactionLayer::Create(this, &core_);
  }

  ~TransactionLayerTest() override {}

  bool RunUntilMessagesSent(size_t count, int64_t milliseconds) {
    quit_after_sent_messages_count_ = count;
    if (connection_->sent_messages_.size() == count)
      return true;

    base::RunLoop run_loop;
    connection_->expected_sent_messages_ = count;
    connection_->run_loop_quit_func_ = run_loop.QuitClosure();
    bool success = RunLoopWithTimeout(&run_loop, milliseconds);
    connection_->run_loop_quit_func_.Reset();
    return success;
  }

  void Init(TransactionLayer* transaction_layer, Core* core) override {
    DCHECK(!init_called_);
    init_called_ = true;
  }

  MOCK_METHOD0(Start, void());
  MOCK_METHOD0(Stop, void());

  void Connect(const GURL& destination,
      const ConnectCalback& callback) override {
    if (expected_connect_destination_.has_value()) {
      ASSERT_TRUE(*expected_connect_destination_ == destination);
    }

    base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(callback, connection_));
  }

 protected:
  MockCore core_;
  base::Optional<GURL> expected_connect_destination_;
  std::unique_ptr<TransactionLayer> transaction_layer_;
  scoped_refptr<MockConnection> connection_;

 private:
  bool init_called_;
  size_t quit_after_sent_messages_count_;
};

TEST_F(TransactionLayerTest, StartSendsRequest) {
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

  base::RunLoop run_loop;

  scoped_refptr<URLRequestContextGetter> request_context_getter(
      new TestURLRequestContextGetter(base::ThreadTaskRunnerHandle::Get()));

  transaction_layer_->SetRequestContext(request_context_getter.get());
  transaction_layer_->Start();

  EXPECT_CALL(*connection_, GetTransportProtocol());
  EXPECT_CALL(*connection_, GetDestination());

  transaction_layer_->SendRequest(message->as_request());

  ASSERT_TRUE(RunUntilMessagesSent(1, 10));
}

}  // namespace
}  // namespace sippet