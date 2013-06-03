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

#ifndef SIPPET_MESSAGE_HEADERS_HAS_PARAMETERS_H_
#define SIPPET_MESSAGE_HEADERS_HAS_PARAMETERS_H_

#include <vector>
#include <utility>
#include <algorithm>
#include <string>
#include <cassert>
#include "sippet/base/raw_ostream.h"

namespace sippet {

class has_parameters {
public:
  typedef std::pair<std::string, std::string> param_type;
  typedef std::vector<param_type>::iterator param_iterator;
  typedef std::vector<param_type>::const_iterator const_param_iterator;

  has_parameters() {}
  ~has_parameters() {}

  // Iterator creation methods.
  param_iterator param_begin()             { return params_.begin(); }
  const_param_iterator param_begin() const { return params_.begin(); }
  param_iterator param_end()               { return params_.end();   }
  const_param_iterator param_end() const   { return params_.end();   }

  // Miscellaneous inspection routines.
  bool param_empty() const { return params_.empty(); }

  // erase - remove a node from the controlled sequence... and delete it.
  param_iterator param_erase(param_iterator where) {
    return params_.erase(where);
  }

  // clear everything
  void param_clear() { params_.clear(); }

  // find an existing parameter
  param_iterator param_find(const std::string &key) {
    struct _pred {
      _pred(const std::string &key) : key_(key) {}
      bool operator ()(const param_type &pair) {
        return pair.first == key_;
      }
     private:
      const std::string &key_;
    };
    return std::find_if(param_begin(), param_end(), _pred(key));
  }

  // set a parameter, or create one if it does not exist
  void param_set(const std::string &key, const std::string &value) {
    assert(!key.empty() && "Key cannot be empty");
    // TODO: value should be unescaped
    param_iterator it = param_find(key);
    if (it == param_end()) {
      params_.push_back(std::make_pair(key, value));
    }
    else {
      (*it).second = value;
    }
  }

  // print parameters
  void print(raw_ostream &os) const {
    for (const_param_iterator i = param_begin(), ie = param_end(); i != ie; ++i) {
      // TODO: value should be escaped
      os << ";" << i->first << "=" << i->second;
    }
  }
private:
  std::vector<param_type> params_;
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_HAS_PARAMETERS_H_

/* Modeline for vim: set tw=79 et ts=4: */

