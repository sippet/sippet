// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/message.h"

#include <string>

#include "base/memory/ptr_util.h"
#include "testing/gtest/include/gtest/gtest.h"

using sippet::Message;
using sippet::Header;
using sippet::raw_ostream;
using sippet::isa;
using sippet::dyn_cast;
using sippet::Request;
using sippet::Response;

class InstanceOfMessage : public Message {
 public:
  InstanceOfMessage() : Message(true, Outgoing) {}
  std::string GetDialogId() const override {
    return "";  // It won't be used here
  }
 private:
  friend class base::RefCountedThreadSafe<InstanceOfMessage>;
  ~InstanceOfMessage() override {}
};

class InstanceOfHeader : public Header {
 private:
  InstanceOfHeader(const InstanceOfHeader &other) : Header(other) {}
  InstanceOfHeader &operator=(const InstanceOfHeader &other);
  InstanceOfHeader *DoClone() const override {
    return new InstanceOfHeader(*this);
  }

 public:
  InstanceOfHeader() : Header(Header::HDR_ACCEPT) {}
  virtual std::unique_ptr<InstanceOfHeader> Clone() {
    return std::unique_ptr<InstanceOfHeader>(DoClone());
  }
  void print(raw_ostream &os) const override { }
};

class MessageTest : public testing::Test {
 public:
  MessageTest() : message_(new InstanceOfMessage()) { }
  ~MessageTest() override {}

  scoped_refptr<Message> message_;
};

TEST_F(MessageTest, Basic) {
  EXPECT_TRUE(message_->empty());

  // Just mark the first header
  message_->push_back(base::WrapUnique(new InstanceOfHeader()));

  std::unique_ptr<Header> one[] = {
    std::unique_ptr<Header>(new InstanceOfHeader()),
  };
  message_->insert(message_->begin(), one, one+1);

  std::unique_ptr<Header> set_of[] = {
    std::unique_ptr<Header>(new InstanceOfHeader()),
    std::unique_ptr<Header>(new InstanceOfHeader()),
  };
  message_->insert(message_->begin(), set_of, set_of+2);

  EXPECT_FALSE(message_->empty());
  EXPECT_EQ(4ul, message_->size());

  // They should have been cloned
  EXPECT_TRUE(one[0].get());
  EXPECT_TRUE(set_of[1].get());

  message_->erase(message_->begin());

  Message::iterator it = message_->begin();
  it++;
  it++;
  it++;

  message_->erase(it, message_->end());
  EXPECT_EQ(2ul, message_->size());

  message_->pop_front();
  EXPECT_EQ(1ul, message_->size());

  message_->clear();
  EXPECT_TRUE(message_->empty());
}

TEST(RequestTest, Basic) {
  const char *raw_message =
    "INVITE sip:alice@biloxi.com SIP/2.0\n"
    "Accept: text /  html;q=1.0, application/sdp ;q=0.9, text/plain   \n"
    "\n\n";
  scoped_refptr<Message> message = Message::Parse(raw_message);

  ASSERT_TRUE(isa<Request>(message));
  ASSERT_TRUE(message->IsRequest());
  ASSERT_FALSE(message->IsResponse());

  scoped_refptr<Request> request = dyn_cast<Request>(message);
}

TEST(ResponseTest, Basic) {
  const char *raw_message = "SIP/2.0 200 OK\n\n";
  scoped_refptr<Message> message = Message::Parse(raw_message);

  ASSERT_TRUE(isa<Response>(message));
  ASSERT_FALSE(message->IsRequest());
  ASSERT_TRUE(message->IsResponse());

  scoped_refptr<Response> response = dyn_cast<Response>(message);
}
