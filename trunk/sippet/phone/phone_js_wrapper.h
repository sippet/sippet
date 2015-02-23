// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_PHONE_JS_WRAPPER_H_
#define SIPPET_PHONE_PHONE_JS_WRAPPER_H_

#include "sippet/phone/phone.h"

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
    public PhoneObserver {
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
  bool Login(const Account &account);
  gin::Handle<CallJsWrapper> MakeCall(const std::string& destination);
  void HangUpAll();
  void Logout();

  // PhoneObserver implementation
  void OnNetworkError(int error_code) override;
  void OnLoginCompleted(int status_code,
      const std::string& status_text) override;
  void OnIncomingCall(const scoped_refptr<Call>& call) override;
  void OnCallError(int status_code,
      const std::string& status_text,
      const scoped_refptr<Call>& call) override;
  void OnCallRinging(const scoped_refptr<Call>& call) override;
  void OnCallEstablished(const scoped_refptr<Call>& call) override;
  void OnCallHungUp(const scoped_refptr<Call>& call) override;

  // Dispatched to message loop
  void RunNetworkError(int error_code);
  void RunLoginCompleted(int status_code,
      const std::string& status_text);
  void RunIncomingCall(const scoped_refptr<Call>& call);
  void RunCallError(int status_code,
      const std::string& status_text,
      const scoped_refptr<Call>& call);
  void RunCallRinging(const scoped_refptr<Call>& call);
  void RunCallEstablished(const scoped_refptr<Call>& call);
  void RunCallHungUp(const scoped_refptr<Call>& call);

 private:
  explicit PhoneJsWrapper(
      v8::Isolate* isolate);

  v8::Isolate* isolate_;
  scoped_refptr<Phone> phone_;
  base::MessageLoop* message_loop_;

  class FunctionCall {
   public:
    FunctionCall(PhoneJsWrapper* outer,
                 const char *hidden_name);
    ~FunctionCall();
    
    void set_runner(const base::WeakPtr<gin::Runner>& runner);
    void Run(int argc, v8::Handle<v8::Value> *argv);

   private:
    PhoneJsWrapper* outer_;
    const char *hidden_name_;
    base::WeakPtr<gin::Runner> runner_;
  };

  FunctionCall on_network_error_;
  FunctionCall on_login_completed_;
  FunctionCall on_incoming_call_;
  FunctionCall on_call_error_;
  FunctionCall on_call_ringing_;
  FunctionCall on_call_established_;
  FunctionCall on_call_hungup_;

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
