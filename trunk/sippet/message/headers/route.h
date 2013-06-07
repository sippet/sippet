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

#ifndef SIPPET_MESSAGE_HEADERS_ROUTE_H_
#define SIPPET_MESSAGE_HEADERS_ROUTE_H_

#include "sippet/message/headers/contact.h"

namespace sippet {

class RouteParam :
  public ContactBase {
public:
  RouteParam() {}
  RouteParam(const RouteParam &other)
    : ContactBase(other) {}
  explicit RouteParam(const GURL &address,
                      const std::string &displayName="")
    : ContactBase(address, displayName) {}

  ~RouteParam() {}

  RouteParam &operator=(const RouteParam &other) {
    ContactBase::operator=(other);
    return *this;
  }
};

class Route :
  public Header,
  public has_multiple<RouteParam> {
private:
  DISALLOW_ASSIGN(Route);
  Route(const Route &other)
    : Header(other), has_multiple(other) {}
  virtual Route *DoClone() const {
    return new Route(*this);
  }
public:
  Route()
    : Header(Header::HDR_ROUTE) {}
  Route(const RouteParam &param)
    : Header(Header::HDR_ROUTE) { push_back(param); }

  scoped_ptr<Route> Clone() const {
    return scoped_ptr<Route>(DoClone());
  }

  virtual void print(raw_ostream &os) const {
    os.write_hname("Route");
    has_multiple::print(os);
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_ROUTE_H_
