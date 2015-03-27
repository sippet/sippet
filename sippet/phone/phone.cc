// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/phone.h"

namespace sippet {
namespace phone {

Settings::Settings() :
  disable_encryption_(false),
  disable_sctp_data_channels_(false) {
}

Settings::~Settings() {
}

} // namespace phone
} // namespace sippet
