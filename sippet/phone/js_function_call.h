// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_JS_FUNCTION_CALL_H_
#define SIPPET_PHONE_JS_FUNCTION_CALL_H_

#include "gin/runner.h"
#include "gin/converter.h"
#include "gin/per_context_data.h"

namespace sippet {
namespace phone {

template<typename Outer>
class JsFunctionCall {
 public:
  JsFunctionCall(Outer* outer,
                 const char *hidden_name) :
    outer_(outer),
    hidden_name_(hidden_name) {
  }

  ~JsFunctionCall() {
  }
  
  void SetFunction(v8::Isolate* isolate, v8::Handle<v8::Function> function) {
    base::WeakPtr<gin::Runner> runner = gin::PerContextData::From(
        isolate->GetCurrentContext())->runner()->GetWeakPtr();
    outer_->GetWrapper(runner->GetContextHolder()->isolate())->SetHiddenValue(
        gin::StringToSymbol(isolate, hidden_name_),
        function);
    runner_ = runner;
  }

  void Run(int argc, v8::Handle<v8::Value> *argv) {
    // This can happen in spite of the weak callback because it is possible for
    // a gin::Handle<> to keep this object alive past when the isolate it is part
    // of is destroyed.
    if (!runner_.get()) {
      return;
    }
 
    gin::Runner::Scope scope(runner_.get());
    v8::Isolate* isolate = runner_->GetContextHolder()->isolate();
    v8::Handle<v8::Function> function = v8::Handle<v8::Function>::Cast(
        outer_->GetWrapper(isolate)->GetHiddenValue(
            gin::StringToSymbol(isolate, hidden_name_)));
    runner_->Call(function, v8::Undefined(isolate), argc, argv);
  }

 private:
  Outer* outer_;
  const char *hidden_name_;
  base::WeakPtr<gin::Runner> runner_;
};

} // namespace phone
} // namespace sippet

#endif // SIPPET_PHONE_JS_FUNCTION_CALL_H_
