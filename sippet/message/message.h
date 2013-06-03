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

#ifndef SIPPET_MESSAGE_MESSAGE_H_
#define SIPPET_MESSAGE_MESSAGE_H_

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

class Message : public base::RefCounted<Message> {
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

  friend class base::RefCounted<Message>;
  virtual ~Message() {}

public:
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
    return headers_.insert(where, header.release());
  }

  //! Insert a header after a specific position in the message.
  iterator insertAfter(iterator where, scoped_ptr<Header> header) {
    return headers_.insertAfter(where, header.release());
  }

  //! Insert a header to the beginning of the message.
  void push_front(scoped_ptr<Header> header) {
    headers_.push_front(header.release());
  }

  //! Insert a header to the end of the message.
  void push_back(scoped_ptr<Header> header) {
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

  //! Print this message to the output.
  virtual void print(raw_ostream &os) const;
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
