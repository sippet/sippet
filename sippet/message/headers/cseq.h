// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_CSEQ_H_
#define SIPPET_MESSAGE_HEADERS_CSEQ_H_

#include <string>
#include "sippet/message/header.h"
#include "sippet/message/method.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class Cseq :
  public Header {
private:
  DISALLOW_ASSIGN(Cseq);
  Cseq(const Cseq &other)
    : Header(other), sequence_(other.sequence_), method_(other.method_) {}
  virtual Cseq *DoClone() const {
    return new Cseq(*this);
  }
public:
  Cseq() : Header(Header::HDR_CSEQ) {}
  Cseq(unsigned sequence, const Atom<Method> &method)
    : Header(Header::HDR_CSEQ), sequence_(sequence), method_(method) {}

  scoped_ptr<Cseq> Clone() const {
    return scoped_ptr<Cseq>(DoClone());
  }

  unsigned sequence() const { return sequence_; }
  void set_sequence(unsigned sequence) { sequence_ = sequence; }

  Atom<Method> method() const { return method_; }
  void set_method(const Atom<Method> &method) { method_ = method; }

  virtual void print(raw_ostream &os) const {
    os.write_hname("CSeq");
    os << sequence_ << " " << method_;
  }
private:
  unsigned sequence_;
  Atom<Method> method_;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_CSEQ_H_
