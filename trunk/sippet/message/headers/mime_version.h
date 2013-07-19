// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_MIME_VERSION_H_
#define SIPPET_MESSAGE_HEADERS_MIME_VERSION_H_

#include <string>
#include <cmath>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/format.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class MimeVersion :
  public Header {
private:
  DISALLOW_ASSIGN(MimeVersion);
  MimeVersion(const MimeVersion &other)
    : Header(other), major_(other.major_), minor_(other.minor_) {}
  virtual MimeVersion *DoClone() const OVERRIDE {
    return new MimeVersion(*this);
  }
public:
  MimeVersion() : Header(Header::HDR_MIME_VERSION), major_(0), minor_(0) {}
  MimeVersion(unsigned major, unsigned minor)
    : Header(Header::HDR_MIME_VERSION), major_(major), minor_(minor) {}

  scoped_ptr<MimeVersion> Clone() const {
    return scoped_ptr<MimeVersion>(DoClone());
  }

  void set_major(unsigned major) { major_ = major; }
  double major() { return major_; }

  void set_minor(unsigned minor) { minor_ = minor; }
  double minor() { return minor_; }

  virtual void print(raw_ostream &os) const OVERRIDE {
    Header::print(os);
    os << major_ << "." << minor_;
  }

private:
  unsigned major_;
  unsigned minor_;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_MIME_VERSION_H_
