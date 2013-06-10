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

#ifndef SIPPET_MESSAGE_PARSER_TOKENIZER_H_
#define SIPPET_MESSAGE_PARSER_TOKENIZER_H_

#include <string>
#include "base/string_piece.h"

namespace sippet {

class Tokenizer {
public:
  Tokenizer(std::string::const_iterator string_begin,
            std::string::const_iterator string_end)
    : current_(string_begin), end_(string_end) {}

  std::string::const_iterator Skip(const base::StringPiece &chars) {
    for (; current_ != end_; ++current_) {
      if (chars.find(*current_) == std::string::npos)
        break;
    }
    return current_;
  }

  std::string::const_iterator SkipNotIn(const base::StringPiece &chars) {
    for (; current_ != end_; ++current_) {
      if (chars.find(*current_) != std::string::npos)
        break;
    }
    return current_;
  }

  std::string::const_iterator SkipTo(char c) {
    for (; current_ != end_; ++current_) {
      if (c == *current_)
        break;
    }
    return current_;
  }

  std::string::const_iterator Skip() {
    if (current_ != end_)
      ++current_;
    return current_;
  }

  bool EndOfInput() const {
    return current_ == end_;
  }

  std::string::const_iterator current() const { return current_; }
  std::string::const_iterator end() const { return end_; }

private:
  std::string::const_iterator current_;
  std::string::const_iterator end_;
};

} // namespace sippet

#endif // SIPPET_MESSAGE_PARSER_TOKENIZER_H_
