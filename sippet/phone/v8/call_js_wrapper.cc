// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/v8/call_js_wrapper.h"

#include "gin/per_context_data.h"

namespace gin {

// Extend Converter to our type Call::Direction
template<>
struct Converter<sippet::phone::CallDirection> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
      sippet::phone::CallDirection val) {
    v8::Handle<v8::Integer> result(v8::Int32::New(isolate,
        static_cast<int32_t>(val)));
    return result;
  }

  static bool FromV8(v8::Isolate* isolate,
      v8::Handle<v8::Value> val,
      sippet::phone::CallDirection* out) {
    if (!val->IsInt32())
      return false;

    v8::Handle<v8::Integer> result(v8::Handle<v8::Integer>::Cast(val));
    *out = static_cast<sippet::phone::CallDirection>(result->Int32Value());
    return true;
  }
};

// Extend Converter to our type Call::State
template<>
struct Converter<sippet::phone::CallState> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
      sippet::phone::CallState val) {
    v8::Handle<v8::Integer> result(v8::Int32::New(isolate,
        static_cast<int32_t>(val)));
    return result;
  }

  static bool FromV8(v8::Isolate* isolate,
      v8::Handle<v8::Value> val,
      sippet::phone::CallState* out) {
    if (!val->IsInt32())
      return false;

    v8::Handle<v8::Integer> result(v8::Handle<v8::Integer>::Cast(val));
    *out = static_cast<sippet::phone::CallState>(result->Int32Value());
    return true;
  }
};

// Extend Converter to type base::Time
template<>
struct Converter<base::Time> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
      const base::Time& val) {
    return ConvertToV8(isolate, val.ToJsTime());
  }

  static bool FromV8(v8::Isolate* isolate,
      v8::Handle<v8::Value> val,
      base::Time* out) {
    if (!val->IsNumber())
      return false;
    v8::Handle<v8::Number> result(v8::Handle<v8::Number>::Cast(val));
    *out = base::Time::FromJsTime(result->NumberValue());
    return true;
  }
};

// Extend Converter to type base::TimeDelta
template<>
struct Converter<base::TimeDelta> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
      const base::TimeDelta& val) {
    return ConvertToV8(isolate, val.InSecondsF());
  }

  static bool FromV8(v8::Isolate* isolate,
      v8::Handle<v8::Value> val,
      base::TimeDelta* out) {
    if (!val->IsNumber())
      return false;
    v8::Handle<v8::Number> result(v8::Handle<v8::Number>::Cast(val));
    *out = base::TimeDelta::FromSecondsD(result->NumberValue());
    return true;
  }
};

} // namespace gin

namespace sippet {
namespace phone {

namespace {

const char kOnCompleted[] = "::sippet::call::OnCompleted";
const char kOnHangupCompleted[] = "::sippet::call::OnHangupCompleted";

}  // namespace

gin::WrapperInfo CallJsWrapper::kWrapperInfo = { gin::kEmbedderNativeGin };

CallJsWrapper::CallJsWrapper(v8::Isolate* isolate) :
  isolate_(isolate),
  message_loop_(base::MessageLoop::current()),
  on_completed_(kOnCompleted),
  on_hangup_completed_(kOnHangupCompleted) {
}

CallJsWrapper::~CallJsWrapper() {
}

gin::Handle<CallJsWrapper> CallJsWrapper::Create(v8::Isolate* isolate) {
  return gin::CreateHandle(isolate, new CallJsWrapper(isolate));
}

void CallJsWrapper::set_call_instance(
    const scoped_refptr<Call>& instance) {
  call_ = instance;
}

void CallJsWrapper::set_completed_function(v8::Handle<v8::Function> function) {
  on_completed_.SetFunction(this, isolate_, function);
}

gin::ObjectTemplateBuilder CallJsWrapper::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  gin::ObjectTemplateBuilder builder(
      Wrappable<CallJsWrapper>::GetObjectTemplateBuilder(isolate));
  builder.SetProperty("direction", &CallJsWrapper::direction);
  builder.SetProperty("state", &CallJsWrapper::state);
  builder.SetProperty("uri", &CallJsWrapper::uri);
  builder.SetProperty("name", &CallJsWrapper::name);
  builder.SetProperty("creationTime", &CallJsWrapper::creation_time);
  builder.SetProperty("startTime", &CallJsWrapper::start_time);
  builder.SetProperty("endTime", &CallJsWrapper::end_time);
  builder.SetProperty("duration", &CallJsWrapper::duration);
  builder.SetMethod("pickUp", &CallJsWrapper::PickUp);
  builder.SetMethod("reject", &CallJsWrapper::Reject);
  builder.SetMethod("hangup", &CallJsWrapper::HangUp);
  builder.SetMethod("sendDtmf", &CallJsWrapper::SendDtmf);
  return builder;
}

CallDirection CallJsWrapper::direction() const {
  return call_->direction();
}

CallState CallJsWrapper::state() const {
  return call_->state();
}

std::string CallJsWrapper::uri() const {
  return call_->uri().spec();
}

std::string CallJsWrapper::name() const {
  return call_->name();
}

base::Time CallJsWrapper::creation_time() const {
  return call_->creation_time();
}

base::Time CallJsWrapper::start_time() const {
  return call_->start_time();
}

base::Time CallJsWrapper::end_time() const {
  return call_->end_time();
}

base::TimeDelta CallJsWrapper::duration() const {
  return call_->duration();
}

void CallJsWrapper::PickUp(v8::Handle<v8::Function> function) {
  on_completed_.SetFunction(this, isolate_, function);
  call_->PickUp(base::Bind(
      &CallJsWrapper::OnCompleted, base::Unretained(this)));
}

void CallJsWrapper::Reject() {
  call_->Reject();
}

void CallJsWrapper::HangUp(v8::Handle<v8::Function> function) {
  on_hangup_completed_.SetFunction(this, isolate_, function);
  call_->HangUp(base::Bind(
      &CallJsWrapper::OnHangupCompleted, base::Unretained(this)));
}

void CallJsWrapper::SendDtmf(const std::string& digits) {
  call_->SendDtmf(digits);
}

void CallJsWrapper::OnCompleted(int error) {
  message_loop_->PostTask(FROM_HERE,
      base::Bind(&CallJsWrapper::RunCompleted,
          base::Unretained(this), error));
}

void CallJsWrapper::OnHangupCompleted(int error) {
  message_loop_->PostTask(FROM_HERE,
      base::Bind(&CallJsWrapper::RunHangupCompleted,
          base::Unretained(this), error));
}

void CallJsWrapper::RunCompleted(int error) {
  on_completed_.Run(this, error);
}

void CallJsWrapper::RunHangupCompleted(int error) {
  on_hangup_completed_.Run(this, error);
}

} // namespace phone
} // namespace sippet
