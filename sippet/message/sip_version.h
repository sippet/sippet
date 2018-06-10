// Copyright (c) 2013-2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPPET_MESSAGE_SIP_VERSION_H_
#define SIPPPET_MESSAGE_SIP_VERSION_H_

#include <stdint.h>

namespace sippet {

// Wrapper for a SIP (major,minor) version pair.
class SipVersion {
 public:
  // Default constructor (major=0, minor=0).
  SipVersion() : value_(0) { }

  // Build from unsigned major/minor pair.
  SipVersion(uint16_t major, uint16_t minor) : value_(major << 16 | minor) {}

  // Major version number.
  uint16_t major_value() const { return value_ >> 16; }

  // Minor version number.
  uint16_t minor_value() const { return value_ & 0xffff; }

  // Overloaded operators:

  bool operator==(const SipVersion& v) const {
    return value_ == v.value_;
  }
  bool operator!=(const SipVersion& v) const {
    return value_ != v.value_;
  }
  bool operator>(const SipVersion& v) const {
    return value_ > v.value_;
  }
  bool operator>=(const SipVersion& v) const {
    return value_ >= v.value_;
  }
  bool operator<(const SipVersion& v) const {
    return value_ < v.value_;
  }
  bool operator<=(const SipVersion& v) const {
    return value_ <= v.value_;
  }

 private:
  uint32_t value_;  // Packed as <major>:<minor>
};

}  // namespace sippet

#endif  // SIPPET_MESSAGE_SIP_VERSION_H_
