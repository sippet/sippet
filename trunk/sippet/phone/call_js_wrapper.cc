// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/call_js_wrapper.h"

namespace gin {

// Extend Converter to our type Call::Type
template<>
struct Converter<sippet::phone::Call::Type> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
      sippet::phone::Call::Type val) {
    v8::Handle<v8::Integer> result(v8::Int32::New(isolate,
        static_cast<int32_t>(val)));
    return result;
  }

  static bool FromV8(v8::Isolate* isolate,
      v8::Handle<v8::Value> val,
      sippet::phone::Call::Type* out) {
    if (!val->IsInt32())
      return false;

    v8::Handle<v8::Integer> result(v8::Handle<v8::Integer>::Cast(val));
    *out = static_cast<sippet::phone::Call::Type>(result->Int32Value());
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

gin::WrapperInfo CallJsWrapper::kWrapperInfo = {
    gin::kEmbedderNativeGin };

CallJsWrapper::CallJsWrapper(
    const scoped_refptr<Call>& call)
  : call_(call) {
}

CallJsWrapper::~CallJsWrapper() {
}

gin::Handle<CallJsWrapper> CallJsWrapper::Create(
    v8::Isolate* isolate,
    const scoped_refptr<Call>& call) {
  return gin::CreateHandle(
      isolate,
      new CallJsWrapper(call));
}

gin::ObjectTemplateBuilder CallJsWrapper::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  gin::ObjectTemplateBuilder builder(
      Wrappable<CallJsWrapper>::GetObjectTemplateBuilder(isolate));
  builder.SetProperty("type", &CallJsWrapper::type);
  builder.SetProperty("state", &CallJsWrapper::state);
  builder.SetProperty("uri", &CallJsWrapper::uri);
  builder.SetProperty("name", &CallJsWrapper::name);
  builder.SetProperty("creationTime", &CallJsWrapper::creation_time);
  builder.SetProperty("startTime", &CallJsWrapper::start_time);
  builder.SetProperty("endTime", &CallJsWrapper::end_time);
  builder.SetProperty("duration", &CallJsWrapper::duration);
  builder.SetMethod("answer", &CallJsWrapper::Answer);
  builder.SetMethod("hangup", &CallJsWrapper::HangUp);
  builder.SetMethod("sendDtmf", &CallJsWrapper::SendDtmf);
  return builder;
}

Call::Type CallJsWrapper::type() const {
  return call_->type();
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

bool CallJsWrapper::Answer(int code) {
  return call_->Answer(code);
}

bool CallJsWrapper::HangUp() {
  return call_->HangUp();
}

void CallJsWrapper::SendDtmf(const std::string& digits) {
  call_->SendDtmf(digits);
}

} // namespace phone
} // namespace sippet