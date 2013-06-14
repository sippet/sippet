// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/message.h"
#include "sippet/message/request.h"
#include "sippet/message/response.h"

#include "testing/gtest/include/gtest/gtest.h"

using namespace sippet;

class InstanceOfMessage : public Message {
public:
  InstanceOfMessage() : Message(true) {}
};

class InstanceOfHeader : public Header {
private:
  InstanceOfHeader(const InstanceOfHeader &other) : Header(other) {}
  InstanceOfHeader &operator=(const InstanceOfHeader &other);
  virtual InstanceOfHeader *DoClone() const {
    return new InstanceOfHeader(*this);
  }
public:
  InstanceOfHeader() : Header(Header::HDR_ACCEPT) {}
  virtual scoped_ptr<InstanceOfHeader> Clone() {
    return scoped_ptr<InstanceOfHeader>(DoClone());
  }
  virtual void print(raw_ostream &os) const { }
};

class MessageTest : public testing::Test {
 public:
  MessageTest() : message_(new InstanceOfMessage()) { }
  ~MessageTest() {}

  scoped_refptr<Message> message_;
};

TEST_F(MessageTest, Basic) {
  EXPECT_TRUE(message_->empty());

  // Just mark the first header
  message_->push_back(scoped_ptr<Header>(new InstanceOfHeader()));
  message_->push_back(scoped_ptr<Header>(new InstanceOfHeader()).Pass());

  scoped_ptr<Header> one[] = {
    scoped_ptr<Header>(new InstanceOfHeader()),
  };
  message_->insert(message_->begin(), one, one+1);

  scoped_ptr<Header> set_of[] = {
    scoped_ptr<Header>(new InstanceOfHeader()),
    scoped_ptr<Header>(new InstanceOfHeader()),
  };
  message_->insert(message_->begin(), set_of, set_of+2);

  EXPECT_FALSE(message_->empty());
  EXPECT_EQ(5, message_->size());

  EXPECT_EQ(0, one[0].get());
  EXPECT_EQ(0, set_of[1].get());

  message_->erase(message_->begin());

  Message::iterator it = message_->begin();
  it++;
  it++;
  it++;

  message_->erase(it, message_->end());
  EXPECT_EQ(3, message_->size());

  message_->pop_front();
  EXPECT_EQ(2, message_->size());

  message_->pop_back();
  EXPECT_EQ(1, message_->size());

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
