// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/ua/auth.h"
#include "sippet/ua/auth_handler.h"
#include "sippet/ua/auth_handler_factory.h"
#include "sippet/message/request.h"
#include "sippet/message/response.h"
#include "sippet/message/headers.h"
#include "sippet/uri/uri.h"
#include "net/base/net_errors.h"

#include <sstream>

namespace sippet {

// Based on net/http/http_auth.cc,
// revision 238260

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

Header::Type Auth::GetChallengeHeaderType(Target target) {
  switch (target) {
    case net::HttpAuth::AUTH_PROXY:
      return Header::HDR_PROXY_AUTHENTICATE;
    case net::HttpAuth::AUTH_SERVER:
      return Header::HDR_WWW_AUTHENTICATE;
    default:
      NOTREACHED();
      return Header::HDR_GENERIC;
  }
}

Header::Type Auth::GetAuthorizationHeaderType(Target target) {
  switch (target) {
    case net::HttpAuth::AUTH_PROXY:
      return Header::HDR_PROXY_AUTHORIZATION;
    case net::HttpAuth::AUTH_SERVER:
      return Header::HDR_AUTHORIZATION;
    default:
      NOTREACHED();
      return Header::HDR_GENERIC;
  }
}

const char* Auth::SchemeToString(Scheme scheme) {
  return net::HttpAuth::SchemeToString(scheme);
}

Auth::Target Auth::GetChallengeTarget(
    const scoped_refptr<Response> &response) {
  Auth::Target target = net::HttpAuth::AUTH_NONE;
  for (Message::iterator i = response->begin(), ie = response->end();
       i != ie; i++) {
    if (Header::HDR_WWW_AUTHENTICATE == i->type()) {
      target = net::HttpAuth::AUTH_SERVER;
      break;
    } else if (Header::HDR_PROXY_AUTHENTICATE == i->type()) {
      target = net::HttpAuth::AUTH_PROXY;
      break;
    }
  }
  return target;
}

Challenge& Auth::GetChallengeFromHeader(Header* header) {
  Challenge *challenge = NULL;
  if (isa<WwwAuthenticate>(header)) {
    WwwAuthenticate *www_authenticate =
        dyn_cast<WwwAuthenticate>(header);
    challenge = www_authenticate;
  }
  else if (isa<ProxyAuthenticate>(header)) {
    ProxyAuthenticate *proxy_authenticate =
        dyn_cast<ProxyAuthenticate>(header);
    challenge = proxy_authenticate;
  }
  else {
    static Challenge null;
    NOTREACHED();
    return null;
  }
  return *challenge;
}

GURL Auth::GetResponseOrigin(const scoped_refptr<Response>& response) {
  std::ostringstream spec;
  DCHECK(response->refer_to());
  GURL request_uri(response->refer_to()->request_uri());
  if (request_uri.SchemeIs("sip") || request_uri.SchemeIs("sips")) {
    SipURI uri(request_uri);
    spec << uri.scheme() << ":"
         << uri.host() << ":"
         << uri.EffectiveIntPort();
    std::pair<bool, std::string> result = uri.parameter("transport");
    if (result.first)
      spec << ";transport=" << result.second;
  }
  // else return empty GURL
  return GURL(spec.str());
}

void Auth::ChooseBestChallenge(
    AuthHandlerFactory* auth_handler_factory,
    const scoped_refptr<Response> &response,
    const std::set<Scheme>& disabled_schemes,
    const net::BoundNetLog& net_log,
    scoped_ptr<AuthHandler>* handler) {
  DCHECK(auth_handler_factory);
  DCHECK(handler->get() == NULL);

  Auth::Target target = GetChallengeTarget(response);
  if (net::HttpAuth::AUTH_NONE == target)
    return;

  // Choose the challenge whose authentication handler gives the maximum score.
  scoped_ptr<AuthHandler> best;
  GURL origin(GetResponseOrigin(response));
  Header::Type header_type = GetChallengeHeaderType(target);
  for (Message::iterator i = response->begin(), ie = response->end();
       i != ie; i++) {
    if (header_type == i->type()) {
      Challenge& cur_challenge = GetChallengeFromHeader(i);
      scoped_ptr<AuthHandler> cur;
      int rv = auth_handler_factory->CreateChallengeAuthHandler(
          cur_challenge, target, origin, net_log, &cur);
      if (rv != net::OK) {
        VLOG(1) << "Unable to create AuthHandler. Status: "
                << net::ErrorToString(rv) << " Challenge: "
                << cur_challenge.ToString();
        continue;
      }
      if (cur.get() && (!best.get() || best->score() < cur->score()) &&
          (disabled_schemes.find(cur->auth_scheme()) == disabled_schemes.end()))
        best.swap(cur);
    }
  }
  handler->swap(best);
}

Auth::AuthorizationResult Auth::HandleChallengeResponse(
      AuthHandler* handler,
      const scoped_refptr<Response> &response,
      const std::set<Scheme>& disabled_schemes) {
  DCHECK(handler);
  DCHECK(response);
  Auth::Scheme current_scheme = handler->auth_scheme();
  if (disabled_schemes.find(current_scheme) != disabled_schemes.end())
    return net::HttpAuth::AUTHORIZATION_RESULT_REJECT;
  std::string current_scheme_name = SchemeToString(current_scheme);
  Auth::Target target = GetChallengeTarget(response);
  Header::Type header_type = GetChallengeHeaderType(target);
  Auth::AuthorizationResult authorization_result =
      net::HttpAuth::AUTHORIZATION_RESULT_INVALID;
  for (Message::iterator i = response->begin(), ie = response->end();
       i != ie; i++) {
    if (header_type == i->type()) {
      Challenge& challenge = GetChallengeFromHeader(i);
      if (!LowerCaseEqualsASCII(challenge.scheme(),
          current_scheme_name.c_str()))
        continue;
      authorization_result = handler->HandleAnotherChallenge(challenge);
      if (net::HttpAuth::AUTHORIZATION_RESULT_INVALID != authorization_result)
        return authorization_result;
    }
  }
  // Finding no matches is equivalent to rejection.
  return net::HttpAuth::AUTHORIZATION_RESULT_REJECT;
}

}  // namespace sippet