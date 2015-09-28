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
  virtual void Register(const net::CompletionCallback& on_completed) = 0;

  // Starts refreshing registration. The callback is executed only in case of
  // unrecoverable errors.
  virtual void StartRefreshRegister(const net::CompletionCallback& on_completed) = 0;

  // Stops refreshing registration.
  virtual void StopRefreshRegister() = 0;

  // Unregister current account.
  virtual void Unregister(const net::CompletionCallback& on_completed) = 0;

  // Unregister all accounts eventually registered on the registrar.
  virtual void UnregisterAll(const net::CompletionCallback& on_completed) = 0;

  // Start a call to the given destination. The completion callback will be
  // executed for each SIP response, including provisional responses, until
  // a final response (success or failure).
  virtual scoped_refptr<Call> MakeCall(const std::string& destination,
      const net::CompletionCallback& on_completed) = 0;
    
 protected:
  friend class base::RefCountedThreadSafe<Phone>;
  virtual ~Phone() {}
};

} // namespace sippet
} // namespace phone

#endif // SIPPET_PHONE_PHONE_H_
