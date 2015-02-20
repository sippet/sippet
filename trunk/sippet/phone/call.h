// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_CALL_H_
#define SIPPET_PHONE_CALL_H_

#include <string>

#include "base/time/time.h"
#include "base/memory/ref_counted.h"
#include "url/gurl.h"

namespace sippet {
namespace phone {

class Phone;

// This class represents a specific call.
// Stores data like type and status and has methods to comunicate with the 
// phone-api.
class Call :
  public base::RefCountedThreadSafe<Call> {
 public:
  // Where the call is from
  enum Type {
    kTypeIncoming,
    kTypeOutgoing
  };

  // Call state: the life cycle of the call
  enum State {
    kStateCalling,
    kStateRinging,
    kStateEstablished,
    kStateHungUp,
    kStateError
  };

  // Gets the current call type.
  virtual Type type() const = 0;

  // Gets the current call state.
  virtual State state() const = 0;

  // Gets the call URI
  virtual GURL uri() const = 0;

  // Gets the caller username or number.
  virtual std::string name() const = 0;

  // Get the time when the call was created
  virtual base::Time creation_time() const = 0;

  // Get the when the call was started (established)
  virtual base::Time start_time() const = 0;

  // Get the time when the call was hung up
  virtual base::Time end_time() const = 0;

  // Get the duration of the call
  virtual base::TimeDelta duration() const = 0;

  // Answers the call (only for incoming calls).
  virtual bool Answer(int code = 200) = 0;

  // Hangs up the call
  virtual bool HangUp() = 0;

  // Send DTMF digits
  virtual void SendDtmf(const std::string& digits) = 0;

 protected:
  friend class base::RefCountedThreadSafe<Call>;
  virtual ~Call() {}
};

} // namespace sippet
} // namespace phone

#endif // SIPPET_PHONE_CALL_H_
