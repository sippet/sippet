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

#ifndef SIPPET_MESSAGE_HEADER_H_
#define SIPPET_MESSAGE_HEADER_H_

#include "sippet/base/ilist_node.h"
#include "base/memory/scoped_ptr.h"

namespace sippet {

class raw_ostream;
class Accept;
class AcceptEncoding;
class AcceptLanguage;
class AlertInfo;
class Allow;
class AuthenticationInfo;
class Authorization;
class CallId;
class CallInfo;
class Contact;
class ContentDisposition;
class ContentEncoding;
class ContentLanguage;
class ContentLength;
class ContentType;
class Cseq;
class Date;
class ErrorInfo;
class Expires;
class From;
class InReplyTo;
class MaxForwards;
class MinExpires;
class MimeVersion;
class Organization;
class Priority;
class ProxyAuthenticate;
class ProxyAuthorization;
class ProxyRequire;
class RecordRoute;
class ReplyTo;
class Require;
class RetryAfter;
class Route;
class Server;
class Subject;
class Supported;
class Timestamp;
class To;
class Unsupported;
class UserAgent;
class Via;
class Warning;
class WwwAuthenticate;
class Generic;

class Header : public ilist_node<Header> {
public:
  //! An enumeration to indicate the message header type.
  enum Type {
    HDR_ACCEPT = 0,
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
    HDR_WWW_AUTHENTICATE,
    HDR_GENERIC
  };

private:
  Type type_;

  Header &operator=(const Header &);

protected:
  Header(const Header &other) : type_(other.type_) {}
  Header(Type type) : type_(type) {}

  virtual Header *DoClone() const = 0;

public:
  static scoped_ptr<Header> Parse(const std::string &raw_header);

  Type type() const { return type_; }

  scoped_ptr<Header> Clone() const { return scoped_ptr<Header>(DoClone()); }

  virtual void print(raw_ostream &os) const = 0;
};

// isa - Provide some specializations of isa so that we don't have to include
// the subtype header files to test to see if the value is a subclass...
//
#define is_a(H, T)                                   \
template <> struct isa_impl<H, Header> {             \
  static inline bool doit(const Header &h) {         \
    return h.type() == Header::T;                    \
  }                                                  \
};

is_a(Accept,                HDR_ACCEPT)
is_a(AcceptEncoding,        HDR_ACCEPT_ENCODING)
is_a(AcceptLanguage,        HDR_ACCEPT_LANGUAGE)
is_a(AlertInfo,             HDR_ALERT_INFO)
is_a(Allow,                 HDR_ALLOW)
is_a(AuthenticationInfo,    HDR_AUTHENTICATION_INFO)
is_a(Authorization,         HDR_AUTHORIZATION)
is_a(CallId,                HDR_CALL_ID)
is_a(CallInfo,              HDR_CALL_INFO)
is_a(Contact,               HDR_CONTACT)
is_a(ContentDisposition,    HDR_CONTENT_DISPOSITION)
is_a(ContentEncoding,       HDR_CONTENT_ENCODING)
is_a(ContentLanguage,       HDR_CONTENT_LANGUAGE)
is_a(ContentLength,         HDR_CONTENT_LENGTH)
is_a(ContentType,           HDR_CONTENT_TYPE)
is_a(Cseq,                  HDR_CSEQ)
is_a(Date,                  HDR_DATE)
is_a(ErrorInfo,             HDR_ERROR_INFO)
is_a(Expires,               HDR_EXPIRES)
is_a(From,                  HDR_FROM)
is_a(InReplyTo,             HDR_IN_REPLY_TO)
is_a(MaxForwards,           HDR_MAX_FORWARDS)
is_a(MinExpires,            HDR_MIN_EXPIRES)
is_a(MimeVersion,           HDR_MIME_VERSION)
is_a(Organization,          HDR_ORGANIZATION)
is_a(Priority,              HDR_PRIORITY)
is_a(ProxyAuthenticate,     HDR_PROXY_AUTHENTICATE)
is_a(ProxyAuthorization,    HDR_PROXY_AUTHORIZATION)
is_a(ProxyRequire,          HDR_PROXY_REQUIRE)
is_a(RecordRoute,           HDR_RECORD_ROUTE)
is_a(ReplyTo,               HDR_REPLY_TO)
is_a(Require,               HDR_REQUIRE)
is_a(RetryAfter,            HDR_RETRY_AFTER)
is_a(Route,                 HDR_ROUTE)
is_a(Server,                HDR_SERVER)
is_a(Subject,               HDR_SUBJECT)
is_a(Supported,             HDR_SUPPORTED)
is_a(Timestamp,             HDR_TIMESTAMP)
is_a(To,                    HDR_TO)
is_a(Unsupported,           HDR_UNSUPPORTED)
is_a(UserAgent,             HDR_USER_AGENT)
is_a(Via,                   HDR_VIA)
is_a(Warning,               HDR_WARNING)
is_a(WwwAuthenticate,       HDR_WWW_AUTHENTICATE)
is_a(Generic,               HDR_GENERIC)

#undef is_a

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADER_H_
