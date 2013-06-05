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

#ifndef SIPPET_MESSAGE_HEADERS_DATE_H_
#define SIPPET_MESSAGE_HEADERS_DATE_H_

#include "base/time.h"
#include "sippet/message/header.h"
#include "sippet/message/headers/bits/single_value.h"
#include "sippet/base/format.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

class Date :
  public Header,
  public single_value<base::Time> {
private:
  Date(const Date &other) : Header(other), single_value(other) {}
  Date &operator=(const Date &other);
  virtual Date *DoClone() const {
    return new Date(*this);
  }
public:
  Date() : Header(Header::HDR_DATE) {}
  Date(const single_value::value_type &date)
    : Header(Header::HDR_DATE), single_value(date) {}

  scoped_ptr<Date> Clone() const {
    return scoped_ptr<Date>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    const char *wkday[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    const char *month[] = {
      "Jan", "Feb", "Mar", "Apr", "May", "Jun",
      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    base::Time::Exploded exploded;
    value().UTCExplode(&exploded);

    os.write_hname("Date");
    os << wkday[exploded.day_of_week]
       << ", "
       << format("%2d %s %d", exploded.day_of_month, month[exploded.month-1], exploded.year)
       << " "
       << format("%2d:%2d:%2d", exploded.hour, exploded.minute, exploded.second)
       << " GMT";
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_DATE_H_