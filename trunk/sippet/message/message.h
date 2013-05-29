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

#ifndef SIPPET_MESSAGE_MESSAGE_H_
#define SIPPET_MESSAGE_MESSAGE_H_

#include <vector>
#include "talk/base/refcount.h"
#include "talk/base/scoped_ptr.h"
#include "talk/base/scoped_ref_ptr.h"
#include "talk/base/constructormagic.h"
#include "sippet/base/raw_ostream.h"
#include "sippet/message/header.h"

namespace sippet {

class Message : public talk_base::RefCountInterface {
private:
  DISALLOW_COPY_AND_ASSIGN(Message);

protected:
  Message();
  virtual ~Message();

public:
  typedef talk_base::scoped_refptr<Message> pointer;
  typedef header_iterator<Header> iterator;
  typedef header_iterator<const Header> const_iterator;

  //! An enumeration to indicate the message header.
  enum HeaderTy {
    HDR_ACCEPT,
    HDR_ACCEPT_ENCODING,
    HDR_ACCEPT_LANGUAGE,
    HDR_ALERT_INFO,
    HDR_ALLOW,
    HDR_AUTHENTICATION_INFO,
    HDR_AUTHORIZATION,
    HDR_CALL_ID,
    HDR_CALL_INFO,
    HDR_CONTACT,
    HDR_CONTENT_DISPOSITION,
    HDR_CONTENT_ENCODING,
    HDR_CONTENT_LANGUAGE,
    HDR_CONTENT_LENGTH,
    HDR_CONTENT_TYPE,
    HDR_CSEQ,
    HDR_DATE,
    HDR_ERROR_INFO,
    HDR_EXPIRES,
    HDR_FROM,
    HDR_IN_REPLY_TO,
    HDR_MAX_FORWARDS,
    HDR_MIN_EXPIRES,
    HDR_MIME_VERSION,
    HDR_ORGANIZATION,
    HDR_PRIORITY,
    HDR_PROXY_AUTHENTICATE,
    HDR_PROXY_AUTHORIZATION,
    HDR_PROXY_REQUIRE,
    HDR_RECORD_ROUTE,
    HDR_REPLY_TO,
    HDR_REQUIRE,
    HDR_RETRY_AFTER,
    HDR_ROUTE,
    HDR_SERVER,
    HDR_SUBJECT,
    HDR_SUPPORTED,
    HDR_TIMESTAMP,
    HDR_TO,
    HDR_UNSUPPORTED,
    HDR_USER_AGENT,
    HDR_VIA,
    HDR_WARNING,
    HDR_WWW_AUTHENTICATE
  };

  //! Returns true if the current message is a request.
  virtual bool IsRequest() = 0;

  //! Returns true if the current message is a response.
  virtual bool IsResponse() = 0;

  //! Returns a STL-like header iterator. Note you cannot change the headers
  //! while iterating.
        iterator begin()       { return iterator(headers_); }
  const_iterator begin() const { return const_iterator(headers_); }

  //! Returns a STL-like header iterator pointing to end. Note you cannot
  //! change the headers while iterating.
        iterator end()         { return iterator(0); }
  const_iterator end() const   { return const_iterator(0); }

  //! Return a reference to the first header found of the indicated name, or
  //! null if not found. Note you cannot change the headers while iterating.
        iterator FindFirst(HeaderTy name);
  const_iterator FindFirst(HeaderTy name) const;

  //! Return a reference to the next header found of the indicated name, or
  //! null if not found. Note you cannot change the headers while iterating.
        iterator FindNext(iterator start, HeaderTy name);
  const_iterator FindNext(iterator start, HeaderTy name) const;

  //! Append a header to the message.
  void Append(Header::pointer header);

  //! Print this message to the output.
  virtual void print(raw_ostream &os);

private:
  Header *headers_;
};

}

#endif // SIPPET_MESSAGE_MESSAGE_H_
