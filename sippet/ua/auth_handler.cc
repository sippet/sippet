// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/ua/auth_handler.h"
#include "sippet/message/headers.h"
#include "net/base/net_errors.h"

namespace sippet {

// Based on net/http/http_auth_handler.cc,
// revision 238260

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace {

net::NetLog::EventType EventTypeFromAuthTarget(Auth::Target target) {
  switch (target) {
    case net::HttpAuth::AUTH_PROXY:
      return net::NetLog::TYPE_AUTH_PROXY;
    case net::HttpAuth::AUTH_SERVER:
      return net::NetLog::TYPE_AUTH_SERVER;
    default:
      NOTREACHED();
      return net::NetLog::TYPE_CANCELLED;
  }
}

} // namespace

AuthHandler::AuthHandler()
  : auth_scheme_(net::HttpAuth::AUTH_SCHEME_MAX),
    score_(-1),
    target_(net::HttpAuth::AUTH_NONE) {
}

AuthHandler::~AuthHandler() {
}

bool AuthHandler::InitFromChallenge(
    const Challenge& challenge,
    Auth::Target target,
    const GURL& origin,
    const net::BoundNetLog& net_log) {
  origin_ = origin;
  target_ = target;
  score_ = -1;
  net_log_ = net_log;

  bool ok = Init(challenge);

  // Init() is expected to set the scheme, realm and score. The
  // realm may be empty.
  DCHECK(!ok || score_ != -1);
  DCHECK(!ok || auth_scheme_ != net::HttpAuth::AUTH_SCHEME_MAX);

  return ok;
}

int AuthHandler::GenerateAuth(
    const net::AuthCredentials* credentials,
    const scoped_refptr<Request> &request,
    const net::CompletionCallback& callback) {
  DCHECK(request);
  DCHECK(credentials != NULL || AllowsDefaultCredentials());
  DCHECK(callback_.is_null());
  callback_ = callback;
  net_log_.BeginEvent(EventTypeFromAuthTarget(target_));
  int rv = GenerateAuthImpl(
      credentials, request,
      base::Bind(&AuthHandler::OnGenerateAuthComplete,
                 base::Unretained(this)));
  if (rv != net::ERR_IO_PENDING)
    FinishGenerateAuth();
  return rv;
}

bool AuthHandler::AllowsDefaultCredentials() {
  return false;
}

bool AuthHandler::AllowsExplicitCredentials() {
  return true;
}

bool AuthHandler::NeedsIdentity() {
  return true;
}

void AuthHandler::OnGenerateAuthComplete(int rv) {
  net::CompletionCallback callback = callback_;
  FinishGenerateAuth();
  if (!callback.is_null())
    callback.Run(rv);
}

void AuthHandler::FinishGenerateAuth() {
  net_log_.EndEvent(EventTypeFromAuthTarget(target_));
  callback_.Reset();
}

}  // namespace net