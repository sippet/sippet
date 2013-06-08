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

#ifndef SIPPET_MESSAGE_RESPONSE_H_
#define SIPPET_MESSAGE_RESPONSE_H_

#include "sippet/message/message.h"
#include "sippet/message/version.h"

namespace sippet {

class Response :
  public Message {
private:
  DISALLOW_COPY_AND_ASSIGN(Response);

private:
  virtual ~Response() {}

public:
  // This constructor will take a default reason phrase for you
  Response(int response_code,
           const Version &version = Version(2,0))
    : Message(false) { /* TODO */ }

  // Just in case you want to define the reason phrase
  Response(int response_code,
           const std::string &reason_phrase,
           const Version &version = Version(2,0))
    : Message(false), response_code_(response_code),
      reason_phrase_(reason_phrase), version_(version) {}

  int response_code() const { return response_code_; }
  void set_response_code(int response_code) {
    response_code_ = response_code;
  }

  std::string reason_phrase() const { return reason_phrase_; }
  void set_reason_phrase(const std::string &reason_phrase) {
    reason_phrase_ = reason_phrase;
  }

  Version version() const { return version_; }
  void set_version(const Version &version) {
    version_ = version;
  }

private:
  Version version_;
  int response_code_;
  std::string reason_phrase_;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_RESPONSE_H_
