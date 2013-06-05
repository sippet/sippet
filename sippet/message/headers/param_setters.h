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

#ifndef SIPPET_MESSAGE_HEADERS_PARAM_SETTERS_H_
#define SIPPET_MESSAGE_HEADERS_PARAM_SETTERS_H_

#include <cstdlib>
#include <string>
#include "sippet/base/format.h"
#include "sippet/base/raw_ostream.h"

namespace sippet {

template<class T>
inline bool has_qvalue(const T& atom) {
  return atom.param_find("q") != atom.param_end();
}

template<class T>
inline double qvalue(const T& atom) {
  assert(has_qvalue(atom) && "Cannot read qvalue");
  return atof(atom.param_find("q")->second.c_str());
}

template<class T>
inline void set_qvalue(T &atom, double q) {
  std::string buffer;
  raw_string_ostream os(buffer);
  os << format("%.3f", q);
  os.flush();
  while (buffer.size() > 3 && buffer.back() == '0')
    buffer.pop_back();
  atom.param_set("q", buffer);
}

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_PARAM_SETTERS_H_
