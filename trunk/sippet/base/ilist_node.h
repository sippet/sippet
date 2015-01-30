/*
 * University of Illinois/NCSA
 * Open Source License
 * 
 * Copyright (c) 2003-2013 University of Illinois at Urbana-Champaign.
 * All rights reserved.
 * 
 * Developed by:
 *     LLVM Team
 *     University of Illinois at Urbana-Champaign
 *     http://llvm.org
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal with
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * 
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimers.
 * 
 *     * Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimers in the
 *       documentation and/or other materials provided with the distribution.
 * 
 *     * Neither the names of the LLVM Team, University of Illinois at
 *       Urbana-Champaign, nor the names of its contributors may be used to
 *       endorse or promote products derived from this Software without specific
 *       prior written permission.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
 * SOFTWARE.
 */

#ifndef SIPPET_BASE_ILIST_NODE_H_
#define SIPPET_BASE_ILIST_NODE_H_

namespace sippet {

template<typename NodeTy>
struct ilist_traits;

/// ilist_half_node - Base class that provides prev services for sentinels.
///
template<typename NodeTy>
class ilist_half_node {
  friend struct ilist_traits<NodeTy>;
  NodeTy *Prev;
protected:
  NodeTy *getPrev() { return Prev; }
  const NodeTy *getPrev() const { return Prev; }
  void setPrev(NodeTy *P) { Prev = P; }
  ilist_half_node() : Prev(0) {}
};

template<typename NodeTy>
struct ilist_nextprev_traits;

/// ilist_node - Base class that provides next/prev services for nodes
/// that use ilist_nextprev_traits or ilist_default_traits.
///
template<typename NodeTy>
class ilist_node : private ilist_half_node<NodeTy> {
  friend struct ilist_nextprev_traits<NodeTy>;
  friend struct ilist_traits<NodeTy>;
  NodeTy *Next;
  NodeTy *getNext() { return Next; }
  const NodeTy *getNext() const { return Next; }
  void setNext(NodeTy *N) { Next = N; }
protected:
  ilist_node() : Next(0) {}

public:
  /// @name Adjacent Node Accessors
  /// @{

  /// \brief Get the previous node, or 0 for the list head.
  NodeTy *getPrevNode() {
    NodeTy *Prev = this->getPrev();

    // Check for sentinel.
    if (!Prev->getNext())
      return 0;

    return Prev;
  }

  /// \brief Get the previous node, or 0 for the list head.
  const NodeTy *getPrevNode() const {
    const NodeTy *Prev = this->getPrev();

    // Check for sentinel.
    if (!Prev->getNext())
      return 0;

    return Prev;
  }

  /// \brief Get the next node, or 0 for the list tail.
  NodeTy *getNextNode() {
    NodeTy *Next = getNext();

    // Check for sentinel.
    if (!Next->getNext())
      return 0;

    return Next;
  }

  /// \brief Get the next node, or 0 for the list tail.
  const NodeTy *getNextNode() const {
    const NodeTy *Next = getNext();

    // Check for sentinel.
    if (!Next->getNext())
      return 0;

    return Next;
  }

  /// @}
};

} // End of sippet namespace

#endif // SIPPET_BASE_ILIST_NODE_H_
