// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_TRANSPORT_NETWORK_SETTINGS_H_
#define SIPPET_TRANSPORT_NETWORK_SETTINGS_H_

namespace sippet {

class NetworkSettings {
 private:
  struct Data {
    int reuse_lifetime_;
    int t1_milliseconds_;
    int t2_milliseconds_;
    int t4_milliseconds_;
    bool enable_compact_headers_;
    // Default values
    Data() :
      reuse_lifetime_(60),
      t1_milliseconds_(500),
      t2_milliseconds_(4000),
      t4_milliseconds_(5000),
      enable_compact_headers_(true) {}
  };

  Data data_;
 public:
  NetworkSettings() {}
  NetworkSettings(const NetworkSettings &other)
    : data_(other.data_) {}

  NetworkSettings &operator=(const NetworkSettings &other) {
    data_ = other.data_;
    return *this;
  }

  // Time to keep idle channels
  int reuse_lifetime() const {
    return data_.reuse_lifetime_;
  }
  void set_reuse_lifetime(int value) {
    data_.reuse_lifetime_ = value;
  }

  // RTT estimate
  int t1_milliseconds() const {
    return data_.t1_milliseconds_;
  }
  void set_t1_milliseconds(int value) {
    data_.t1_milliseconds_ = value;
  }

  // Max retransmit interval for non-INVITE requests and INVITE responses
  int t2_milliseconds() const {
    return data_.t2_milliseconds_;
  }
  void set_t2_milliseconds(int value) {
    data_.t2_milliseconds_ = value;
  }

  // Max duration a SIP message will remain in the network
  int t4_milliseconds() const {
    return data_.t4_milliseconds_;
  }
  void set_t4_milliseconds(int value) {
    data_.t4_milliseconds_ = value;
  }

  // Whether to use compact headers in generated messages
  bool enable_compact_headers() const {
    return data_.enable_compact_headers_;
  }
  void set_enable_compact_headers(bool value) {
    data_.enable_compact_headers_ = value;
  }
};

} // End of sippet namespace

#endif // SIPPET_TRANSPORT_NETWORK_SETTINGS_H_
