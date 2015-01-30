/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include <algorithm>  // max

#include "testing/gtest/include/gtest/gtest.h"

#include "webrtc/call.h"
#include "webrtc/common_video/interface/i420_video_frame.h"
#include "webrtc/frame_callback.h"
#include "webrtc/modules/rtp_rtcp/interface/rtp_header_parser.h"
#include "webrtc/modules/rtp_rtcp/source/rtcp_sender.h"
#include "webrtc/modules/rtp_rtcp/source/rtcp_utility.h"
#include "webrtc/system_wrappers/interface/critical_section_wrapper.h"
#include "webrtc/system_wrappers/interface/event_wrapper.h"
#include "webrtc/system_wrappers/interface/scoped_ptr.h"
#include "webrtc/system_wrappers/interface/sleep.h"
#include "webrtc/system_wrappers/interface/thread_wrapper.h"
#include "webrtc/test/direct_transport.h"
#include "webrtc/test/fake_encoder.h"
#include "webrtc/test/frame_generator_capturer.h"
#include "webrtc/test/null_transport.h"
#include "webrtc/video/transport_adapter.h"
#include "webrtc/video_send_stream.h"

namespace webrtc {

class SendTransportObserver : public test::NullTransport {
 public:
  explicit SendTransportObserver(unsigned long timeout_ms)
      : rtp_header_parser_(RtpHeaderParser::Create()),
        send_test_complete_(EventWrapper::Create()),
        timeout_ms_(timeout_ms) {}

  EventTypeWrapper Wait() { return send_test_complete_->Wait(timeout_ms_); }

  virtual void Stop() {}

 protected:
  scoped_ptr<RtpHeaderParser> rtp_header_parser_;
  scoped_ptr<EventWrapper> send_test_complete_;

 private:
  unsigned long timeout_ms_;
};

class VideoSendStreamTest : public ::testing::Test {
 public:
  VideoSendStreamTest() : fake_encoder_(Clock::GetRealTimeClock()) {}

 protected:
  void RunSendTest(Call* call,
                   const VideoSendStream::Config& config,
                   SendTransportObserver* observer) {
    VideoSendStream* send_stream = call->CreateSendStream(config);
    scoped_ptr<test::FrameGeneratorCapturer> frame_generator_capturer(
        test::FrameGeneratorCapturer::Create(
            send_stream->Input(), 320, 240, 30, Clock::GetRealTimeClock()));
    send_stream->StartSend();
    frame_generator_capturer->Start();

    EXPECT_EQ(kEventSignaled, observer->Wait());

    observer->Stop();
    frame_generator_capturer->Stop();
    send_stream->StopSend();
    call->DestroySendStream(send_stream);
  }

  VideoSendStream::Config GetSendTestConfig(Call* call) {
    VideoSendStream::Config config = call->GetDefaultSendConfig();
    config.encoder = &fake_encoder_;
    config.internal_source = false;
    config.rtp.ssrcs.push_back(kSendSsrc);
    test::FakeEncoder::SetCodecSettings(&config.codec, 1);
    return config;
  }

  void TestNackRetransmission(uint32_t retransmit_ssrc);

  static const uint32_t kSendSsrc;
  static const uint32_t kSendRtxSsrc;

  test::FakeEncoder fake_encoder_;
};

const uint32_t VideoSendStreamTest::kSendSsrc = 0xC0FFEE;
const uint32_t VideoSendStreamTest::kSendRtxSsrc = 0xBADCAFE;

TEST_F(VideoSendStreamTest, SendsSetSsrc) {
  class SendSsrcObserver : public SendTransportObserver {
   public:
    SendSsrcObserver() : SendTransportObserver(30 * 1000) {}

    virtual bool SendRTP(const uint8_t* packet, size_t length) OVERRIDE {
      RTPHeader header;
      EXPECT_TRUE(
          rtp_header_parser_->Parse(packet, static_cast<int>(length), &header));

      if (header.ssrc == kSendSsrc)
        send_test_complete_->Set();

      return true;
    }
  } observer;

  Call::Config call_config(&observer);
  scoped_ptr<Call> call(Call::Create(call_config));

  VideoSendStream::Config send_config = GetSendTestConfig(call.get());
  send_config.rtp.max_packet_size = 128;

  RunSendTest(call.get(), send_config, &observer);
}

TEST_F(VideoSendStreamTest, SupportsCName) {
  static std::string kCName = "PjQatC14dGfbVwGPUOA9IH7RlsFDbWl4AhXEiDsBizo=";
  class CNameObserver : public SendTransportObserver {
   public:
    CNameObserver() : SendTransportObserver(30 * 1000) {}

    virtual bool SendRTCP(const uint8_t* packet, size_t length) OVERRIDE {
      RTCPUtility::RTCPParserV2 parser(packet, length, true);
      EXPECT_TRUE(parser.IsValid());

      RTCPUtility::RTCPPacketTypes packet_type = parser.Begin();
      while (packet_type != RTCPUtility::kRtcpNotValidCode) {
        if (packet_type == RTCPUtility::kRtcpSdesChunkCode) {
          EXPECT_EQ(parser.Packet().CName.CName, kCName);
          send_test_complete_->Set();
        }

        packet_type = parser.Iterate();
      }

      return true;
    }
  } observer;

  Call::Config call_config(&observer);
  scoped_ptr<Call> call(Call::Create(call_config));

  VideoSendStream::Config send_config = GetSendTestConfig(call.get());
  send_config.rtp.c_name = kCName;

  RunSendTest(call.get(), send_config, &observer);
}

TEST_F(VideoSendStreamTest, SupportsAbsoluteSendTime) {
  static const uint8_t kAbsSendTimeExtensionId = 13;
  class AbsoluteSendTimeObserver : public SendTransportObserver {
   public:
    AbsoluteSendTimeObserver() : SendTransportObserver(30 * 1000) {
      EXPECT_TRUE(rtp_header_parser_->RegisterRtpHeaderExtension(
          kRtpExtensionAbsoluteSendTime, kAbsSendTimeExtensionId));
    }

    virtual bool SendRTP(const uint8_t* packet, size_t length) OVERRIDE {
      RTPHeader header;
      EXPECT_TRUE(
          rtp_header_parser_->Parse(packet, static_cast<int>(length), &header));

      if (header.extension.absoluteSendTime > 0)
        send_test_complete_->Set();

      return true;
    }
  } observer;

  Call::Config call_config(&observer);
  scoped_ptr<Call> call(Call::Create(call_config));

  VideoSendStream::Config send_config = GetSendTestConfig(call.get());
  send_config.rtp.extensions.push_back(
      RtpExtension("abs-send-time", kAbsSendTimeExtensionId));

  RunSendTest(call.get(), send_config, &observer);
}

TEST_F(VideoSendStreamTest, SupportsTransmissionTimeOffset) {
  static const uint8_t kTOffsetExtensionId = 13;
  class DelayedEncoder : public test::FakeEncoder {
   public:
    explicit DelayedEncoder(Clock* clock) : test::FakeEncoder(clock) {}
    virtual int32_t Encode(
        const I420VideoFrame& input_image,
        const CodecSpecificInfo* codec_specific_info,
        const std::vector<VideoFrameType>* frame_types) OVERRIDE {
      // A delay needs to be introduced to assure that we get a timestamp
      // offset.
      SleepMs(5);
      return FakeEncoder::Encode(input_image, codec_specific_info, frame_types);
    }
  } encoder(Clock::GetRealTimeClock());

  class TransmissionTimeOffsetObserver : public SendTransportObserver {
   public:
    TransmissionTimeOffsetObserver() : SendTransportObserver(30 * 1000) {
      EXPECT_TRUE(rtp_header_parser_->RegisterRtpHeaderExtension(
          kRtpExtensionTransmissionTimeOffset, kTOffsetExtensionId));
    }

    virtual bool SendRTP(const uint8_t* packet, size_t length) OVERRIDE {
      RTPHeader header;
      EXPECT_TRUE(
          rtp_header_parser_->Parse(packet, static_cast<int>(length), &header));

      EXPECT_GT(header.extension.transmissionTimeOffset, 0);
      send_test_complete_->Set();

      return true;
    }
  } observer;

  Call::Config call_config(&observer);
  scoped_ptr<Call> call(Call::Create(call_config));

  VideoSendStream::Config send_config = GetSendTestConfig(call.get());
  send_config.encoder = &encoder;
  send_config.rtp.extensions.push_back(
      RtpExtension("toffset", kTOffsetExtensionId));

  RunSendTest(call.get(), send_config, &observer);
}

class FakeReceiveStatistics : public NullReceiveStatistics {
 public:
  FakeReceiveStatistics(uint32_t send_ssrc,
                        uint32_t last_sequence_number,
                        uint32_t cumulative_lost,
                        uint8_t fraction_lost)
      : lossy_stats_(new LossyStatistician(last_sequence_number,
                                           cumulative_lost,
                                           fraction_lost)) {
    stats_map_[send_ssrc] = lossy_stats_.get();
  }

  virtual StatisticianMap GetActiveStatisticians() const OVERRIDE {
    return stats_map_;
  }

  virtual StreamStatistician* GetStatistician(uint32_t ssrc) const OVERRIDE {
    return lossy_stats_.get();
  }

 private:
  class LossyStatistician : public StreamStatistician {
   public:
    LossyStatistician(uint32_t extended_max_sequence_number,
                      uint32_t cumulative_lost,
                      uint8_t fraction_lost) {
      stats_.fraction_lost = fraction_lost;
      stats_.cumulative_lost = cumulative_lost;
      stats_.extended_max_sequence_number = extended_max_sequence_number;
    }
    virtual bool GetStatistics(Statistics* statistics, bool reset) OVERRIDE {
      *statistics = stats_;
      return true;
    }
    virtual void GetDataCounters(uint32_t* bytes_received,
                                 uint32_t* packets_received) const OVERRIDE {
      *bytes_received = 0;
      *packets_received = 0;
    }
    virtual uint32_t BitrateReceived() const OVERRIDE { return 0; }
    virtual void ResetStatistics() OVERRIDE {}
    virtual bool IsRetransmitOfOldPacket(const RTPHeader& header,
                                         int min_rtt) const OVERRIDE {
      return false;
    }

    virtual bool IsPacketInOrder(uint16_t sequence_number) const OVERRIDE {
      return true;
    }
    Statistics stats_;
  };

  scoped_ptr<LossyStatistician> lossy_stats_;
  StatisticianMap stats_map_;
};

TEST_F(VideoSendStreamTest, SupportsFec) {
  static const int kRedPayloadType = 118;
  static const int kUlpfecPayloadType = 119;
  class FecObserver : public SendTransportObserver {
   public:
    FecObserver()
        : SendTransportObserver(30 * 1000),
          transport_adapter_(&transport_),
          send_count_(0),
          received_media_(false),
          received_fec_(false) {}

    void SetReceiver(PacketReceiver* receiver) {
      transport_.SetReceiver(receiver);
    }

    virtual bool SendRTP(const uint8_t* packet, size_t length) OVERRIDE {
      RTPHeader header;
      EXPECT_TRUE(
          rtp_header_parser_->Parse(packet, static_cast<int>(length), &header));

      // Send lossy receive reports to trigger FEC enabling.
      if (send_count_++ % 2 != 0) {
        // Receive statistics reporting having lost 50% of the packets.
        FakeReceiveStatistics lossy_receive_stats(
            kSendSsrc, header.sequenceNumber, send_count_ / 2, 127);
        RTCPSender rtcp_sender(
            0, false, Clock::GetRealTimeClock(), &lossy_receive_stats);
        EXPECT_EQ(0, rtcp_sender.RegisterSendTransport(&transport_adapter_));

        rtcp_sender.SetRTCPStatus(kRtcpNonCompound);
        rtcp_sender.SetRemoteSSRC(kSendSsrc);

        RTCPSender::FeedbackState feedback_state;

        EXPECT_EQ(0, rtcp_sender.SendRTCP(feedback_state, kRtcpRr));
      }

      EXPECT_EQ(kRedPayloadType, header.payloadType);

      uint8_t encapsulated_payload_type = packet[header.headerLength];

      if (encapsulated_payload_type == kUlpfecPayloadType) {
        received_fec_ = true;
      } else {
        received_media_ = true;
      }

      if (received_media_ && received_fec_)
        send_test_complete_->Set();

      return true;
    }

    virtual void Stop() OVERRIDE { transport_.StopSending(); }

   private:
    internal::TransportAdapter transport_adapter_;
    test::DirectTransport transport_;
    int send_count_;
    bool received_media_;
    bool received_fec_;
  } observer;

  Call::Config call_config(&observer);
  scoped_ptr<Call> call(Call::Create(call_config));

  observer.SetReceiver(call->Receiver());

  VideoSendStream::Config send_config = GetSendTestConfig(call.get());
  send_config.rtp.fec.red_payload_type = kRedPayloadType;
  send_config.rtp.fec.ulpfec_payload_type = kUlpfecPayloadType;

  RunSendTest(call.get(), send_config, &observer);
}

void VideoSendStreamTest::TestNackRetransmission(uint32_t retransmit_ssrc) {
  class NackObserver : public SendTransportObserver {
   public:
    explicit NackObserver(uint32_t retransmit_ssrc)
        : SendTransportObserver(30 * 1000),
          transport_adapter_(&transport_),
          send_count_(0),
          retransmit_ssrc_(retransmit_ssrc),
          nacked_sequence_number_(0) {}

    void SetReceiver(PacketReceiver* receiver) {
      transport_.SetReceiver(receiver);
    }

    virtual bool SendRTP(const uint8_t* packet, size_t length) OVERRIDE {
      RTPHeader header;
      EXPECT_TRUE(
          rtp_header_parser_->Parse(packet, static_cast<int>(length), &header));

      // Nack second packet after receiving the third one.
      if (++send_count_ == 3) {
        nacked_sequence_number_ = header.sequenceNumber - 1;
        NullReceiveStatistics null_stats;
        RTCPSender rtcp_sender(
            0, false, Clock::GetRealTimeClock(), &null_stats);
        EXPECT_EQ(0, rtcp_sender.RegisterSendTransport(&transport_adapter_));

        rtcp_sender.SetRTCPStatus(kRtcpNonCompound);
        rtcp_sender.SetRemoteSSRC(kSendSsrc);

        RTCPSender::FeedbackState feedback_state;

        EXPECT_EQ(0,
                  rtcp_sender.SendRTCP(
                      feedback_state, kRtcpNack, 1, &nacked_sequence_number_));
      }

      uint16_t sequence_number = header.sequenceNumber;

      if (header.ssrc == retransmit_ssrc_ && retransmit_ssrc_ != kSendSsrc) {
        // Not kSendSsrc, assume correct RTX packet. Extract sequence number.
        const uint8_t* rtx_header = packet + header.headerLength;
        sequence_number = (rtx_header[0] << 8) + rtx_header[1];
      }

      if (sequence_number == nacked_sequence_number_) {
        EXPECT_EQ(retransmit_ssrc_, header.ssrc);
        send_test_complete_->Set();
      }

      return true;
    }

    virtual void Stop() OVERRIDE { transport_.StopSending(); }

   private:
    internal::TransportAdapter transport_adapter_;
    test::DirectTransport transport_;
    int send_count_;
    uint32_t retransmit_ssrc_;
    uint16_t nacked_sequence_number_;
  } observer(retransmit_ssrc);

  Call::Config call_config(&observer);
  scoped_ptr<Call> call(Call::Create(call_config));
  observer.SetReceiver(call->Receiver());

  VideoSendStream::Config send_config = GetSendTestConfig(call.get());
  send_config.rtp.nack.rtp_history_ms = 1000;
  if (retransmit_ssrc != kSendSsrc)
    send_config.rtp.rtx.ssrcs.push_back(retransmit_ssrc);

  RunSendTest(call.get(), send_config, &observer);
}

TEST_F(VideoSendStreamTest, RetransmitsNack) {
  // Normal NACKs should use the send SSRC.
  TestNackRetransmission(kSendSsrc);
}

TEST_F(VideoSendStreamTest, RetransmitsNackOverRtx) {
  // NACKs over RTX should use a separate SSRC.
  TestNackRetransmission(kSendRtxSsrc);
}

TEST_F(VideoSendStreamTest, MaxPacketSize) {
  class PacketSizeObserver : public SendTransportObserver {
   public:
    PacketSizeObserver(size_t max_length) : SendTransportObserver(30 * 1000),
      max_length_(max_length), accumulated_size_(0) {}

    virtual bool SendRTP(const uint8_t* packet, size_t length) OVERRIDE {
      RTPHeader header;
      EXPECT_TRUE(
          rtp_header_parser_->Parse(packet, static_cast<int>(length), &header));

      EXPECT_LE(length, max_length_);

      accumulated_size_ += length;

      // Marker bit set indicates last fragment of a packet
      if (header.markerBit) {
        if (accumulated_size_ + length > max_length_) {
          // The packet was fragmented, total size was larger than max size,
          // but size of individual fragments were within size limit => pass!
          send_test_complete_->Set();
        }
        accumulated_size_ = 0; // Last fragment, reset packet size
      }

      return true;
    }

   private:
    size_t max_length_;
    size_t accumulated_size_;
  };

  static const uint32_t kMaxPacketSize = 128;

  PacketSizeObserver observer(kMaxPacketSize);
  Call::Config call_config(&observer);
  scoped_ptr<Call> call(Call::Create(call_config));

  VideoSendStream::Config send_config = GetSendTestConfig(call.get());
  send_config.rtp.max_packet_size = kMaxPacketSize;

  RunSendTest(call.get(), send_config, &observer);
}

// The test will go through a number of phases.
// 1. Start sending packets.
// 2. As soon as the RTP stream has been detected, signal a low REMB value to
//    activate the auto muter.
// 3. Wait until |kMuteTimeFrames| have been captured without seeing any RTP
//    packets.
// 4. Signal a high REMB and the wait for the RTP stream to start again.
//    When the stream is detected again, the test ends.
TEST_F(VideoSendStreamTest, AutoMute) {
  static const int kMuteTimeFrames = 60;  // Mute for 2 seconds @ 30 fps.

  class RembObserver : public SendTransportObserver, public I420FrameCallback {
   public:
    RembObserver()
        : SendTransportObserver(30 * 1000),  // Timeout after 30 seconds.
          transport_adapter_(&transport_),
          clock_(Clock::GetRealTimeClock()),
          test_state_(kBeforeMute),
          rtp_count_(0),
          last_sequence_number_(0),
          mute_frame_count_(0),
          low_remb_bps_(0),
          high_remb_bps_(0),
          crit_sect_(CriticalSectionWrapper::CreateCriticalSection()) {}

    void SetReceiver(PacketReceiver* receiver) {
      transport_.SetReceiver(receiver);
    }

    virtual bool SendRTCP(const uint8_t* packet, size_t length) OVERRIDE {
      // Receive statistics reporting having lost 0% of the packets.
      // This is needed for the send-side bitrate controller to work properly.
      CriticalSectionScoped lock(crit_sect_.get());
      SendRtcpFeedback(0);  // REMB is only sent if value is > 0.
      return true;
    }

    virtual bool SendRTP(const uint8_t* packet, size_t length) OVERRIDE {
      CriticalSectionScoped lock(crit_sect_.get());
      ++rtp_count_;
      RTPHeader header;
      EXPECT_TRUE(
          rtp_header_parser_->Parse(packet, static_cast<int>(length), &header));
      last_sequence_number_ = header.sequenceNumber;

      if (test_state_ == kBeforeMute) {
        // The stream has started. Try to mute it.
        SendRtcpFeedback(low_remb_bps_);
        test_state_ = kDuringMute;
      } else if (test_state_ == kDuringMute) {
        mute_frame_count_ = 0;
      } else if (test_state_ == kWaitingForPacket) {
        send_test_complete_->Set();
      }

      return true;
    }

    // This method implements the I420FrameCallback.
    void FrameCallback(I420VideoFrame* video_frame) OVERRIDE {
      CriticalSectionScoped lock(crit_sect_.get());
      if (test_state_ == kDuringMute && ++mute_frame_count_ > kMuteTimeFrames) {
        SendRtcpFeedback(high_remb_bps_);
        test_state_ = kWaitingForPacket;
      }
    }

    void set_low_remb_bps(int value) { low_remb_bps_ = value; }

    void set_high_remb_bps(int value) { high_remb_bps_ = value; }

    virtual void Stop() OVERRIDE { transport_.StopSending(); }

   private:
    enum TestState {
      kBeforeMute,
      kDuringMute,
      kWaitingForPacket,
      kAfterMute
    };

    virtual void SendRtcpFeedback(int remb_value) {
      FakeReceiveStatistics receive_stats(
          kSendSsrc, last_sequence_number_, rtp_count_, 0);
      RTCPSender rtcp_sender(0, false, clock_, &receive_stats);
      EXPECT_EQ(0, rtcp_sender.RegisterSendTransport(&transport_adapter_));

      rtcp_sender.SetRTCPStatus(kRtcpNonCompound);
      rtcp_sender.SetRemoteSSRC(kSendSsrc);
      if (remb_value > 0) {
        rtcp_sender.SetREMBStatus(true);
        rtcp_sender.SetREMBData(remb_value, 0, NULL);
      }
      RTCPSender::FeedbackState feedback_state;
      EXPECT_EQ(0, rtcp_sender.SendRTCP(feedback_state, kRtcpRr));
    }

    internal::TransportAdapter transport_adapter_;
    test::DirectTransport transport_;
    Clock* clock_;
    TestState test_state_;
    int rtp_count_;
    int last_sequence_number_;
    int mute_frame_count_;
    int low_remb_bps_;
    int high_remb_bps_;
    scoped_ptr<CriticalSectionWrapper> crit_sect_;
  } observer;

  Call::Config call_config(&observer);
  scoped_ptr<Call> call(Call::Create(call_config));
  observer.SetReceiver(call->Receiver());

  VideoSendStream::Config send_config = GetSendTestConfig(call.get());
  send_config.rtp.nack.rtp_history_ms = 1000;
  send_config.pre_encode_callback = &observer;
  send_config.auto_mute = true;
  unsigned int min_bitrate_bps =
      send_config.codec.simulcastStream[0].minBitrate * 1000;
  observer.set_low_remb_bps(min_bitrate_bps - 10000);
  unsigned int threshold_window = std::max(min_bitrate_bps / 10, 10000u);
  ASSERT_GT(send_config.codec.simulcastStream[0].maxBitrate * 1000,
            min_bitrate_bps + threshold_window + 5000);
  observer.set_high_remb_bps(min_bitrate_bps + threshold_window + 5000);

  RunSendTest(call.get(), send_config, &observer);
}

}  // namespace webrtc
