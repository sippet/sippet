/*
 *  Copyright (c) 2015 The Sippet project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include <string>
#include <sstream>

#include "testing/gtest/include/gtest/gtest.h"
#include "webrtc/modules/audio_coding/codecs/g729/include/g729_interface.h"
#include "webrtc/modules/audio_coding/codecs/g729/g729_inst.h"
#include "webrtc/modules/audio_coding/neteq/tools/audio_loop.h"
#include "webrtc/test/testsupport/fileutils.h"

namespace webrtc {

using test::AudioLoop;
using ::testing::TestWithParam;
using ::testing::Values;
using ::testing::Combine;

// Maximum number of bytes in output bitstream.
const size_t kMaxBytes = 1000;
// Sample rate of G729.
const int kG729RateKhz = 8;
// Number of samples-per-channel in a 20 ms frame, sampled at 8 kHz.
const int kG72920msFrameSamples = kG729RateKhz * 20;
// Number of samples-per-channel in a 10 ms frame, sampled at 8 kHz.
const int kG72910msFrameSamples = kG729RateKhz * 10;

// Some collected G.729 frames for testing the bitfield encoder
const uint8_t expected_test_bits[2][10] = {
  { 0x1a, 0xc1, 0x00, 0xa0, 0x00, 0xfa, 0xc2, 0xff, 0xfd, 0x50 },
  { 0xe0, 0xA2, 0xD4, 0x21, 0x41, 0x69, 0x59, 0xEF, 0x20, 0x50 },
};

// Expected outputs from the above
const int16_t expected_test_parm[2][PRM_SIZE] = {
  { 0x001a, 0x0304, 0x0002, 0x0001, 0x0000, 0x000f,
    0x0056, 0x0002, 0x1fff, 0x000a, 0x0050 },
  { 0x00e0, 0x028b, 0x0050, 0x0001, 0x0141, 0x0006,
    0x004a, 0x0019, 0x1de4, 0x0000, 0x0050 },
};

template<typename T>
void PrintWord(const T *v, size_t size, const size_t columns,
               std::ostream &out) {
  size_t i;
  for (i = 0; i < size; i++) {
    if (i > 0)
      out << " ";
    out << std::setfill('0')
        << std::setw(sizeof(T) * 2)
        << std::hex
        << (int)v[i];
  }
  if (size < columns) { // Complete with spaces
    size_t j, k;
    for (j = i; j < columns; j++) {
      out << " ";
      for (k = 0; k < sizeof(T) * 2; k++)
        out << " ";
    }
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
    PrintWord(a + i, len, columns, out);
    if (memcmp(a + i, b + i, len * sizeof(T)) == 0) {
      out << "   ";
    }
    else {
      out << " ! ";
      ++errors;
    }
    PrintWord(b + i, len, columns, out);
    out << "\n";
  }
  if (!errors)
    return ::testing::AssertionSuccess();
  return ::testing::AssertionFailure()
    << m_expr << " and " << n_expr << " differ:\n"
    << out.str();
}

#define EXPECT_ARRAY_EQ(expected, actual, length) \
  EXPECT_PRED_FORMAT3(AssertSameArrays, expected, actual, length)
#define ASSERT_ARRAY_EQ(expected, actual, length) \
  ASSERT_PRED_FORMAT3(AssertSameArrays, expected, actual, length)

class G729BitFieldTest : public TestWithParam<
  std::tuple<const uint8_t *, size_t, const int16_t *> > {
protected:
  G729BitFieldTest();

  const uint8_t *expected_bits_;
  size_t expected_bits_size_;
  const int16_t *expected_params_;
};

G729BitFieldTest::G729BitFieldTest()
  : expected_bits_(std::get<0>(GetParam())),
    expected_bits_size_(std::get<1>(GetParam())),
    expected_params_(std::get<2>(GetParam())) {
}

// Test the bitfield encoder
TEST_P(G729BitFieldTest, Encode) {
  uint8_t bits[10];
  int16_t parm[PRM_SIZE+1];
  parm[0] = expected_bits_size_ == 10 ? 1 : 2; // 1 = speech, 2 = sid
  memcpy(&parm[1], expected_params_, PRM_SIZE*sizeof(int16_t));
  ASSERT_EQ(expected_bits_size_, prm2bits_rtp(parm, bits));
  EXPECT_ARRAY_EQ(expected_bits_, bits, expected_bits_size_);
}

// Test the bitfield encoder
TEST_P(G729BitFieldTest, Decode) {
  int16_t parm[PRM_SIZE + 2];
  bits2prm_rtp(expected_bits_, expected_bits_size_, parm);
  EXPECT_EQ(0, parm[0]); // It's not an erasure
  int16_t type = expected_bits_size_ == 10 ? 1 : 2; // 1 = speech, 2 = sid
  EXPECT_EQ(type, parm[1]);
  EXPECT_ARRAY_EQ(expected_params_, &parm[2], PRM_SIZE);
}

INSTANTIATE_TEST_CASE_P(VariousBitFields,
                        G729BitFieldTest,
                        Values(
                          std::make_tuple(expected_test_bits[0], 10,
                                          expected_test_parm[0]),
                          std::make_tuple(expected_test_bits[1], 10,
                                          expected_test_parm[1])
                        ));

class G729Test : public TestWithParam<const char*> {
 protected:
  G729Test();

  // Prepare |speech_data_| for encoding, read from a hard-coded file.
  // After preparation, |speech_data_.GetNextBlock()| returns a pointer to a
  // block of |block_length_ms| milliseconds. The data is looped every
  // |loop_length_ms| milliseconds.
  void PrepareSpeechData(int block_length_ms, int loop_length_ms);

  int EncodeDecode(WebRtcG729EncInst* encoder,
                   const int16_t* input_audio,
                   const int input_samples,
                   WebRtcG729DecInst* decoder,
                   int16_t* output_audio,
                   int16_t* audio_type);

  WebRtcG729EncInst* g729_encoder_;
  WebRtcG729DecInst* g729_decoder_;

  AudioLoop speech_data_;
  scoped_ptr<uint8_t[]> expected_bitstream_;
  std::string input_name_;
  uint8_t bitstream_[kMaxBytes];
  int encoded_bytes_;
};

G729Test::G729Test()
    : g729_encoder_(NULL),
      g729_decoder_(NULL),
      input_name_(GetParam()),
      encoded_bytes_(0) {
}

void G729Test::PrepareSpeechData(int block_length_ms,
                                 int loop_length_ms) {
  const std::string file_name =
        webrtc::test::ResourcePath("g729a/" + input_name_, "pcm");
  if (loop_length_ms < block_length_ms) {
    loop_length_ms = block_length_ms;
  }
  EXPECT_TRUE(speech_data_.Init(file_name,
                                loop_length_ms * kG729RateKhz,
                                block_length_ms * kG729RateKhz));

  const std::string encoded_name =
        webrtc::test::ResourcePath("g729a/" + input_name_, "g729a");
  FILE* fp = fopen(encoded_name.c_str(), "rb");
  EXPECT_TRUE(fp) << "Couldn't open encoded file " << encoded_name;
  expected_bitstream_.reset(new uint8_t[loop_length_ms]);
  size_t samples_read = fread(expected_bitstream_.get(), sizeof(uint8_t),
        loop_length_ms, fp);
  fclose(fp);
  EXPECT_EQ(samples_read, loop_length_ms);
}

int G729Test::EncodeDecode(WebRtcG729EncInst* encoder,
                           const int16_t* input_audio,
                           const int input_samples,
                           WebRtcG729DecInst* decoder,
                           int16_t* output_audio,
                           int16_t* audio_type) {
  encoded_bytes_ = WebRtcG729_Encode(encoder,
                                     input_audio,
                                     input_samples,
                                     bitstream_);

  EXPECT_EQ((input_samples / kG72910msFrameSamples) * 10, encoded_bytes_);
  EXPECT_ARRAY_EQ(expected_bitstream_.get(), bitstream_,
    (input_samples / kG72910msFrameSamples) * 10);

  return WebRtcG729_Decode(decoder, bitstream_,
                           encoded_bytes_, output_audio,
                           audio_type);
}

// Test failing Create.
TEST_P(G729Test, G729CreateFail) {
  // Test to see that an invalid pointer is caught.
  EXPECT_EQ(-1, WebRtcG729_EncoderCreate(NULL));
  EXPECT_EQ(-1, WebRtcG729_DecoderCreate(NULL));
}

// Test failing Free.
TEST_P(G729Test, G729FreeFail) {
  // Test to see that an invalid pointer is caught.
  EXPECT_EQ(-1, WebRtcG729_EncoderFree(NULL));
  EXPECT_EQ(-1, WebRtcG729_DecoderFree(NULL));
}

// Test normal Create and Free.
TEST_P(G729Test, G729CreateFree) {
  EXPECT_EQ(0, WebRtcG729_EncoderCreate(&g729_encoder_));
  EXPECT_EQ(0, WebRtcG729_DecoderCreate(&g729_decoder_));
  EXPECT_TRUE(g729_encoder_ != NULL);
  EXPECT_TRUE(g729_decoder_ != NULL);
  // Free encoder and decoder memory.
  EXPECT_EQ(0, WebRtcG729_EncoderFree(g729_encoder_));
  EXPECT_EQ(0, WebRtcG729_DecoderFree(g729_decoder_));
}

TEST_P(G729Test, G729EncodeDecode) {
  PrepareSpeechData(20, 20);

  // Create encoder memory.
  EXPECT_EQ(0, WebRtcG729_EncoderCreate(&g729_encoder_));
  EXPECT_EQ(0, WebRtcG729_DecoderCreate(&g729_decoder_));

  // Initialize
  EXPECT_EQ(0, WebRtcG729_EncoderInit(g729_encoder_));
  EXPECT_EQ(0, WebRtcG729_DecoderInit(g729_decoder_));

  // Encode & decode.
  int16_t audio_type;
  int16_t* output_data_decode = new int16_t[kG72920msFrameSamples];
  EXPECT_EQ(kG72920msFrameSamples,
            EncodeDecode(g729_encoder_, speech_data_.GetNextBlock(),
                         kG72920msFrameSamples, g729_decoder_,
                         output_data_decode, &audio_type));

  // Free memory.
  delete[] output_data_decode;
  EXPECT_EQ(0, WebRtcG729_EncoderFree(g729_encoder_));
  EXPECT_EQ(0, WebRtcG729_DecoderFree(g729_decoder_));
}

// Encode and decode one frame, initialize the decoder and
// decode once more.
TEST_P(G729Test, G729DecodeInit) {
  PrepareSpeechData(20, 20);

  // Create encoder memory.
  EXPECT_EQ(0, WebRtcG729_EncoderCreate(&g729_encoder_));
  EXPECT_EQ(0, WebRtcG729_DecoderCreate(&g729_decoder_));

  // Initialize
  EXPECT_EQ(0, WebRtcG729_EncoderInit(g729_encoder_));
  EXPECT_EQ(0, WebRtcG729_DecoderInit(g729_decoder_));

  // Encode & decode.
  int16_t audio_type;
  int16_t* output_data_decode = new int16_t[kG72920msFrameSamples];
  EXPECT_EQ(kG72920msFrameSamples,
            EncodeDecode(g729_encoder_, speech_data_.GetNextBlock(),
                         kG72920msFrameSamples, g729_decoder_,
                         output_data_decode, &audio_type));

  EXPECT_EQ(0, WebRtcG729_DecoderInit(g729_decoder_));

  EXPECT_EQ(kG72920msFrameSamples,
            WebRtcG729_Decode(g729_decoder_, bitstream_,
                              encoded_bytes_, output_data_decode,
                              &audio_type));

  // Free memory.
  delete[] output_data_decode;
  EXPECT_EQ(0, WebRtcG729_EncoderFree(g729_encoder_));
  EXPECT_EQ(0, WebRtcG729_DecoderFree(g729_decoder_));
}

// Test PLC.
TEST_P(G729Test, G729DecodePlc) {
  PrepareSpeechData(20, 20);

  // Create encoder memory.
  EXPECT_EQ(0, WebRtcG729_EncoderCreate(&g729_encoder_));
  EXPECT_EQ(0, WebRtcG729_DecoderCreate(&g729_decoder_));

  // Initialize
  EXPECT_EQ(0, WebRtcG729_EncoderInit(g729_encoder_));
  EXPECT_EQ(0, WebRtcG729_DecoderInit(g729_decoder_));

  // Encode & decode.
  int16_t audio_type;
  int16_t* output_data_decode = new int16_t[kG72920msFrameSamples];
  EXPECT_EQ(kG72920msFrameSamples,
            EncodeDecode(g729_encoder_, speech_data_.GetNextBlock(),
                         kG72920msFrameSamples, g729_decoder_,
                         output_data_decode, &audio_type));

  // Call decoder PLC.
  int16_t* plc_buffer = new int16_t[kG72910msFrameSamples];
  EXPECT_EQ(kG72910msFrameSamples,
            WebRtcG729_DecodePlc(g729_decoder_, plc_buffer, 1));

  // Free memory.
  delete[] plc_buffer;
  delete[] output_data_decode;
  EXPECT_EQ(0, WebRtcG729_EncoderFree(g729_encoder_));
  EXPECT_EQ(0, WebRtcG729_DecoderFree(g729_decoder_));
}

// Duration estimation.
TEST_P(G729Test, G729DurationEstimation) {
  PrepareSpeechData(20, 20);

  // Create.
  EXPECT_EQ(0, WebRtcG729_EncoderCreate(&g729_encoder_));
  EXPECT_EQ(0, WebRtcG729_DecoderCreate(&g729_decoder_));

  // Initialize
  EXPECT_EQ(0, WebRtcG729_EncoderInit(g729_encoder_));
  EXPECT_EQ(0, WebRtcG729_DecoderInit(g729_decoder_));

  // 10 ms. We use only first 10 ms of a 20 ms block.
  encoded_bytes_ = WebRtcG729_Encode(g729_encoder_,
                                     speech_data_.GetNextBlock(),
                                     kG72910msFrameSamples,
                                     bitstream_);
  // 20 ms
  encoded_bytes_ = WebRtcG729_Encode(g729_encoder_,
                                     speech_data_.GetNextBlock(),
                                     kG72920msFrameSamples,
                                     bitstream_);

  // Free memory.
  EXPECT_EQ(0, WebRtcG729_EncoderFree(g729_encoder_));
  EXPECT_EQ(0, WebRtcG729_DecoderFree(g729_decoder_));
}

INSTANTIATE_TEST_CASE_P(VariousInputs,
                        G729Test,
                        Values("sample", "speech"));

}  // namespace webrtc
