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

#ifndef SIPPET_BASE_FORMAT_H_
#define SIPPET_BASE_FORMAT_H_

#include <cassert>
#include <cstdio>
#include "base/strings/string_util.h"

namespace sippet {

/// format_object_base - This is a helper class used for handling formatted
/// output.  It is the abstract base class of a templated derived class.
class format_object_base {
protected:
  const char *Fmt;
  virtual void home(); // Out of line virtual method.

  /// snprint - Call snprintf() for this object, on the given buffer and size.
  virtual int snprint(char *Buffer, unsigned BufferSize) const = 0;

public:
  explicit format_object_base(const char *fmt) : Fmt(fmt) {}
  virtual ~format_object_base() {}

  /// print - Format the object into the specified buffer.  On success, this
  /// returns the length of the formatted string.  If the buffer is too small,
  /// this returns a length to retry with, which will be larger than BufferSize.
  unsigned print(char *Buffer, unsigned BufferSize) const {
    assert(BufferSize && "Invalid buffer size!");

    // Print the string, leaving room for the terminating null.
    int N = snprint(Buffer, BufferSize);

    // VC++ and old GlibC return negative on overflow, just double the size.
    if (N < 0)
      return BufferSize*2;

    // Other impls yield number of bytes needed, not including the final '\0'.
    if (unsigned(N) >= BufferSize)
      return N+1;

    // Otherwise N is the length of output (not including the final '\0').
    return N;
  }
};

/// format_object1 - This is a templated helper class used by the format
/// function that captures the object to be formated and the format string. When
/// actually printed, this synthesizes the string into a temporary buffer
/// provided and returns whether or not it is big enough.
template <typename T>
class format_object1 : public format_object_base {
  T Val;
public:
  format_object1(const char *fmt, const T &val)
    : format_object_base(fmt), Val(val) {
  }

  virtual int snprint(char *Buffer, unsigned BufferSize) const {
    return base::snprintf(Buffer, BufferSize, Fmt, Val);
  }
};

/// format_object2 - This is a templated helper class used by the format
/// function that captures the object to be formated and the format string. When
/// actually printed, this synthesizes the string into a temporary buffer
/// provided and returns whether or not it is big enough.
template <typename T1, typename T2>
class format_object2 : public format_object_base {
  T1 Val1;
  T2 Val2;
public:
  format_object2(const char *fmt, const T1 &val1, const T2 &val2)
  : format_object_base(fmt), Val1(val1), Val2(val2) {
  }

  virtual int snprint(char *Buffer, unsigned BufferSize) const {
    return base::snprintf(Buffer, BufferSize, Fmt, Val1, Val2);
  }
};

/// format_object3 - This is a templated helper class used by the format
/// function that captures the object to be formated and the format string. When
/// actually printed, this synthesizes the string into a temporary buffer
/// provided and returns whether or not it is big enough.
template <typename T1, typename T2, typename T3>
class format_object3 : public format_object_base {
  T1 Val1;
  T2 Val2;
  T3 Val3;
public:
  format_object3(const char *fmt, const T1 &val1, const T2 &val2,const T3 &val3)
    : format_object_base(fmt), Val1(val1), Val2(val2), Val3(val3) {
  }

  virtual int snprint(char *Buffer, unsigned BufferSize) const {
    return base::snprintf(Buffer, BufferSize, Fmt, Val1, Val2, Val3);
  }
};

/// format_object4 - This is a templated helper class used by the format
/// function that captures the object to be formated and the format string. When
/// actually printed, this synthesizes the string into a temporary buffer
/// provided and returns whether or not it is big enough.
template <typename T1, typename T2, typename T3, typename T4>
class format_object4 : public format_object_base {
  T1 Val1;
  T2 Val2;
  T3 Val3;
  T4 Val4;
public:
  format_object4(const char *fmt, const T1 &val1, const T2 &val2,
                 const T3 &val3, const T4 &val4)
    : format_object_base(fmt), Val1(val1), Val2(val2), Val3(val3), Val4(val4) {
  }

  virtual int snprint(char *Buffer, unsigned BufferSize) const {
    return base::snprintf(Buffer, BufferSize, Fmt, Val1, Val2, Val3, Val4);
  }
};

/// format_object5 - This is a templated helper class used by the format
/// function that captures the object to be formated and the format string. When
/// actually printed, this synthesizes the string into a temporary buffer
/// provided and returns whether or not it is big enough.
template <typename T1, typename T2, typename T3, typename T4, typename T5>
class format_object5 : public format_object_base {
  T1 Val1;
  T2 Val2;
  T3 Val3;
  T4 Val4;
  T5 Val5;
public:
  format_object5(const char *fmt, const T1 &val1, const T2 &val2,
                 const T3 &val3, const T4 &val4, const T5 &val5)
    : format_object_base(fmt), Val1(val1), Val2(val2), Val3(val3), Val4(val4),
      Val5(val5) {
  }

  virtual int snprint(char *Buffer, unsigned BufferSize) const {
    return base::snprintf(Buffer, BufferSize, Fmt, Val1, Val2, Val3, Val4, Val5);
  }
};

/// This is a helper function that is used to produce formatted output.
///
/// This is typically used like:
/// \code
///   OS << format("%0.4f", myfloat) << '\n';
/// \endcode
template <typename T>
inline format_object1<T> format(const char *Fmt, const T &Val) {
  return format_object1<T>(Fmt, Val);
}

/// This is a helper function that is used to produce formatted output.
///
/// This is typically used like:
/// \code
///   OS << format("%0.4f", myfloat) << '\n';
/// \endcode
template <typename T1, typename T2>
inline format_object2<T1, T2> format(const char *Fmt, const T1 &Val1,
                                     const T2 &Val2) {
  return format_object2<T1, T2>(Fmt, Val1, Val2);
}

/// This is a helper function that is used to produce formatted output.
///
/// This is typically used like:
/// \code
///   OS << format("%0.4f", myfloat) << '\n';
/// \endcode
template <typename T1, typename T2, typename T3>
  inline format_object3<T1, T2, T3> format(const char *Fmt, const T1 &Val1,
                                           const T2 &Val2, const T3 &Val3) {
  return format_object3<T1, T2, T3>(Fmt, Val1, Val2, Val3);
}

/// This is a helper function that is used to produce formatted output.
///
/// This is typically used like:
/// \code
///   OS << format("%0.4f", myfloat) << '\n';
/// \endcode
template <typename T1, typename T2, typename T3, typename T4>
inline format_object4<T1, T2, T3, T4> format(const char *Fmt, const T1 &Val1,
                                             const T2 &Val2, const T3 &Val3,
                                             const T4 &Val4) {
  return format_object4<T1, T2, T3, T4>(Fmt, Val1, Val2, Val3, Val4);
}

/// This is a helper function that is used to produce formatted output.
///
/// This is typically used like:
/// \code
///   OS << format("%0.4f", myfloat) << '\n';
/// \endcode
template <typename T1, typename T2, typename T3, typename T4, typename T5>
inline format_object5<T1, T2, T3, T4, T5> format(const char *Fmt,const T1 &Val1,
                                             const T2 &Val2, const T3 &Val3,
                                             const T4 &Val4, const T5 &Val5) {
  return format_object5<T1, T2, T3, T4, T5>(Fmt, Val1, Val2, Val3, Val4, Val5);
}

} // End of sippet namespace

#endif // SIPPET_BASE_FORMAT_H_
