// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/ua/user_agent.h"
#include "net/base/net_errors.h"

namespace sippet {
namespace ua {

UserAgent::UserAgent() {
}

void UserAgent::AppendHandler(Delegate *delegate) {
  handlers_.push_back(delegate);
}

scoped_refptr<Request> UserAgent::CreateRequest(
    const Method &method,
    const GURL &from,
    const GURL &to) {
  NOTIMPLEMENTED();
  return 0;
}

scoped_refptr<Request> UserAgent::CreateRequest(
    const Method &method,
    const scoped_refptr<Dialog> &dialog) {
  NOTIMPLEMENTED();
  return 0;
}

scoped_refptr<Response> UserAgent::CreateResponse(
    int response_code,
    const scoped_refptr<Request> &request) {
  NOTIMPLEMENTED();
  return 0;
}

int UserAgent::Send(
    const scoped_refptr<Message> &message,
    const net::CompletionCallback& callback) {
  NOTIMPLEMENTED();
  return net::ERR_NOT_IMPLEMENTED;
}

void UserAgent::OnChannelConnected(const EndPoint &destination) {
  NOTIMPLEMENTED();
}

void UserAgent::OnChannelClosed(const EndPoint &destination, int err) {
  NOTIMPLEMENTED();
}

void UserAgent::OnIncomingRequest(
    const scoped_refptr<Request> &request) {
  NOTIMPLEMENTED();
}

void UserAgent::OnIncomingResponse(
    const scoped_refptr<Response> &response) {
  NOTIMPLEMENTED();
}

void UserAgent::OnTimedOut(const std::string &id) {
  NOTIMPLEMENTED();
}

void UserAgent::OnTransportError(const std::string &id, int error) {
  NOTIMPLEMENTED();
}

} // End of ua namespace
} // End of sippet namespace
