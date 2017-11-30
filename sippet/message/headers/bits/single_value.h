// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_BITS_SINGLE_VALUE_H_
#define SIPPET_MESSAGE_HEADERS_BITS_SINGLE_VALUE_H_

namespace sippet {

template<typename T>
class single_value {
public:
  typedef T value_type;

  single_value() {}
  single_value(const value_type &value) : value_(value) {}
  single_value(const single_value &other) : value_(other.value_) {}
  ~single_value() {}

  single_value &operator=(const single_value &other) {
    value_ = other.value_;
    return *this;
  }

  void set_value(const value_type &value) { value_ = value; }
  value_type value() const { return value_; }

  void print(raw_ostream &os) const {
    os << value();
  }

private:
  value_type value_;
};

template<typename T> inline
raw_ostream &operator << (raw_ostream &os, const single_value<T> &v) {
  v.print(os);
  return os;
}

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_BITS_SINGLE_VALUE_H_
