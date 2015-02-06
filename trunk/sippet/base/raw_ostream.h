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

#ifndef SIPPET_BASE_RAW_OSTREAM_H_
#define SIPPET_BASE_RAW_OSTREAM_H_

#include <string>
#include "base/strings/string_piece.h"
#include "base/basictypes.h"

namespace sippet {

class format_object_base;

//! This class implements an extremely fast bulk output stream that can *only*
//! output to a stream. It does not support seeking, reopening, rewinding, line
//! buffered disciplines etc. It is a simple buffer that outputs a chunk at a
//! time.
class raw_ostream {
private:
  DISALLOW_COPY_AND_ASSIGN(raw_ostream);

  /// The buffer is handled in such a way that the buffer is
  /// uninitialized, unbuffered, or out of space when OutBufCur >=
  /// OutBufEnd. Thus a single comparison suffices to determine if we
  /// need to take the slow path to write a single character.
  ///
  /// The buffer is in one of three states:
  ///  1. Unbuffered (BufferMode == Unbuffered)
  ///  1. Uninitialized (BufferMode != Unbuffered && OutBufStart == 0).
  ///  2. Buffered (BufferMode != Unbuffered && OutBufStart != 0 &&
  ///               OutBufEnd - OutBufStart >= 1).
  ///
  /// If buffered, then the raw_ostream owns the buffer if (BufferMode ==
  /// InternalBuffer); otherwise the buffer has been set via SetBuffer and is
  /// managed by the subclass.
  ///
  /// If a subclass installs an external buffer using SetBuffer then it can wait
  /// for a \see write_impl() call to handle the data which has been put into
  /// this buffer.
  char *OutBufStart, *OutBufEnd, *OutBufCur;

  enum BufferKind {
    Unbuffered = 0,
    InternalBuffer,
    ExternalBuffer
  } BufferMode;

public:
  explicit raw_ostream(bool unbuffered=false)
    : BufferMode(unbuffered ? Unbuffered : InternalBuffer) {
    // Start out ready to flush.
    OutBufStart = OutBufEnd = OutBufCur = 0;
  }

  virtual ~raw_ostream();

  /// tell - Return the current offset with the file.
  uint64 tell() const { return current_pos() + GetNumBytesInBuffer(); }

  //===--------------------------------------------------------------------===//
  // Configuration Interface
  //===--------------------------------------------------------------------===//

  /// SetBuffered - Set the stream to be buffered, with an automatically
  /// determined buffer size.
  void SetBuffered();

  /// SetBufferSize - Set the stream to be buffered, using the
  /// specified buffer size.
  void SetBufferSize(size_t Size) {
    flush();
    SetBufferAndMode(new char[Size], Size, InternalBuffer);
  }

  size_t GetBufferSize() const {
    // If we're supposed to be buffered but haven't actually gotten around
    // to allocating the buffer yet, return the value that would be used.
    if (BufferMode != Unbuffered && OutBufStart == 0)
      return preferred_buffer_size();

    // Otherwise just return the size of the allocated buffer.
    return OutBufEnd - OutBufStart;
  }

  /// SetUnbuffered - Set the stream to be unbuffered. When
  /// unbuffered, the stream will flush after every write. This routine
  /// will also flush the buffer immediately when the stream is being
  /// set to unbuffered.
  void SetUnbuffered() {
    flush();
    SetBufferAndMode(0, 0, Unbuffered);
  }

  size_t GetNumBytesInBuffer() const {
    return OutBufCur - OutBufStart;
  }

  //===--------------------------------------------------------------------===//
  // Data Output Interface
  //===--------------------------------------------------------------------===//

  void flush() {
    if (OutBufCur != OutBufStart)
      flush_nonempty();
  }

  raw_ostream &operator<<(char C) {
    if (OutBufCur >= OutBufEnd)
      return write(C);
    *OutBufCur++ = C;
    return *this;
  }

  raw_ostream &operator<<(unsigned char C) {
    if (OutBufCur >= OutBufEnd)
      return write(C);
    *OutBufCur++ = C;
    return *this;
  }

  raw_ostream &operator<<(signed char C) {
    if (OutBufCur >= OutBufEnd)
      return write(C);
    *OutBufCur++ = C;
    return *this;
  }

  raw_ostream &operator<<(const base::StringPiece &Str) {
    // Inline fast path, particularly for strings with a known length.
    size_t Size = Str.size();

    // Make sure we can use the fast path.
    if (OutBufCur+Size > OutBufEnd)
      return write(Str.data(), Size);

    memcpy(OutBufCur, Str.data(), Size);
    OutBufCur += Size;
    return *this;
  }

  raw_ostream &operator<<(unsigned long N);
  raw_ostream &operator<<(long N);
  raw_ostream &operator<<(unsigned long long N);
  raw_ostream &operator<<(long long N);
  raw_ostream &operator<<(unsigned int N) {
    return this->operator<<(static_cast<unsigned long>(N));
  }

  raw_ostream &operator<<(int N) {
    return this->operator<<(static_cast<long>(N));
  }

  raw_ostream &operator<<(double N);

  /// write_hex - Output \p N in hexadecimal, without any prefix or padding.
  raw_ostream &write_hex(unsigned long long N);

  /// write_escaped - Output \p Str, turning '\\', '\t', '\n', '"', and
  /// anything that doesn't satisfy std::isprint into an escape sequence.
  raw_ostream &write_escaped(const base::StringPiece &Str, bool UseHexEscapes = false);

  raw_ostream &write(unsigned char C);
  raw_ostream &write(const char *Ptr, size_t Size);

  // Formatted output, see the format() function in base/format.h.
  raw_ostream &operator<<(const format_object_base &Fmt);

  /// indent - Insert 'NumSpaces' spaces.
  raw_ostream &indent(unsigned NumSpaces);

  //===--------------------------------------------------------------------===//
  // Subclass Interface
  //===--------------------------------------------------------------------===//

private:
  /// write_impl - The is the piece of the class that is implemented
  /// by subclasses.  This writes the \p Size bytes starting at
  /// \p Ptr to the underlying stream.
  ///
  /// This function is guaranteed to only be called at a point at which it is
  /// safe for the subclass to install a new buffer via SetBuffer.
  ///
  /// \param Ptr The start of the data to be written. For buffered streams this
  /// is guaranteed to be the start of the buffer.
  ///
  /// \param Size The number of bytes to be written.
  ///
  /// \invariant { Size > 0 }
  virtual void write_impl(const char *Ptr, size_t Size) = 0;

  // An out of line virtual method to provide a home for the class vtable.
  virtual void handle();

  /// current_pos - Return the current position within the stream, not
  /// counting the bytes currently in the buffer.
  virtual uint64 current_pos() const = 0;

protected:
  /// SetBuffer - Use the provided buffer as the raw_ostream buffer. This is
  /// intended for use only by subclasses which can arrange for the output to go
  /// directly into the desired output buffer, instead of being copied on each
  /// flush.
  void SetBuffer(char *BufferStart, size_t Size) {
    SetBufferAndMode(BufferStart, Size, ExternalBuffer);
  }

  /// preferred_buffer_size - Return an efficient buffer size for the
  /// underlying output mechanism.
  virtual size_t preferred_buffer_size() const;

  /// getBufferStart - Return the beginning of the current stream buffer, or 0
  /// if the stream is unbuffered.
  const char *getBufferStart() const { return OutBufStart; }

  //===--------------------------------------------------------------------===//
  // Private Interface
  //===--------------------------------------------------------------------===//
private:
  /// SetBufferAndMode - Install the given buffer and mode.
  void SetBufferAndMode(char *BufferStart, size_t Size, BufferKind Mode);

  /// flush_nonempty - Flush the current buffer, which is known to be
  /// non-empty. This outputs the currently buffered data and resets
  /// the buffer to empty.
  void flush_nonempty();

  /// copy_to_buffer - Copy data into the buffer. Size must not be
  /// greater than the number of unused bytes in the buffer.
  void copy_to_buffer(const char *Ptr, size_t Size);
};

//===----------------------------------------------------------------------===//
// Output Stream Adaptors
//===----------------------------------------------------------------------===//

/// raw_string_ostream - A raw_ostream that writes to an std::string.  This is a
/// simple adaptor class. This class does not encounter output errors.
class raw_string_ostream : public raw_ostream {
  std::string &OS;

  /// write_impl - See raw_ostream::write_impl.
  void write_impl(const char *Ptr, size_t Size) override;

  /// current_pos - Return the current position within the stream, not
  /// counting the bytes currently in the buffer.
  uint64 current_pos() const override;

public:
  explicit raw_string_ostream(std::string &O) : OS(O) {}
  ~raw_string_ostream() override;

  /// str - Flushes the stream contents to the target string and returns
  ///  the string's reference.
  std::string& str() {
    flush();
    return OS;
  }
};

} // End of sippet namespace

#endif // SIPPET_BASE_RAW_OSTREAM_H_
