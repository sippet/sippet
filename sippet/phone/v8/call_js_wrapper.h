// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_V8_CALL_JS_WRAPPER_H_
#define SIPPET_PHONE_V8_CALL_JS_WRAPPER_H_

#include "sippet/phone/call.h"
#include "sippet/phone/v8/js_function_call.h"

#include "base/message_loop/message_loop.h"
#include "gin/handle.h"
#include "gin/object_template_builder.h"
#include "gin/wrappable.h"

namespace sippet {
namespace phone {

class CallJsWrapper
  : public gin::Wrappable<CallJsWrapper>,
    public Call::Delegate {
public:
  static gin::WrapperInfo kWrapperInfo;
  ~CallJsWrapper() override;
  static gin::Handle<CallJsWrapper> Create(
      v8::Isolate* isolate,
      const scoped_refptr<Call>& call);

  // gin::Wrappable<PhoneJsWrapper> overrides
  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) override;

  // JS interface implementation.
  Call::Direction direction() const;
  Call::State state() const;
  std::string uri() const;
  std::string name() const;
  base::Time creation_time() const;
  base::Time start_time() const;
  base::Time end_time() const;
  base::TimeDelta duration() const;
  bool PickUp();
  bool Reject();
  bool HangUp();
  void SendDtmf(const std::string& digits);

  // Set the callbacks
  void On(const std::string& key, v8::Handle<v8::Function> function);

  // Call::Delegate implementation
  void OnError(int status_code,
      const std::string& status_text) override;
  void OnRinging() override;
  void OnEstablished() override;
  void OnHungUp() override;

  // Dispatched to message loop
  void RunError(int status_code,
      const std::string& status_text);
  void RunRinging();
  void RunEstablished();
  void RunHungUp();

private:
  explicit CallJsWrapper(
      v8::Isolate* isolate,
      const scoped_refptr<Call>& call);

  v8::Isolate* isolate_;
  scoped_refptr<Call> call_;
  base::MessageLoop* message_loop_;

  JsFunctionCall<CallJsWrapper> on_error_;
  JsFunctionCall<CallJsWrapper> on_ringing_;
  JsFunctionCall<CallJsWrapper> on_established_;
  JsFunctionCall<CallJsWrapper> on_hungup_;

  DISALLOW_COPY_AND_ASSIGN(CallJsWrapper);
};

} // namespace phone
} // namespace sippet

#endif // SIPPET_PHONE_V8_CALL_JS_WRAPPER_H_
