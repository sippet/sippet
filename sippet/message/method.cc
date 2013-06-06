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

#include "sippet/message/method.h"
#include "base/string_util.h"
#include <functional>

namespace {

const char *methods[] = {
  "INVITE",
  "ACK",
  "CANCEL",
  "PRACK",
  "BYE",
  "REFER",
  "INFO",
  "UPDATE",
  "OPTIONS",
  "REGISTER",
  "MESSAGE",
  "SUBSCRIBE",
  "NOTIFY",
  "PUBLISH",
  "PULL",
  "PUSH",
  "STORE"
};

const int max_size = sizeof(methods) / sizeof(methods[0]);

} // End of empty namespace

namespace sippet {

Method::NullMethod Method::NullMethod::instance;

const char *Method::KnownMethod::str() {
  int index = static_cast<int>(method_);
  if (index >= max_size)
    return "????"; // should not happen
  return methods[index];
}

Method::MethodImp *Method::coerce(const char *str) {
  std::string input(str);
  input = StringToUpperASCII(input);
  for (int i = 0; i < max_size; ++i) {
    if (input == methods[i])
      return new KnownMethod(static_cast<Type>(i));
  }
  return new UnknownMethod(str);
}

} // End of sippet namespace
