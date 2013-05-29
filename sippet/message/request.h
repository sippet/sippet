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

#ifndef SIPPET_MESSAGE_REQUEST_H_
#define SIPPET_MESSAGE_REQUEST_H_

#include "sippet/message/message.h"

namespace sippet {

class Request : public Message {
private:
  DISALLOW_COPY_AND_ASSIGN(Request);

private:
  Request(MethodTy method, const Uri &destUri);
  virtual ~Request();

public:
  typedef talk_base::scoped_refptr<Request> pointer;

  //! An enumeration to indicate the request method.
  enum MessageTy {
    METHOD_INVITE     // Indicates the method INVITE
    METHOD_ACK        // Indicates the method ACK
    METHOD_BYE        // Indicates the method BYE
    METHOD_CANCEL     // Indicates the method CANCEL
    METHOD_OPTIONS    // Indicates the method OPTIONS
    METHOD_REGISTER   // Indicates the method REGISTER
    METHOD_PRACK      // Indicates the method PRACK
    METHOD_SUBSCRIBE  // Indicates the method SUBSCRIBE
    METHOD_NOTIFY     // Indicates the method NOTIFY
    METHOD_PUBLISH    // Indicates the method PUBLISH
    METHOD_INFO       // Indicates the method INFO
    METHOD_REFER      // Indicates the method REFER
    METHOD_MESSAGE    // Indicates the method MESSAGE
    METHOD_UPDATE     // Indicates the method UPDATE
  };

  //! Create a new request message.
  static pointer Create(MethodTy method, const Uri &destUri) {
    return new Request(method, destUri);
  }

  //! Returns true if the current message is a request.
  virtual bool IsRequest() { return true; }

  //! Returns true if the current message is a response.
  virtual bool IsResponse() { return false; }

private:
  MethodTy method_;
  Uri destUri_;
};

}

#endif // SIPPET_MESSAGE_REQUEST_H_

/* Modeline for vim: set tw=79 et ts=4: */

