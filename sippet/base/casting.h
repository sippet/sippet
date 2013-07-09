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

#ifndef SIPPET_BASE_CASTING_H_
#define SIPPET_BASE_CASTING_H_

#include "sippet/base/type_traits.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/ref_counted.h"
#include <cassert>

namespace sippet {

//===----------------------------------------------------------------------===//
//                          isa<x> Support Templates
//===----------------------------------------------------------------------===//

// Define a template that can be specialized by smart pointers to reflect the
// fact that they are automatically dereferenced, and are not involved with the
// template selection process...  the default implementation is a noop.
//
template<typename From> struct simplify_type {
  typedef       From SimpleType;        // The real type this represents...

  // An accessor to get the real value...
  static SimpleType &getSimplifiedValue(From &Val) { return Val; }
};

template<typename From> struct simplify_type<const From> {
  typedef typename simplify_type<From>::SimpleType NonConstSimpleType;
  typedef typename add_const_past_pointer<NonConstSimpleType>::type
    SimpleType;
  typedef typename add_lvalue_reference_if_not_pointer<SimpleType>::type
    RetType;
  static RetType getSimplifiedValue(const From& Val) {
    return simplify_type<From>::getSimplifiedValue(const_cast<From&>(Val));
  }
};

// The core of the implementation of isa<X> is here; To and From should be
// the names of classes.  This template can be specialized to customize the
// implementation of isa<> without rewriting it from scratch.
template <typename To, typename From, typename Enabler = void>
struct isa_impl {
  static inline bool doit(const From &Val) {
    return To::classof(&Val);
  }
};

/// \brief Always allow upcasts, and perform no dynamic check for them.
template <typename To, typename From>
struct isa_impl<To, From,
                typename enable_if<
                  sippet::is_base_of<To, From>
                >::type
               > {
  static inline bool doit(const From &) { return true; }
};

template <typename To, typename From> struct isa_impl_cl {
  static inline bool doit(const From &Val) {
    return isa_impl<To, From>::doit(Val);
  }
};

template <typename To, typename From> struct isa_impl_cl<To, const From> {
  static inline bool doit(const From &Val) {
    return isa_impl<To, From>::doit(Val);
  }
};

template <typename To, typename From> struct isa_impl_cl<To, From*> {
  static inline bool doit(const From *Val) {
    assert(Val && "isa<> used on a null pointer");
    return isa_impl<To, From>::doit(*Val);
  }
};

template <typename To, typename From> struct isa_impl_cl<To, From*const> {
  static inline bool doit(const From *Val) {
    assert(Val && "isa<> used on a null pointer");
    return isa_impl<To, From>::doit(*Val);
  }
};

template <typename To, typename From> struct isa_impl_cl<To, const From*> {
  static inline bool doit(const From *Val) {
    assert(Val && "isa<> used on a null pointer");
    return isa_impl<To, From>::doit(*Val);
  }
};

template <typename To, typename From> struct isa_impl_cl<To, const From*const> {
  static inline bool doit(const From *Val) {
    assert(Val && "isa<> used on a null pointer");
    return isa_impl<To, From>::doit(*Val);
  }
};

template<typename To, typename From, typename SimpleFrom>
struct isa_impl_wrap {
  // When From != SimplifiedType, we can simplify the type some more by using
  // the simplify_type template.
  static bool doit(const From &Val) {
    return isa_impl_wrap<To, SimpleFrom,
      typename simplify_type<SimpleFrom>::SimpleType>::doit(
                          simplify_type<const From>::getSimplifiedValue(Val));
  }
};

template<typename To, typename FromTy>
struct isa_impl_wrap<To, FromTy, FromTy> {
  // When From == SimpleType, we are as simple as we are going to get.
  static bool doit(const FromTy &Val) {
    return isa_impl_cl<To,FromTy>::doit(Val);
  }
};

// isa<X> - Return true if the parameter to the template is an instance of the
// template type argument.  Used like this:
//
//  if (isa<Type>(myVal)) { ... }
//
template <class X, class Y>
inline bool isa(const Y &Val) {
  return isa_impl_wrap<X, const Y,
                       typename simplify_type<const Y>::SimpleType>::doit(Val);
}
template <class X, class Y>
inline bool isa(const scoped_ptr<Y> &Val) {
  return isa_impl_wrap<X, const Y*,
                       typename simplify_type<const Y*>::SimpleType>::doit(Val.get());
}
template <class X, class Y>
inline bool isa(const scoped_refptr<Y> &Val) {
  return isa_impl_wrap<X, const Y*,
                       typename simplify_type<const Y*>::SimpleType>::doit(Val.get());
}

//===----------------------------------------------------------------------===//
//                          cast<x> Support Templates
//===----------------------------------------------------------------------===//

template<class To, class From> struct cast_retty;


// Calculate what type the 'cast' function should return, based on a requested
// type of To and a source type of From.
template<class To, class From> struct cast_retty_impl {
  typedef To& ret_type;         // Normal case, return Ty&
};
template<class To, class From> struct cast_retty_impl<To, const From> {
  typedef const To &ret_type;   // Normal case, return Ty&
};

template<class To, class From> struct cast_retty_impl<To, From*> {
  typedef To* ret_type;         // Pointer arg case, return Ty*
};

template<class To, class From> struct cast_retty_impl<To, const From*> {
  typedef const To* ret_type;   // Constant pointer arg case, return const Ty*
};

template<class To, class From> struct cast_retty_impl<To, const From*const> {
  typedef const To* ret_type;   // Constant pointer arg case, return const Ty*
};

template<class To, class From> struct cast_retty_impl<To, scoped_ptr<From> > {
  typedef To *ret_type;         // Reference is not passed
};

template<class To, class From>
struct cast_retty_impl<To, scoped_refptr<From> > {
  typedef scoped_refptr<To> ret_type; // Return new ref
};

template<class To, class From>
struct cast_retty_impl<To, const scoped_refptr<From> > {
  typedef scoped_refptr<To> ret_type; // Return new ref
};


template<class To, class From, class SimpleFrom>
struct cast_retty_wrap {
  // When the simplified type and the from type are not the same, use the type
  // simplifier to reduce the type, then reuse cast_retty_impl to get the
  // resultant type.
  typedef typename cast_retty<To, SimpleFrom>::ret_type ret_type;
};

template<class To, class FromTy>
struct cast_retty_wrap<To, FromTy, FromTy> {
  // When the simplified type is equal to the from type, use it directly.
  typedef typename cast_retty_impl<To,FromTy>::ret_type ret_type;
};

template<class To, class From>
struct cast_retty {
  typedef typename cast_retty_wrap<To, From,
                   typename simplify_type<From>::SimpleType>::ret_type ret_type;
};

// Ensure the non-simple values are converted using the simplify_type template
// that may be specialized by smart pointers...
//
template<class To, class From, class SimpleFrom> struct cast_convert_val {
  // This is not a simple type, use the template to simplify it...
  static typename cast_retty<To, From>::ret_type doit(From &Val) {
    return cast_convert_val<To, SimpleFrom,
      typename simplify_type<SimpleFrom>::SimpleType>::doit(
                          simplify_type<From>::getSimplifiedValue(Val));
  }
};

template<class To, class FromTy> struct cast_convert_val<To,FromTy,FromTy> {
  // This _is_ a simple type, just cast it.
  static typename cast_retty<To, FromTy>::ret_type doit(const FromTy &Val) {
    typename cast_retty<To, FromTy>::ret_type Res2
     = (typename cast_retty<To, FromTy>::ret_type)const_cast<FromTy&>(Val);
    return Res2;
  }
};



// cast<X> - Return the argument parameter cast to the specified type.  This
// casting operator asserts that the type is correct, so it does not return null
// on failure.  It does not allow a null argument (use cast_or_null for that).
// It is typically used like this:
//
//  cast<Instruction>(myVal)->getParent()
//
template <class X, class Y>
inline typename cast_retty<X, const Y>::ret_type cast(const Y &Val) {
  assert(isa<X>(Val) && "cast<Ty>() argument of incompatible type!");
  return cast_convert_val<X, const Y,
                        typename simplify_type<const Y>::SimpleType>::doit(Val);
}

template <class X, class Y>
inline typename cast_retty<X, Y>::ret_type cast(Y &Val) {
  assert(isa<X>(Val) && "cast<Ty>() argument of incompatible type!");
  return cast_convert_val<X, Y,
                          typename simplify_type<Y>::SimpleType>::doit(Val);
}

template <class X, class Y>
inline typename enable_if<
  is_same<Y, typename simplify_type<Y>::SimpleType>,
  typename cast_retty<X, Y*>::ret_type
>::type cast(Y *Val) {
  assert(isa<X>(Val) && "cast<Ty>() argument of incompatible type!");
  return cast_convert_val<X, Y*,
                          typename simplify_type<Y*>::SimpleType>::doit(Val);
}

template <class X, class Y>
inline typename cast_retty<X, const scoped_ptr<Y> >::ret_type
  cast(const scoped_ptr<Y> &Val) {
  assert(isa<X>(Val) && "cast<Ty>() argument of incompatible type!");
  return cast<X>(Val.get());
}

template <class X, class Y>
inline typename cast_retty<X, const scoped_refptr<Y> >::ret_type
  cast(const scoped_refptr<Y> &Val) {
  assert(isa<X>(Val) && "cast<Ty>() argument of incompatible type!");
  return cast<X>(Val.get());
}

// cast_or_null<X> - Functionally identical to cast, except that a null value is
// accepted.
//
template <class X, class Y>
inline typename cast_retty<X, Y*>::ret_type cast_or_null(Y *Val) {
  if (Val == 0) return 0;
  assert(isa<X>(Val) && "cast_or_null<Ty>() argument of incompatible type!");
  return cast<X>(Val);
}


// dyn_cast<X> - Return the argument parameter cast to the specified type.  This
// casting operator returns null if the argument is of the wrong type, so it can
// be used to test for a type as well as cast if successful.  This should be
// used in the context of an if statement like this:
//
//  if (const Instruction *I = dyn_cast<Instruction>(myVal)) { ... }
//

template <class X, class Y>
inline typename cast_retty<X, const Y>::ret_type dyn_cast(const Y &Val) {
  return isa<X>(Val) ? cast<X>(Val) : 0;
}

template <class X, class Y>
inline typename cast_retty<X, Y>::ret_type dyn_cast(Y &Val) {
  return isa<X>(Val) ? cast<X>(Val) : 0;
}

template <class X, class Y>
inline typename cast_retty<X, scoped_ptr<Y> >::ret_type
  dyn_cast(scoped_ptr<Y> &Val) {
  return isa<X>(Val) ? cast<X>(Val.get()) : 0;
}

template <class X, class Y>
inline typename cast_retty<X, scoped_refptr<Y> >::ret_type
  dyn_cast(scoped_refptr<Y> &Val) {
  return isa<X>(Val) ? cast<X>(Val.get()) : 0;
}

template <class X, class Y>
inline typename cast_retty<X, scoped_refptr<Y> >::ret_type
  dyn_cast(const scoped_refptr<Y> &Val) {
  return isa<X>(Val) ? cast<X>(Val.get()) : 0;
}

template <class X, class Y>
inline typename enable_if<
  is_same<Y, typename simplify_type<Y>::SimpleType>,
  typename cast_retty<X, Y*>::ret_type
>::type dyn_cast(Y *Val) {
  return isa<X>(Val) ? cast<X>(Val) : 0;
}

// dyn_cast_or_null<X> - Functionally identical to dyn_cast, except that a null
// value is accepted.
//
template <class X, class Y>
inline typename cast_retty<X, Y*>::ret_type dyn_cast_or_null(Y *Val) {
  return (Val && isa<X>(Val)) ? cast<X>(Val) : 0;
}

} // End of sippet namespace

#endif // SIPPET_BASE_CASTING_H_
