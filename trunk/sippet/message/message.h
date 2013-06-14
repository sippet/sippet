// Copyright (c) 2013 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_MESSAGE_MESSAGE_H_
#define SIPPET_MESSAGE_MESSAGE_H_

#include <algorithm>
#include "sippet/base/ilist.h"
#include "sippet/base/casting.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "sippet/message/header.h"

namespace sippet {

// Traits for intrusive list of headers...
template<> struct ilist_traits<sippet::Header>
  : public ilist_default_traits<sippet::Header> {

  /// \brief Return a node that marks the end of a list.
  ///
  /// The sentinel is relative to this instance, so we use a non-static
  /// method.
  sippet::Header *createSentinel() const {
    // Since i(p)lists always publicly derive from their corresponding traits,
    // placing a data member in this class will augment the i(p)list.  But since
    // the NodeTy is expected to be publicly derive from ilist_node<NodeTy>,
    // there is a legal viable downcast from it to NodeTy. We use this trick to
    // superimpose an i(p)list with a "ghostly" NodeTy, which becomes the
    // sentinel. Dereferencing the sentinel is forbidden (save the
    // ilist_node<NodeTy>), so no one will ever notice the superposition.
    return static_cast<sippet::Header*>(&Sentinel);
  }
  static void destroySentinel(sippet::Header*) {}

  sippet::Header *provideInitialHead() const { return createSentinel(); }
  sippet::Header *ensureHead(sippet::Header*) const { return createSentinel(); }
  static void noteHead(sippet::Header*, sippet::Header*) {}
private:
  mutable ilist_half_node<sippet::Header> Sentinel;
};

class raw_ostream;
class Request;
class Response;

class Message
  : public base::RefCountedThreadSafe<Message> {
public:
  typedef iplist<Header> HeaderListType;

  // Header iterators...
  typedef HeaderListType::iterator iterator;
  typedef HeaderListType::const_iterator const_iterator;
  typedef HeaderListType::reverse_iterator reverse_iterator;
  typedef HeaderListType::const_reverse_iterator const_reverse_iterator;
  typedef HeaderListType::reference reference;
  typedef HeaderListType::const_reference const_reference;
  typedef HeaderListType::size_type size_type;

private:
  bool isRequest_;
  HeaderListType headers_;

  DISALLOW_COPY_AND_ASSIGN(Message);

protected:
  Message(bool isRequest) : isRequest_(isRequest) {}

  friend class base::RefCountedThreadSafe<Message>;
  virtual ~Message() {}

public:
  static scoped_refptr<Message> Parse(const std::string &raw_message);

  //! Returns true if the current message is a request.
  bool IsRequest() const { return isRequest_; }

  //! Returns true if the current message is a response.
  bool IsResponse() const { return !isRequest_; }

  //===--------------------------------------------------------------------===//
  /// Header iterator methods
  ///
  iterator       begin()       { return headers_.begin(); }
  const_iterator begin() const { return headers_.begin(); }
  iterator       end  ()       { return headers_.end();   }
  const_iterator end  () const { return headers_.end();   }

  reverse_iterator       rbegin()       { return headers_.rbegin(); }
  const_reverse_iterator rbegin() const { return headers_.rbegin(); }
  reverse_iterator       rend  ()       { return headers_.rend();   }
  const_reverse_iterator rend  () const { return headers_.rend();   }

  size_type      size() const { return headers_.size();  }
  bool          empty() const { return headers_.empty(); }

  reference       front()       { return headers_.front(); }
  const_reference front() const { return headers_.front(); }
  reference       back()        { return headers_.back();  }
  const_reference back() const  { return headers_.back();  }

  //! Insert a header before a specific position in the message.
  iterator insert(iterator where, scoped_ptr<Header> header) {
    return header ? headers_.insert(where, header.release()) : where;
  }

  //! Insert a header after a specific position in the message.
  iterator insertAfter(iterator where, scoped_ptr<Header> header) {
    return header ? headers_.insertAfter(where, header.release()) : where;
  }

  //! Insert a header to the beginning of the message.
  void push_front(scoped_ptr<Header> header) {
    if (header)
      headers_.push_front(header.release());
  }

  //! Insert a header to the end of the message.
  void push_back(scoped_ptr<Header> header) {
    if (header)
      headers_.push_back(header.release());
  }
 
  //! Remove an existing header and return an iterator to the next header.
  iterator erase(iterator position) {
    return headers_.erase(position);
  }

  //! Remove all headers in the given interval.
  void erase(iterator first, iterator last) {
    headers_.erase(first, last);
  }

  //! Clear all headers.
  void clear() {
    headers_.clear();
  }

  //! Remove the first header of the message.
  void pop_front() {
    headers_.pop_front();
  }

  //! Remove the last header of the message.
  void pop_back() {
    headers_.pop_back();
  }

  //! Insert a set of headers to the message before a certain element, in
  //! order. The set of headers must be of type scoped_ptr<Header>.
  template<class InIt>
  void insert(iterator where, InIt first, InIt last) {
    for (; first != last; ++first)
      headers_.insert(where, first->release());
  }

  //! Erase all headers matching a given predicate.
  template<class Pr1> void erase_if(Pr1 pred) {
    headers_.erase_if(pred);
  }

  // Find first header of given type.
  template<class HeaderType>
  iterator find_first() {
    return std::find_if(headers_.begin(), headers_.end(),
      equals<HeaderType>());
  }
  template<class HeaderType>
  const_iterator find_first() const {
    return std::find_if(headers_.begin(), headers_.end(),
      equals<HeaderType>());
  }

  // Find next header of given type.
  template<class HeaderType>
  iterator find_next(iterator where) {
    if (where == end())
      return where;
    return std::find_if(++where, headers_.end(),
      equals<HeaderType>());
  }
  template<class HeaderType>
  const_iterator find_next(iterator where) const {
    if (where == end())
      return where;
    return std::find_if(++where, headers_.end(),
      equals<HeaderType>());
  }

  // Find next header of given type.
  template<class HeaderType>
  HeaderType *get() {
    iterator it = find_first<HeaderType>();
    return it != end() ? dyn_cast<HeaderType>(it) : 0;
  }
  template<class HeaderType>
  const HeaderType *find_next(iterator where) const {
    const_iterator it = find_first<HeaderType>();
    return it != end() ? dyn_cast<HeaderType>(it) : 0;
  }

  //! Print this message to the output.
  virtual void print(raw_ostream &os) const;

private:
  template<class HeaderType>
  struct equals : public std::unary_function<const Header &, bool> {
    bool operator()(const Header &header) {
      return isa<HeaderType>(header);
    }
  };
};

// isa - Provide some specializations of isa so that we don't have to include
// the subtype header files to test to see if the value is a subclass...
//
template <> struct isa_impl<Request, Message> {
  static inline bool doit(const Message &m) {
    return m.IsRequest();
  }
};

template <> struct isa_impl<Response, Message> {
  static inline bool doit(const Message &m) {
    return m.IsResponse();
  }
};

} // End of sippet namespace

#endif // SIPPET_MESSAGE_MESSAGE_H_
