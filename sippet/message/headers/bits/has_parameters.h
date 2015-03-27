// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_HEADERS_BITS_HAS_PARAMETERS_H_
#define SIPPET_MESSAGE_HEADERS_BITS_HAS_PARAMETERS_H_

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

 protected:
  has_parameters(const has_parameters &other);
  has_parameters &operator=(const has_parameters &other);

 public:
  has_parameters();
  ~has_parameters();

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
    return std::find_if(param_begin(), param_end(), first_equals(key));
  }
  const_param_iterator param_find(const std::string &key) const {
    return std::find_if(param_begin(), param_end(), first_equals(key));
  }

  // set a parameter, or create one if it does not exist
  void param_set(const std::string &key, const std::string &value) {
    assert(!key.empty() && "Key cannot be empty");
    // TODO: value should be unescaped
    param_iterator it = param_find(key);
    if (it == param_end()) {
      params_.push_back(std::make_pair(key, value));
    } else {
      (*it).second = value;
    }
  }

  // print parameters
  void print(raw_ostream &os) const {
    for (const_param_iterator i = param_begin(), ie = param_end(); i != ie; ++i) {
      // TODO: value should be escaped
      os << ";" << i->first;
      if (!i->second.empty())
        os << "=" << i->second;
    }
  }
private:
  std::vector<param_type> params_;

  struct first_equals : std::unary_function<const std::string&,bool> {
    first_equals(const std::string &key) : key_(key) {}
    bool operator ()(const param_type &pair) {
      return pair.first == key_;
    }
   private:
    const std::string &key_;
  };
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_HEADERS_BITS_HAS_PARAMETERS_H_
