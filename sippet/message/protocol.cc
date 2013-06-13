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

#include "sippet/message/protocol.h"

#include <cstring>
#include <algorithm>

#include "base/string_util.h"

namespace sippet {

namespace {
  const char *names[] = {
#define X(method) \
  #method,
  #include "sippet/message/known_protocols.h"
#undef X
    "", // for Unknown
  };

  bool string_less(const char *a, const char *b) {
    return base::strcasecmp(a, b) < 0;
  }
}

const char *AtomTraits<Protocol>::string_of(type t) {
  return names[static_cast<int>(t)];
}

AtomTraits<Protocol>::type
AtomTraits<Protocol>::coerce(const char *str) {
  Protocol::Type type = Protocol::Unknown;
  const char **first = names;
  const char **last = names + ARRAYSIZE(names) - 1; // don't include the last
  const char **found = std::lower_bound(first, last, str, string_less);
  if (found != last
      && base::strcasecmp(*found, str) == 0) {
    type = static_cast<Protocol::Type>(found - first);
  }
  return type;
}

} // End of empty namespace
