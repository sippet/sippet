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

#include "sippet/message/headers/accept.h"
#include "sippet/message/headers/accept_encoding.h"
#include "sippet/message/headers/accept_language.h"
#include "sippet/message/headers/alert_info.h"
#include "sippet/message/headers/allow.h"
#include "sippet/message/headers/authentication_info.h"
#include "sippet/message/headers/authorization.h"
#include "sippet/message/headers/call_id.h"
#include "sippet/message/headers/call_info.h"
#include "sippet/message/headers/contact.h"
#include "sippet/message/headers/content_disposition.h"
#include "sippet/message/headers/content_encoding.h"
#include "sippet/message/headers/content_language.h"
#include "sippet/message/headers/content_length.h"
#include "sippet/message/headers/content_type.h"
#include "sippet/message/headers/cseq.h"
#include "sippet/message/headers/date.h"
#include "sippet/message/headers/error_info.h"
#include "sippet/message/headers/expires.h"
#include "sippet/message/headers/from.h"
#include "sippet/message/headers/has_multiple.h"
#include "sippet/message/headers/has_parameters.h"
#include "sippet/message/headers/in_reply_to.h"
#include "sippet/message/headers/max_forwards.h"
#include "sippet/message/headers/mime_version.h"
#include "sippet/message/headers/min_expires.h"
#include "sippet/message/headers/organization.h"
#include "sippet/message/headers/priority.h"
#include "sippet/message/headers/proxy_authenticate.h"
#include "sippet/message/headers/proxy_authorization.h"
#include "sippet/message/headers/proxy_require.h"
#include "sippet/message/headers/record_route.h"
#include "sippet/message/headers/reply_to.h"
#include "sippet/message/headers/require.h"
#include "sippet/message/headers/retry_after.h"
#include "sippet/message/headers/route.h"
#include "sippet/message/headers/server.h"
#include "sippet/message/headers/subject.h"
#include "sippet/message/headers/supported.h"
#include "sippet/message/headers/timestamp.h"
#include "sippet/message/headers/to.h"
#include "sippet/message/headers/unsupported.h"
#include "sippet/message/headers/user_agent.h"
#include "sippet/message/headers/via.h"
#include "sippet/message/headers/warning.h"
#include "sippet/message/headers/www_authenticate.h"

#include "testing/gtest/include/gtest/gtest.h"

using namespace sippet;

class InstanceOfMessage : public Message {
public:
  InstanceOfMessage() : Message(true) {}
};

class InstanceOfHeader : public Header {
public:
  InstanceOfHeader() : Header(Header::HDR_ACCEPT) {}
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

TEST_F(MessageTest, Accept) {
  scoped_ptr<Accept> accept(new Accept);

  EXPECT_EQ(Header::HDR_ACCEPT, accept->type());

  Header *h = accept.get();
  EXPECT_TRUE(isa<Accept>(h));
}
