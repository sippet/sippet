// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_CALL_H_
#define SIPPET_PHONE_CALL_H_

#include <string>

#include "base/time/time.h"
#include "base/memory/ref_counted.h"

namespace sippet {
namespace phone {

class Phone;

// This class represents a specific call.
// Stores data like type and status and has methods to comunicate with the 
// phone-api.
class Call :
  public base::RefCountedThreadSafe<Call> {
 private:
  DISALLOW_COPY_AND_ASSIGN(Call);
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
  Type type() const { return type_; }

  // Gets the current call state.
  State state() const { return state_; }

  // Gets the call URI
  std::string uri() const { return uri_; }

  // Gets the caller username or number
  std::string name() const;

  // Answers the call (only for incoming calls).
  void Answer(const int code = 200);

  // Hangs up the call
  void HangUp();

  // Get the time when the call was created
  base::Time creation_time() const { return creation_time_; }

  // Get the when the call was started (established)
  base::Time start_time() const { return start_time_; }

  // Get the time when the call was hung up
  base::Time end_time() const { return end_time_; }

  // Get the duration of the call
  base::TimeDelta duration() const { return end_time_ - start_time_; }

 protected:
  friend class Phone;
  friend class base::RefCountedThreadSafe<Call>;

  Call(const std::string& uri,
       Phone* phone);
  ~Call() override {}

  void MakeCall();
};

} // namespace sippet
} // namespace phone

#endif // SIPPET_PHONE_CALL_H_
