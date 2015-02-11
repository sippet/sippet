/*
 *  Copyright (c) 2015 The Sippet project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MODULES_AUDIO_CODING_CODECS_G729_G729_INST_H_
#define WEBRTC_MODULES_AUDIO_CODING_CODECS_G729_G729_INST_H_

#include "g729a.h"

struct WebRtcG729EncInst {
  void *enc;
};

struct WebRtcG729DecInst {
  UWord8 last_frm[L_PACKED_G729A];
  void *dec;
};

#endif  // WEBRTC_MODULES_AUDIO_CODING_CODECS_G729_G729_INST_H_
