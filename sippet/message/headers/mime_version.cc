// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/mime_version.h"

namespace sippet {

MimeVersion::MimeVersion()
  : Header(Header::HDR_MIME_VERSION), major_(0), minor_(0) {
}

MimeVersion::MimeVersion(unsigned major, unsigned minor)
  : Header(Header::HDR_MIME_VERSION), major_(major), minor_(minor) {
}

MimeVersion::MimeVersion(const MimeVersion &other)
  : Header(other), major_(other.major_), minor_(other.minor_) {
}

MimeVersion::~MimeVersion() {
}

MimeVersion *MimeVersion::DoClone() const {
  return new MimeVersion(*this);
}

void MimeVersion::print(raw_ostream &os) const {
  Header::print(os);
  os << major_ << "." << minor_;
}

}  // namespace sippet
