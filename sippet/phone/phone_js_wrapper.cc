// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/phone_js_wrapper.h"
#include "sippet/phone/call_js_wrapper.h"

namespace gin {

// Extend Converter to our type Phone::State
template<>
struct Converter<sippet::phone::Phone::State> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
      sippet::phone::Phone::State val) {
    v8::Handle<v8::Integer> result(v8::Int32::New(isolate,
      static_cast<int32_t>(val)));
    return result;
  }

  static bool FromV8(v8::Isolate* isolate,
      v8::Handle<v8::Value> val,
    sippet::phone::Phone::State* out) {
    if (!val->IsInt32())
      return false;

    v8::Handle<v8::Integer> result(v8::Handle<v8::Integer>::Cast(val));
    *out = static_cast<sippet::phone::Phone::State>(result->Int32Value());
    return true;
  }
};

// Extend Converter to our type IceServer
template<>
struct Converter<sippet::phone::IceServer> {
  static const char kUri[];
  static const char kUsername[];
  static const char kPassword[];

  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
      const sippet::phone::IceServer &val) {
    v8::Handle<v8::Object> result(v8::Object::New(isolate));
    result->Set(v8::String::NewFromUtf8(isolate, kUri),
        ConvertToV8(isolate, val.uri()));
    result->Set(v8::String::NewFromUtf8(isolate, kUsername),
        ConvertToV8(isolate, val.username()));
    result->Set(v8::String::NewFromUtf8(isolate, kPassword),
        ConvertToV8(isolate, val.password()));
    return result;
  }

  static bool FromV8(v8::Isolate* isolate,
    v8::Handle<v8::Value> val,
    sippet::phone::IceServer* out) {
    if (!val->IsObject())
      return false;
    v8::Handle<v8::Object> input(v8::Handle<v8::Object>::Cast(val));
    if (!input->Has(v8::String::NewFromUtf8(isolate, kUri)))
      return false;
    out->set_uri(V8ToString(
        input->Get(v8::String::NewFromUtf8(isolate, kUri))));
    if (input->Has(v8::String::NewFromUtf8(isolate, kUsername))) {
      out->set_username(V8ToString(
          input->Get(v8::String::NewFromUtf8(isolate, kUsername))));
    }
    if (input->Has(v8::String::NewFromUtf8(isolate, kPassword))) {
      out->set_password(V8ToString(
          input->Get(v8::String::NewFromUtf8(isolate, kPassword))));
    }
    return true;
  }
};

const char Converter<sippet::phone::IceServer>::kUri[] = "uri";
const char Converter<sippet::phone::IceServer>::kUsername[] = "username";
const char Converter<sippet::phone::IceServer>::kPassword[] = "password";

// Extend Converter to our type Settings
template<>
struct Converter<sippet::phone::Settings> {
  static const char kDisableEncryption[];
  static const char kIceServers[];

  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
      const sippet::phone::Settings &val) {
    v8::Handle<v8::Object> result(v8::Object::New(isolate));
    result->Set(v8::String::NewFromUtf8(isolate, kDisableEncryption),
        ConvertToV8(isolate, val.disable_encryption()));
    result->Set(v8::String::NewFromUtf8(isolate, kIceServers),
        ConvertToV8(isolate, val.ice_servers()));
    return result;
  }

  static bool FromV8(v8::Isolate* isolate,
      v8::Handle<v8::Value> val,
      sippet::phone::Settings* out) {
    if (!val->IsObject())
      return false;
    sippet::phone::Settings settings;
    v8::Handle<v8::Object> input(v8::Handle<v8::Object>::Cast(val));
    if (input->Has(v8::String::NewFromUtf8(isolate, kDisableEncryption))) {
      bool disable_encryption = false;
      ConvertFromV8(isolate,
          input->Get(v8::String::NewFromUtf8(isolate, kDisableEncryption)),
          &disable_encryption);
      settings.set_disable_encryption(disable_encryption);
    }
    if (input->Has(v8::String::NewFromUtf8(isolate, kIceServers))) {
      sippet::phone::Settings::IceServers ice_servers;
      ConvertFromV8(isolate,
        input->Get(v8::String::NewFromUtf8(isolate, kIceServers)),
        &ice_servers);
      settings.ice_servers().swap(ice_servers);
    }
    *out = settings;
    return true;
  }
};

const char Converter<sippet::phone::Settings>::kDisableEncryption[] =
    "disable_encryption";
const char Converter<sippet::phone::Settings>::kIceServers[] =
    "ice_servers";

// Extend Converter to our type Account
template<>
struct Converter<sippet::phone::Account> {
  static const char kUsername[];
  static const char kPassword[];
  static const char kHost[];

  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
      const sippet::phone::Account &val) {
    v8::Handle<v8::Object> result(v8::Object::New(isolate));
    result->Set(v8::String::NewFromUtf8(isolate, kUsername),
        ConvertToV8(isolate, val.username()));
    result->Set(v8::String::NewFromUtf8(isolate, kPassword),
        ConvertToV8(isolate, val.password()));
    result->Set(v8::String::NewFromUtf8(isolate, kHost),
        ConvertToV8(isolate, val.host()));
    return result;
  }

  static bool FromV8(v8::Isolate* isolate,
      v8::Handle<v8::Value> val,
      sippet::phone::Account* out) {
    if (!val->IsObject())
      return false;
    v8::Handle<v8::Object> input(v8::Handle<v8::Object>::Cast(val));
    if (!input->Has(v8::String::NewFromUtf8(isolate, kUsername))
        || !input->Has(v8::String::NewFromUtf8(isolate, kPassword))
        || !input->Has(v8::String::NewFromUtf8(isolate, kHost))) {
      return false;
    }
    out->set_username(
        V8ToString(input->Get(v8::String::NewFromUtf8(isolate, kUsername))));
    out->set_password(
        V8ToString(input->Get(v8::String::NewFromUtf8(isolate, kPassword))));
    out->set_host(
        V8ToString(input->Get(v8::String::NewFromUtf8(isolate, kHost))));
    return true;
  }
};

const char Converter<sippet::phone::Account>::kUsername[] =
    "username";
const char Converter<sippet::phone::Account>::kPassword[] =
    "password";
const char Converter<sippet::phone::Account>::kHost[] =
    "host";

} // namespace gin

namespace sippet {
namespace phone {

gin::WrapperInfo PhoneJsWrapper::kWrapperInfo = {
    gin::kEmbedderNativeGin };
const char PhoneJsWrapper::kModuleName[] =
    "sippet/phone/phone";

PhoneJsWrapper::~PhoneJsWrapper() {
}

gin::Handle<PhoneJsWrapper> PhoneJsWrapper::Create(
    v8::Isolate* isolate,
    Phone *phone) {
  return gin::CreateHandle(
      isolate,
      new PhoneJsWrapper(isolate, phone));
}

gin::ObjectTemplateBuilder PhoneJsWrapper::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  gin::ObjectTemplateBuilder builder(
      Wrappable<PhoneJsWrapper>::GetObjectTemplateBuilder(isolate));
  builder.SetProperty("state", &PhoneJsWrapper::state);
  builder.SetMethod("init", &PhoneJsWrapper::Init);
  builder.SetMethod("login", &PhoneJsWrapper::Login);
  builder.SetMethod("hangupAll", &PhoneJsWrapper::HangUpAll);
  builder.SetMethod("logout", &PhoneJsWrapper::Logout);
  builder.SetMethod("makeCall", &PhoneJsWrapper::MakeCall);
  return builder;
}

Phone::State PhoneJsWrapper::state() const {
  return phone_->state();
}

bool PhoneJsWrapper::Init(const Settings& settings) {
  return phone_->Init(settings);
}

bool PhoneJsWrapper::Login(const Account &account) {
  return phone_->Login(account);
}

gin::Handle<CallJsWrapper> PhoneJsWrapper::MakeCall(
    const std::string& destination) {
  scoped_refptr<Call> call = phone_->MakeCall(destination);
  return CallJsWrapper::Create(isolate_, call);
}

void PhoneJsWrapper::HangUpAll() {
  phone_->HangUpAll();
}

void PhoneJsWrapper::Logout() {
  phone_->Logout();
}

} // namespace phone
} // namespace sippet