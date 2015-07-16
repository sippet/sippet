// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_PHONE_H_
#define SIPPET_PHONE_PHONE_H_

#include <string>
#include <vector>

#include "base/memory/ref_counted.h"
#include "url/gurl.h"

#include "sippet/phone/settings.h"
#include "sippet/phone/call.h"
#include "sippet/phone/phone_state.h"

namespace sippet {
namespace phone {

// Base phone class
class Phone :
  public base::RefCountedThreadSafe<Phone> {
 public:
  // Phone delegate
  class Delegate {
   public:
    // Called to inform network errors, at any moment.
    virtual void OnNetworkError(int error_code) = 0;

    // Called to inform completion of the last registration attempt.
    virtual void OnRegisterCompleted(int status_code,
             const std::string& status_text) = 0;

    // Called to inform a refresh registration error.
    virtual void OnRefreshError(int status_code,
             const std::string& status_text) = 0;

    // Called to inform completion of the last unregistration attempt.
    virtual void OnUnregisterCompleted(int status_code,
             const std::string& status_text) = 0;

    // Called on incoming calls.
    virtual void OnIncomingCall(const scoped_refptr<Call>& call) = 0;

   protected:
    // Dtor protected as objects shouldn't be deleted via this interface.
    ~Delegate() {}
  };

  // Initialize the |Phone| system.
  static void Initialize();

  // Create a |Phone| instance.
  static scoped_refptr<Phone> Create(Delegate *delegate);

  // Get the |Phone| state.
  virtual PhoneState state() const = 0;

  // Initialize a |Phone| instance.
  virtual bool Init(const Settings& settings) = 0;

  // Register the Phone to receive incoming requests.
  virtual bool Register() = 0;

  // Unregister current account.
  virtual bool Unregister() = 0;

  // Unregister all contacts associated with the account.
  virtual bool UnregisterAll() = 0;

  // Start a call to the given destination.
  virtual scoped_refptr<Call> MakeCall(const std::string& destination) = 0;
    
  // Hang up all active calls.
  virtual void HangUpAll() = 0;

 protected:
  friend class base::RefCountedThreadSafe<Phone>;
  virtual ~Phone() {}
};

} // namespace sippet
} // namespace phone

#endif // SIPPET_PHONE_PHONE_H_
