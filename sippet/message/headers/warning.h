/* 
 * Copyright (c) 2013, Guilherme Balena Versiani
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies, 
 * either expressed or implied, of the FreeBSD Project.
 */

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
    os.write_hname("Warning");
    has_multiple::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_WARNING_H_
