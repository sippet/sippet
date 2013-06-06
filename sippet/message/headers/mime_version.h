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
  virtual MimeVersion *DoClone() const {
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

  virtual void print(raw_ostream &os) const {
    os.write_hname("MIME-Version");
    os << major_ << "." << minor_;
  }

private:
  unsigned major_;
  unsigned minor_;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_MIME_VERSION_H_
