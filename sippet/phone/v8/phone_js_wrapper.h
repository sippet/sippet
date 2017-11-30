// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_V8_PHONE_JS_WRAPPER_H_
#define SIPPET_PHONE_V8_PHONE_JS_WRAPPER_H_

#include "sippet/phone/phone.h"
#include "sippet/phone/v8/js_callback.h"

#include "base/message_loop/message_loop.h"
#include "gin/handle.h"
#include "gin/object_template_builder.h"
#include "gin/runner.h"
#include "gin/wrappable.h"

namespace sippet {
namespace phone {

class CallJsWrapper;

class PhoneJsWrapper :
    public gin::Wrappable<PhoneJsWrapper>,
    public Phone::Delegate {
 public:
  static gin::WrapperInfo kWrapperInfo;
  ~PhoneJsWrapper() override;
  static gin::Handle<PhoneJsWrapper> Create(
      v8::Isolate* isolate);

  // gin::Wrappable<PhoneJsWrapper> overrides
  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) override;

  // JS interface implementation.
  PhoneState state() const;
  bool Init(gin::Arguments args);
  void Register(v8::Handle<v8::Function> function);
  void StartRefreshRegister(v8::Handle<v8::Function> function);
  void StopRefreshRegister();
  void Unregister(v8::Handle<v8::Function> function);
  void UnregisterAll(v8::Handle<v8::Function> function);
  gin::Handle<CallJsWrapper> MakeCall(const std::string& destination,
      v8::Handle<v8::Function> function);
  void On(const std::string& key, v8::Handle<v8::Function> function);

  // Phone::Delegate implementation
  void OnIncomingCall(const scoped_refptr<Call>& call) override;

  // Callbacks
  void OnRegisterCompleted(int error);
  void OnRefreshCompleted(int error);
  void OnUnregisterCompleted(int error);

  // Dispatched to message loop
  void RunRegisterCompleted(int error);
  void RunRefreshCompleted(int error);
  void RunUnregisterCompleted(int error);
  void RunIncomingCall(const scoped_refptr<Call>& call);

 private:
  explicit PhoneJsWrapper(
      v8::Isolate* isolate);

  v8::Isolate* isolate_;
  scoped_refptr<Phone> phone_;
  base::MessageLoop* message_loop_;

  JsCallback<void(int)> on_register_completed_;
  JsCallback<void(int)> on_refresh_completed_;
  JsCallback<void(int)> on_unregister_completed_;
  JsCallback<void(gin::Handle<CallJsWrapper>)> on_incoming_call_;

  DISALLOW_COPY_AND_ASSIGN(PhoneJsWrapper);
};

class PhoneJsModule : public gin::Wrappable<PhoneJsModule> {
public:
  static const char kName[];
  static gin::WrapperInfo kWrapperInfo;
  static gin::Handle<PhoneJsModule> Create(v8::Isolate* isolate);
  static v8::Local<v8::Value> GetModule(v8::Isolate* isolate);

private:
  PhoneJsModule();
  ~PhoneJsModule() override;

  gin::ObjectTemplateBuilder
      GetObjectTemplateBuilder(v8::Isolate* isolate) override;
};

}  // namespace phone
}  // namespace sippet

#endif // SIPPET_PHONE_V8_PHONE_JS_WRAPPER_H_
