// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_V8_CALL_JS_WRAPPER_H_
#define SIPPET_PHONE_V8_CALL_JS_WRAPPER_H_

#include "sippet/phone/call.h"
#include "sippet/phone/v8/js_callback.h"

#include "base/message_loop/message_loop.h"
#include "gin/handle.h"
#include "gin/object_template_builder.h"
#include "gin/wrappable.h"

namespace sippet {
namespace phone {

class CallJsWrapper
  : public gin::Wrappable<CallJsWrapper> {
public:
  static gin::WrapperInfo kWrapperInfo;
  ~CallJsWrapper() override;

  static gin::Handle<CallJsWrapper> Create(
      v8::Isolate* isolate);

  void set_call_instance(const scoped_refptr<Call>& instance);
  void set_completed_function(v8::Handle<v8::Function> function);

  // gin::Wrappable<PhoneJsWrapper> overrides
  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) override;

  // JS interface implementation.
  CallDirection direction() const;
  CallState state() const;
  std::string uri() const;
  std::string name() const;
  base::Time creation_time() const;
  base::Time start_time() const;
  base::Time end_time() const;
  base::TimeDelta duration() const;
  void PickUp(v8::Handle<v8::Function> function);
  void Reject();
  void HangUp(v8::Handle<v8::Function> function);
  void SendDtmf(const std::string& digits);

  // Callbacks
  void OnRinging();
  void OnEstablished();
  void OnHungUp();

  // Dispatched to message loop
  void RunCompleted(int error);
  void RunHangupCompleted(int error);

  void OnCompleted(int error);
  void OnHangupCompleted(int error);

private:
  explicit CallJsWrapper(v8::Isolate* isolate);

  v8::Isolate* isolate_;
  scoped_refptr<Call> call_;
  base::MessageLoop* message_loop_;

  JsCallback<void(int)> on_completed_;
  JsCallback<void(int)> on_hangup_completed_;

  DISALLOW_COPY_AND_ASSIGN(CallJsWrapper);
};

} // namespace phone
} // namespace sippet

#endif // SIPPET_PHONE_V8_CALL_JS_WRAPPER_H_
