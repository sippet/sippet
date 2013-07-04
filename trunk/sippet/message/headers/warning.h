// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_WARNING_H_
#define SIPPET_MESSAGE_HEADERS_WARNING_H_

#include "sippet/message/header.h"
#include "sippet/message/headers/bits/has_multiple.h"
#include "sippet/base/format.h"

namespace sippet {

class WarnParam {
public:
  WarnParam() : warn_code_(0) {}
  WarnParam(const WarnParam &other)
    : warn_code_(other.warn_code_), warn_agent_(other.warn_agent_),
      warn_text_(other.warn_text_) {}
  WarnParam(unsigned warn_code,
            const std::string &warn_agent,
            const std::string &warn_text)
    : warn_code_(warn_code), warn_agent_(warn_agent),
      warn_text_(warn_text) {}

  ~WarnParam() {}

  WarnParam &operator=(const WarnParam &other) {
    warn_code_ = other.warn_code_;
    warn_agent_ = other.warn_agent_;
    warn_text_ = other.warn_text_;
    return *this;
  }

  unsigned warn_code() const { return warn_code_; }
  void set_warn_code(unsigned warn_code) { warn_code_ = warn_code; }

  std::string warn_agent() const { return warn_agent_; }
  void set_warn_agent(const std::string &warn_agent) {
    warn_agent_ = warn_agent;
  }

  std::string warn_text() const { return warn_text_; }
  void set_warn_text(const std::string &warn_text) {
    warn_text_ = warn_text;
  }

  void print(raw_ostream &os) const {
    os << warn_code_ << " " << warn_agent_ << " \"" << warn_text_ << "\"";
  }
private:
  unsigned warn_code_;
  std::string warn_agent_;
  std::string warn_text_;
};

inline
raw_ostream &operator<<(raw_ostream &os, const WarnParam &p) {
  p.print(os);
  return os;
}

class Warning :
  public Header,
  public has_multiple<WarnParam> {
private:
  DISALLOW_ASSIGN(Warning);
  Warning(const Warning &other)
    : Header(other), has_multiple(other) {}
  virtual Warning *DoClone() const {
    return new Warning(*this);
  }
public:
  Warning()
    : Header(Header::HDR_WARNING) {}
  Warning(const WarnParam &param)
    : Header(Header::HDR_WARNING) { push_back(param); }

  scoped_ptr<Warning> Clone() const {
    return scoped_ptr<Warning>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    Header::print(os);
    has_multiple::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_WARNING_H_
