// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_PHONE_JS_WRAPPER_H_
#define SIPPET_PHONE_PHONE_JS_WRAPPER_H_

#include "sippet/phone/phone.h"
#include "sippet/phone/js_function_call.h"

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
  Phone::State state() const;
  bool Init(gin::Arguments args);
  bool Register();
  void Unregister();
  gin::Handle<CallJsWrapper> MakeCall(const std::string& destination);
  void HangUpAll();
  
  // Set the callbacks
  void On(const std::string& key, v8::Handle<v8::Function> function);

  // Phone::Delegate implementation
  void OnNetworkError(int error_code) override;
  void OnRegisterCompleted(int status_code,
      const std::string& status_text) override;
  void OnIncomingCall(const scoped_refptr<Call>& call) override;

  // Dispatched to message loop
  void RunNetworkError(int error_code);
  void RunLoginCompleted(int status_code,
      const std::string& status_text);
  void RunIncomingCall(const scoped_refptr<Call>& call);

 private:
  explicit PhoneJsWrapper(
      v8::Isolate* isolate);

  v8::Isolate* isolate_;
  scoped_refptr<Phone> phone_;
  base::MessageLoop* message_loop_;

  JsFunctionCall<PhoneJsWrapper> on_network_error_;
  JsFunctionCall<PhoneJsWrapper> on_login_completed_;
  JsFunctionCall<PhoneJsWrapper> on_incoming_call_;

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

} // namespace phone
} // namespace sippet

#endif // SIPPET_PHONE_PHONE_JS_WRAPPER_H_
