// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/transport_layer.h"

#include "sippet/message/message.h"

namespace sippet {

TransportLayer::~TransportLayer() {}

// static
std::unique_ptr<TransportLayer> TransportLayer::Create() {
  return nullptr;
}

}  // namespace sippet