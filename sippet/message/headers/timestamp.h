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

#ifndef SIPPET_MESSAGE_HEADERS_TIMESTAMP_H_
#define SIPPET_MESSAGE_HEADERS_TIMESTAMP_H_

#include <string>
#include <cmath>
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/format.h"
#include "sippet/base/raw_ostream.h"
#include "base/strings/string_number_conversions.h"

namespace sippet {

class Timestamp :
  public Header {
private:
  Timestamp(const Timestamp &other)
    : Header(other), timestamp_(other.timestamp_), delay_(other.delay_) {}
  Timestamp &operator=(const Timestamp &other);
  virtual Timestamp *DoClone() const {
    return new Timestamp(*this);
  }
public:
  Timestamp() : Header(Header::HDR_TIMESTAMP), timestamp_(0), delay_(0) {}
  Timestamp(const double &timestamp, const double &delay=.0)
    : Header(Header::HDR_TIMESTAMP), timestamp_(timestamp), delay_(delay) {}

  scoped_ptr<Timestamp> Clone() const {
    return scoped_ptr<Timestamp>(DoClone());
  }

  void set_timestamp(double timestamp) { timestamp_ = timestamp; }
  double timestamp() { return timestamp_; }

  void set_delay(double delay) { delay_ = delay; }
  double delay() { return delay_; }

  virtual void print(raw_ostream &os) const {
    os.write_hname("Timestamp");
    print_double(os, timestamp_);
    if (delay_ != 0) {
      os << " ";
      print_double(os, delay_);
    }
  }

private:
  double timestamp_;
  double delay_;

  static void print_double(raw_ostream &os, double v) {
    double i;
    double frac = modf(v, &i);
    if (frac == 0)
      os << static_cast<int>(i);
    else
      os << base::DoubleToString(v);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_TIMESTAMP_H_
