// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/call_js_wrapper.h"

#include "gin/per_context_data.h"

namespace gin {

// Extend Converter to our type Call::Direction
template<>
struct Converter<sippet::phone::Call::Direction> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
      sippet::phone::Call::Direction val) {
    v8::Handle<v8::Integer> result(v8::Int32::New(isolate,
        static_cast<int32_t>(val)));
    return result;
  }

  static bool FromV8(v8::Isolate* isolate,
      v8::Handle<v8::Value> val,
      sippet::phone::Call::Direction* out) {
    if (!val->IsInt32())
      return false;

    v8::Handle<v8::Integer> result(v8::Handle<v8::Integer>::Cast(val));
    *out = static_cast<sippet::phone::Call::Direction>(result->Int32Value());
    return true;
  }
};

// Extend Converter to our type Call::State
template<>
struct Converter<sippet::phone::Call::State> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
      sippet::phone::Call::State val) {
    v8::Handle<v8::Integer> result(v8::Int32::New(isolate,
        static_cast<int32_t>(val)));
    return result;
  }

  static bool FromV8(v8::Isolate* isolate,
      v8::Handle<v8::Value> val,
      sippet::phone::Call::State* out) {
    if (!val->IsInt32())
      return false;

    v8::Handle<v8::Integer> result(v8::Handle<v8::Integer>::Cast(val));
    *out = static_cast<sippet::phone::Call::State>(result->Int32Value());
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

const char kOnError[] = "::sippet::call::OnError";
const char kOnRinging[] = "::sippet::call::OnRinging";
const char kOnEstablished[] = "::sippet::call::OnEstablished";
const char kOnHungUp[] = "::sippet::call::OnHungUp";

}  // namespace

gin::WrapperInfo CallJsWrapper::kWrapperInfo = {
    gin::kEmbedderNativeGin };

CallJsWrapper::CallJsWrapper(
    v8::Isolate* isolate,
    const scoped_refptr<Call>& call) :
  isolate_(isolate),
  call_(call),
  message_loop_(base::MessageLoop::current()),
  on_error_(this, kOnError),
  on_ringing_(this, kOnRinging),
  on_established_(this, kOnEstablished),
  on_hungup_(this, kOnHungUp) {
  call_->set_delegate(this);
}

CallJsWrapper::~CallJsWrapper() {
  call_->set_delegate(nullptr);
}

gin::Handle<CallJsWrapper> CallJsWrapper::Create(
    v8::Isolate* isolate,
    const scoped_refptr<Call>& call) {
  return gin::CreateHandle(
      isolate,
      new CallJsWrapper(isolate, call));
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
  builder.SetMethod("on", &CallJsWrapper::On);
  return builder;
}

Call::Direction CallJsWrapper::direction() const {
  return call_->direction();
}

Call::State CallJsWrapper::state() const {
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

bool CallJsWrapper::PickUp() {
  return call_->PickUp();
}

bool CallJsWrapper::Reject() {
  return call_->Reject();
}

bool CallJsWrapper::HangUp() {
  return call_->HangUp();
}

void CallJsWrapper::SendDtmf(const std::string& digits) {
  call_->SendDtmf(digits);
}

void CallJsWrapper::On(const std::string& key,
                       v8::Handle<v8::Function> function) {
  struct {
    const char *key;
    JsFunctionCall<CallJsWrapper> &function_call;
  } pairs[] = {
    { "error", on_error_ },
    { "ringing", on_ringing_ },
    { "established", on_established_ },
    { "hungUp", on_hungup_ },
  };

  for (int i = 0; i < arraysize(pairs); i++) {
    if (pairs[i].key == key) {
      pairs[i].function_call.SetFunction(isolate_, function);
      break;
    }
  }
}

void CallJsWrapper::OnError(int status_code,
    const std::string& status_text) {
  message_loop_->PostTask(FROM_HERE,
      base::Bind(&CallJsWrapper::RunError, base::Unretained(this),
          status_code, status_text));
}

void CallJsWrapper::OnRinging() {
  message_loop_->PostTask(FROM_HERE,
      base::Bind(&CallJsWrapper::RunRinging, base::Unretained(this)));
}

void CallJsWrapper::OnEstablished() {
  message_loop_->PostTask(FROM_HERE,
      base::Bind(&CallJsWrapper::RunEstablished, base::Unretained(this)));
}

void CallJsWrapper::OnHungUp() {
  message_loop_->PostTask(FROM_HERE,
      base::Bind(&CallJsWrapper::RunHungUp, base::Unretained(this)));
}

void CallJsWrapper::RunError(int status_code,
    const std::string& status_text) {
  v8::Handle<v8::Value> args[] = {
    gin::ConvertToV8(isolate_, status_code),
    gin::ConvertToV8(isolate_, status_text)
  };
  on_error_.Run(arraysize(args), args);
}

void CallJsWrapper::RunRinging() {
  on_ringing_.Run(0, nullptr);
}

void CallJsWrapper::RunEstablished() {
  on_established_.Run(0, nullptr);
}

void CallJsWrapper::RunHungUp() {
  on_hungup_.Run(0, nullptr);
}

} // namespace phone
} // namespace sippet
