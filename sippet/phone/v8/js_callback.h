// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_JS_FUNCTION_CALL_H_
#define SIPPET_PHONE_JS_FUNCTION_CALL_H_

#include "gin/runner.h"
#include "gin/converter.h"
#include "gin/per_context_data.h"
#include "gin/wrappable.h"

namespace sippet {
namespace phone {

template<typename Sig>
class JsCallback;

template<typename... Args>
class JsCallback<void(Args...)> {
 public:
  JsCallback(const char *hidden_name) :
    hidden_name_(hidden_name) {
  }

  ~JsCallback() {
  }

  template<typename T>
  void SetFunction(T* object, v8::Isolate* isolate,
      v8::Handle<v8::Function> function) {
    runner_ = gin::PerContextData::From(
        isolate->GetCurrentContext())->runner()->GetWeakPtr();
    object->GetWrapper(runner_->GetContextHolder()->isolate())->SetHiddenValue(
        gin::StringToSymbol(isolate, hidden_name_), function);
  }

  template<typename T>
  void Run(T* object) {
    // This can happen in spite of the weak callback because it is possible for
    // a gin::Handle<> to keep this object alive past when the isolate it is part
    // of is destroyed.
    if (!runner_.get()) {
      return;
    }
 
    gin::Runner::Scope scope(runner_.get());
    v8::Isolate* isolate = runner_->GetContextHolder()->isolate();
    v8::Handle<v8::Function> function = v8::Handle<v8::Function>::Cast(
        object->GetWrapper(isolate)->GetHiddenValue(
            gin::StringToSymbol(isolate, hidden_name_)));
    runner_->Call(function, v8::Undefined(isolate), 0, nullptr);
  }

  template<typename T>
  void Run(T* object, const Args&... args) {
    // This can happen in spite of the weak callback because it is possible for
    // a gin::Handle<> to keep this object alive past when the isolate it is part
    // of is destroyed.
    if (!runner_.get()) {
      return;
    }
 
    gin::Runner::Scope scope(runner_.get());
    v8::Isolate* isolate = runner_->GetContextHolder()->isolate();

    int argc = sizeof...(Args);
    v8::Local<v8::Value> argv[sizeof...(Args)];
    Argv(isolate, argv, 0, args...);

    v8::Handle<v8::Function> function = v8::Handle<v8::Function>::Cast(
        object->GetWrapper(isolate)->GetHiddenValue(
            gin::StringToSymbol(isolate, hidden_name_)));

    runner_->Call(function, v8::Undefined(isolate), argc, argv);
  }

 private:
  const char *hidden_name_;
  base::WeakPtr<gin::Runner> runner_;

  template<typename T>
  void Argv(v8::Isolate* isolate, v8::Local<v8::Value> *argv,
      T arg) {
    *argv = gin::ConvertToV8(isolate, arg);
  }
  template<typename T, typename... Arguments>
  void Argv(v8::Isolate* isolate, v8::Local<v8::Value> *argv,
      T arg, const Args&... args) {
    *argv = gin::ConvertToV8(isolate, arg);
    Argv(isolate, argv+1, args...);
  }
};

} // namespace phone
} // namespace sippet

#endif // SIPPET_PHONE_JS_FUNCTION_CALL_H_
