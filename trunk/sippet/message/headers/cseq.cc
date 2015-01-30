// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/message/headers/cseq.h"

namespace sippet {

Cseq::Cseq()
  : Header(Header::HDR_CSEQ) {
}

Cseq::Cseq(unsigned sequence, const Method &method)
  : Header(Header::HDR_CSEQ), sequence_(sequence), method_(method) {
}

Cseq::Cseq(const Cseq &other)
  : Header(other), sequence_(other.sequence_), method_(other.method_) {
}

Cseq::~Cseq() {
}

Cseq *Cseq::DoClone() const {
  return new Cseq(*this);
}

void Cseq::print(raw_ostream &os) const {
  Header::print(os);
  os << sequence_ << " " << method_;
}

} // End of sippet namespace
