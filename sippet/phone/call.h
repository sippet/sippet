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
  // Call direction: incoming or outgoing.
  enum Direction {
    kDirectionIncoming,
    kDirectionOutgoing
  };

  // Call state: corresponds to the |Call| lifecycle.
  enum State {
    kStateCalling,
    kStateRinging,
    kStateEstablished,
    kStateHungUp,
    kStateError
  };

  // Call delegate.
  class Delegate {
   public:
    // Called to inform a |Call| error.
    virtual void OnError(int status_code,
           const std::string& status_text) = 0;

    // Called when callee phone starts ringing.
    virtual void OnRinging() = 0;

    // Called when callee picks up the phone.
    virtual void OnEstablished() = 0;

    // Called when callee hangs up.
    virtual void OnHungUp() = 0;

   protected:
    // Dtor protected as objects shouldn't be deleted via this interface.
    ~Delegate() {}
  };

  // Get the current |Call| direction.
  virtual Direction direction() const = 0;

  // Get the current |Call| state.
  virtual State state() const = 0;

  // Set the |Call| delegate.
  virtual void set_delegate(Delegate *delegate) = 0;

  // Get the |Call| URI
  virtual GURL uri() const = 0;

  // Gets the callee username or number.
  virtual std::string name() const = 0;

  // Get the time when the |Call| was created
  virtual base::Time creation_time() const = 0;

  // Get the time when the |Call| has started (established)
  virtual base::Time start_time() const = 0;

  // Get the time when the |Call| was hung up
  virtual base::Time end_time() const = 0;

  // Get the duration of the |Call|
  virtual base::TimeDelta duration() const = 0;

  // Pick up the call (only for incoming calls). No effect if not in
  // |kStateRinging| state.
  virtual bool PickUp() = 0;

  // Reject the call (only for incoming calls). No effect if not in
  // |kStateRinging| state.
  virtual bool Reject() = 0;

  // Hang up the call. No effect if not in |kStateEstablished| state.
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
