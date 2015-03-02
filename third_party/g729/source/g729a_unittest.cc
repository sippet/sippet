// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <bitset>

#include "g729a.h"

extern "C" {
#include "ld8a.h"
}

#include "testing/gtest/include/gtest/gtest.h"

namespace {

template<typename T>
void PrintWord(const T *v, size_t size, std::ostream &out) {
  size_t i;
  for (i = 0; i < size; i++) {
    if (i > 0)
      out << " ";
    out << std::setfill('0')
        << std::setw(sizeof(T)*2)
        << std::hex
        << (int)v[i];
  }
}

// A predicate-formatter for asserting that two arrays are the same
template<typename T>
::testing::AssertionResult AssertSameArrays(const char* m_expr,
                                            const char* n_expr,
                                            const char* size_expr,
                                            const T *m,
                                            const T *n,
                                            size_t size) {
  const size_t columns = 4;
  std::ostringstream out;
  const T *a = reinterpret_cast<const T*>(m);
  const T *b = reinterpret_cast<const T*>(n);
  size_t i, errors = 0;
  for (i = 0; i < size; i += columns) {
    size_t len = columns;
    if (i + len > size)
      len = size - i;
    out << "  ";
    PrintWord(a + i, len, out);
    if (memcmp(a + i, b + i, len * sizeof(T)) == 0) {
      out << "   ";
    }
    else {
      out << " ! ";
      ++errors;
    }
    PrintWord(b + i, len, out);
    out << "\n";
  }
  if (!errors)
    return ::testing::AssertionSuccess();
  return ::testing::AssertionFailure()
      << m_expr << " and " << n_expr << " differ:\n"
      << out.str();
}

}  // empty namespace

class G729Test : public testing::Test {
 public:
  void SetUp() override {
  }
 protected:
  static const UWord8 expected_bits_[2][10];
  static const Word16 expected_parm_[2][PRM_SIZE];
};

const UWord8 G729Test::expected_bits_[2][10] = {
  { 0x1a, 0xc1, 0x00, 0xa0, 0x00, 0xfa, 0xc2, 0xff, 0xfd, 0x50 },
  { 0xe0, 0xA2, 0xD4, 0x21, 0x41, 0x69, 0x59, 0xEF, 0x20, 0x50 },
};

const Word16 G729Test::expected_parm_[2][PRM_SIZE] = {
  { 0x001a, 0x0304, 0x0002, 0x0001, 0x0000, 0x000f,
    0x0056, 0x0002, 0x1fff, 0x000a, 0x0050 },
  { 0x00e0, 0x028b, 0x0050, 0x0001, 0x0141, 0x0006,
    0x004a, 0x0019, 0x1de4, 0x0000, 0x0050 },
};

TEST_F(G729Test, bits2prm) {
  Word16 output_parm[PRM_SIZE];

  for (int i = 0; i < sizeof(expected_bits_)/sizeof(expected_bits_[0]); i++) {
    bits2prm_ld8k(expected_bits_[i], output_parm);

    EXPECT_PRED_FORMAT3(AssertSameArrays,
      expected_parm_[i], output_parm, PRM_SIZE);
  }
}

TEST_F(G729Test, prm2bits) {
  UWord8 output_bits[10];
  
  for (int i = 0; i < sizeof(expected_bits_) / sizeof(expected_bits_[0]); i++) {
    prm2bits_ld8k(expected_parm_[i], output_bits);

    EXPECT_PRED_FORMAT3(AssertSameArrays, expected_bits_[i], output_bits,
      10);
  }
}
