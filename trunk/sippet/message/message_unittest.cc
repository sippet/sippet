/* 
 * Copyright (c) 2013, Guilherme Balena Versiani
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies, 
 * either expressed or implied, of the FreeBSD Project.
 */

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
  const char *raw_message = "INVITE sip:alice@biloxi.com SIP/2.0\n\n";
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
