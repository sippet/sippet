// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webrtc/modules/audio_coding/codecs/g729/include/audio_encoder_g729.h"

#include <limits>
#include "webrtc/base/checks.h"
#include "webrtc/modules/audio_coding/codecs/g729/include/g729_interface.h"

namespace webrtc {

namespace {

const int kSampleRateHz = 8000;
const int kNumChannels = 1;
const size_t kSamplesPer10msFrame = 80;

} // empty namespace

AudioEncoderG729::AudioEncoderG729(const Config& config)
    : payload_type_(config.payload_type),
      num_10ms_frames_per_packet_(config.frame_size_ms / 10) {
  CHECK_EQ(config.frame_size_ms % 10, 0)
      << "Frame size must be an integer multiple of 10 ms.";
  speech_buffer_.reserve(kSamplesPer10msFrame);
  CHECK_EQ(0, WebRtcG729_EncoderCreate(&inst_));
  CHECK_EQ(0, WebRtcG729_EncoderInit(inst_, config.enable_dtx));
}

AudioEncoderG729::~AudioEncoderG729() {
  CHECK_EQ(0, WebRtcG729_EncoderFree(inst_));
}

int AudioEncoderG729::sample_rate_hz() const {
  return kSampleRateHz;
}
int AudioEncoderG729::num_channels() const {
  return kNumChannels;
}
int AudioEncoderG729::Num10MsFramesInNextPacket() const {
  return num_10ms_frames_per_packet_;
}

int AudioEncoderG729::Max10MsFramesInAPacket() const {
  return num_10ms_frames_per_packet_;
}

bool AudioEncoderG729::EncodeInternal(uint32_t timestamp,
                                      const int16_t* audio,
                                      size_t max_encoded_bytes,
                                      uint8_t* encoded,
                                      EncodedInfo* info) {
  CHECK_GE(max_encoded_bytes, size_t(10));
  if (speech_buffer_.empty())
    first_timestamp_in_buffer_ = timestamp;
  speech_buffer_.insert(speech_buffer_.end(), audio,
                        audio + kSamplesPer10msFrame);
  if (speech_buffer_.size() < (static_cast<size_t>(num_10ms_frames_per_packet_) *
                               kSamplesPer10msFrame)) {
    info->encoded_bytes = 0;
    return true;
  }
  CHECK_EQ(speech_buffer_.size(),
           static_cast<size_t>(num_10ms_frames_per_packet_) *
           kSamplesPer10msFrame);
  int16_t r = WebRtcG729_Encode(
      inst_, &speech_buffer_[0],
      num_10ms_frames_per_packet_ * kSamplesPer10msFrame,
      encoded);
  speech_buffer_.clear();
  if (r < 0)
    return false;
  info->encoded_bytes = r;
  info->encoded_timestamp = first_timestamp_in_buffer_;
  info->payload_type = payload_type_;
  return true;
}

}  // namespace webrtc
