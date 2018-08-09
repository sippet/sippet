// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transaction_layer_impl.h"
#include "sippet/transaction_layer_core.h"
#include "sippet/transaction_layer_factory.h"
#include "sippet/transport_layer.h"
#include "sippet/message/request.h"
#include "sippet/message/response.h"

namespace sippet {

namespace {

static TransactionLayerFactory* g_factory = nullptr;

}  // namespace

TransactionLayerImpl::TransactionLayerImpl(TransportLayer* transport,
    Core* core, const TransactionConfig& config)
    : core_(new TransactionLayerCore(transport, core, config)) {
  transport->Init(this, core);
}

TransactionLayerImpl::~TransactionLayerImpl() {}

void TransactionLayerImpl::SetRequestContext(
    net::URLRequestContextGetter* request_context_getter) {
  core_->SetRequestContext(request_context_getter);
}

void TransactionLayerImpl::Start() {
  core_->Start();
}

std::string TransactionLayerImpl::SendRequest(scoped_refptr<Request> request) {
  return core_->SendRequest(request);
}

void TransactionLayerImpl::SendResponse(scoped_refptr<Response> response) {
  core_->SendResponse(response);
}

void TransactionLayerImpl::Terminate(const std::string& id) {
  core_->Terminate(id);
}

void TransactionLayerImpl::OnMessage(scoped_refptr<Message> message,
    scoped_refptr<TransportLayer::Connection> connection) {
  core_->OnMessage(message, connection);
}

void TransactionLayerImpl::OnTransportError(const std::string& id, int error) {
  core_->OnTransportError(id, error);
}

// static
TransactionLayerFactory* TransactionLayerImpl::factory() {
  return g_factory;
}

// static
void TransactionLayerImpl::set_factory(TransactionLayerFactory* factory) {
  g_factory = factory;
}

}  // namespace sippet