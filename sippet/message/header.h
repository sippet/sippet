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

class Header : public ilist_node<Header> {
public:
  //! An enumeration to indicate the message header type.
  enum Type {
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

private:
  Type type_;

  Header &operator=(const Header &);

protected:
  Header(const Header &other) : type_(other.type_) {}
  Header(Type type) : type_(type) {}

  virtual Header *DoClone() const = 0;

public:
  Type type() const { return type_; }

  scoped_ptr<Header> Clone() const { return scoped_ptr<Header>(DoClone()); }

  virtual void print(raw_ostream &os) const = 0;
};

// isa - Provide some specializations of isa so that we don't have to include
// the subtype header files to test to see if the value is a subclass...
//
template <> struct isa_impl<Accept, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_ACCEPT;
  }
};

template <> struct isa_impl<AcceptEncoding, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_ACCEPT_ENCODING;
  }
};

template <> struct isa_impl<AcceptLanguage, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_ACCEPT_LANGUAGE;
  }
};

template <> struct isa_impl<AlertInfo, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_ALERT_INFO;
  }
};

template <> struct isa_impl<Allow, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_ALLOW;
  }
};

template <> struct isa_impl<AuthenticationInfo, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_AUTHENTICATION_INFO;
  }
};

template <> struct isa_impl<Authorization, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_AUTHORIZATION;
  }
};

template <> struct isa_impl<CallId, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_CALL_ID;
  }
};

template <> struct isa_impl<CallInfo, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_CALL_INFO;
  }
};

template <> struct isa_impl<Contact, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_CONTACT;
  }
};

template <> struct isa_impl<ContentDisposition, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_CONTENT_DISPOSITION;
  }
};

template <> struct isa_impl<ContentEncoding, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_CONTENT_ENCODING;
  }
};

template <> struct isa_impl<ContentLanguage, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_CONTENT_LANGUAGE;
  }
};

template <> struct isa_impl<ContentLength, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_CONTENT_LENGTH;
  }
};

template <> struct isa_impl<ContentType, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_CONTENT_TYPE;
  }
};

template <> struct isa_impl<Cseq, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_CSEQ;
  }
};

template <> struct isa_impl<Date, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_DATE;
  }
};

template <> struct isa_impl<ErrorInfo, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_ERROR_INFO;
  }
};

template <> struct isa_impl<Expires, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_EXPIRES;
  }
};

template <> struct isa_impl<From, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_FROM;
  }
};

template <> struct isa_impl<InReplyTo, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_IN_REPLY_TO;
  }
};

template <> struct isa_impl<MaxForwards, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_MAX_FORWARDS;
  }
};

template <> struct isa_impl<MinExpires, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_MIN_EXPIRES;
  }
};

template <> struct isa_impl<MimeVersion, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_MIME_VERSION;
  }
};

template <> struct isa_impl<Organization, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_ORGANIZATION;
  }
};

template <> struct isa_impl<Priority, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_PRIORITY;
  }
};

template <> struct isa_impl<ProxyAuthenticate, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_PROXY_AUTHENTICATE;
  }
};

template <> struct isa_impl<ProxyAuthorization, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_PROXY_AUTHORIZATION;
  }
};

template <> struct isa_impl<ProxyRequire, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_PROXY_REQUIRE;
  }
};

template <> struct isa_impl<RecordRoute, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_RECORD_ROUTE;
  }
};

template <> struct isa_impl<ReplyTo, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_REPLY_TO;
  }
};

template <> struct isa_impl<Require, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_REQUIRE;
  }
};

template <> struct isa_impl<RetryAfter, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_RETRY_AFTER;
  }
};

template <> struct isa_impl<Route, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_ROUTE;
  }
};

template <> struct isa_impl<Server, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_SERVER;
  }
};

template <> struct isa_impl<Subject, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_SUBJECT;
  }
};

template <> struct isa_impl<Supported, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_SUPPORTED;
  }
};

template <> struct isa_impl<Timestamp, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_TIMESTAMP;
  }
};

template <> struct isa_impl<To, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_TO;
  }
};

template <> struct isa_impl<Unsupported, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_UNSUPPORTED;
  }
};

template <> struct isa_impl<UserAgent, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_USER_AGENT;
  }
};

template <> struct isa_impl<Via, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_VIA;
  }
};

template <> struct isa_impl<Warning, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_WARNING;
  }
};

template <> struct isa_impl<WwwAuthenticate, Header> {
  static inline bool doit(const Header &h) {
    return h.type() == Header::HDR_WWW_AUTHENTICATE;
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADER_H_

/* Modeline for vim: set tw=79 et ts=4: */

