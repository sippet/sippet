// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/phone_js_wrapper.h"
#include "sippet/phone/call_js_wrapper.h"

#include "gin/per_context_data.h"

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

namespace {

const char kOnNetworkError[] = "::sippet::phone::OnNetworkError";
const char kOnLoginCompleted[] = "::sippet::phone::OnLoginCompleted";
const char kOnIncomingCall[] = "::sippet::phone::OnIncomingCall";
const char kOnCallError[] = "::sippet::phone::OnCallError";
const char kOnCallRinging[] = "::sippet::phone::OnCallRinging";
const char kOnCallEstablished[] = "::sippet::phone::OnCallEstablished";
const char kOnCallHungUp[] = "::sippet::phone::OnCallHungUp";

}  // namespace

gin::WrapperInfo PhoneJsWrapper::kWrapperInfo = {
    gin::kEmbedderNativeGin };

PhoneJsWrapper::PhoneJsWrapper(
    v8::Isolate* isolate) :
  isolate_(isolate),
  phone_(Phone::Create(this)),
  on_network_error_(this, kOnNetworkError),
  on_login_completed_(this, kOnLoginCompleted),
  on_incoming_call_(this, kOnIncomingCall),
  on_call_error_(this, kOnCallError),
  on_call_ringing_(this, kOnCallRinging),
  on_call_established_(this, kOnCallEstablished),
  on_call_hungup_(this, kOnCallHungUp) {
}

PhoneJsWrapper::~PhoneJsWrapper() {
}

gin::Handle<PhoneJsWrapper> PhoneJsWrapper::Create(
    v8::Isolate* isolate) {
  return gin::CreateHandle(
      isolate,
      new PhoneJsWrapper(isolate));
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

void PhoneJsWrapper::OnNetworkError(int error_code) {
  message_loop_->PostTask(FROM_HERE,
      base::Bind(&PhoneJsWrapper::RunNetworkError, base::Unretained(this),
          error_code));
}

void PhoneJsWrapper::OnLoginCompleted(int status_code,
    const std::string& status_text) {
  message_loop_->PostTask(FROM_HERE,
      base::Bind(&PhoneJsWrapper::RunLoginCompleted, base::Unretained(this),
          status_code, status_text));
}

void PhoneJsWrapper::OnIncomingCall(const scoped_refptr<Call>& call) {
  message_loop_->PostTask(FROM_HERE,
      base::Bind(&PhoneJsWrapper::RunIncomingCall, base::Unretained(this),
          call));
}

void PhoneJsWrapper::OnCallError(int status_code,
    const std::string& status_text,
    const scoped_refptr<Call>& call) {
  message_loop_->PostTask(FROM_HERE,
      base::Bind(&PhoneJsWrapper::RunCallError, base::Unretained(this),
          status_code, status_text, call));
}

void PhoneJsWrapper::OnCallRinging(const scoped_refptr<Call>& call) {
  message_loop_->PostTask(FROM_HERE,
      base::Bind(&PhoneJsWrapper::RunCallRinging, base::Unretained(this),
          call));
}

void PhoneJsWrapper::OnCallEstablished(const scoped_refptr<Call>& call) {
  message_loop_->PostTask(FROM_HERE,
      base::Bind(&PhoneJsWrapper::RunCallEstablished, base::Unretained(this),
          call));
}

void PhoneJsWrapper::OnCallHungUp(const scoped_refptr<Call>& call) {
  message_loop_->PostTask(FROM_HERE,
      base::Bind(&PhoneJsWrapper::RunCallHungUp, base::Unretained(this),
          call));
}

void PhoneJsWrapper::RunNetworkError(int error_code) {
  v8::Handle<v8::Value> args[] = {
    gin::ConvertToV8(isolate_, error_code),
  };
  on_network_error_.Run(sizeof(args)/sizeof(args[0]), args);
}

void PhoneJsWrapper::RunLoginCompleted(int status_code,
    const std::string& status_text) {
  v8::Handle<v8::Value> args[] = {
    gin::ConvertToV8(isolate_, status_code),
    gin::ConvertToV8(isolate_, status_text),
  };
  on_login_completed_.Run(sizeof(args) / sizeof(args[0]), args);
}

void PhoneJsWrapper::RunIncomingCall(const scoped_refptr<Call>& call) {
  v8::Handle<v8::Value> args[] = {
    CallJsWrapper::Create(isolate_, call).ToV8(),
  };
  on_incoming_call_.Run(sizeof(args) / sizeof(args[0]), args);
}

void PhoneJsWrapper::RunCallError(int status_code,
    const std::string& status_text,
    const scoped_refptr<Call>& call) {
  v8::Handle<v8::Value> args[] = {
    gin::ConvertToV8(isolate_, status_code),
    gin::ConvertToV8(isolate_, status_text),
    CallJsWrapper::Create(isolate_, call).ToV8(),
  };
  on_call_error_.Run(sizeof(args) / sizeof(args[0]), args);
}

void PhoneJsWrapper::RunCallRinging(const scoped_refptr<Call>& call) {
  v8::Handle<v8::Value> args[] = {
    CallJsWrapper::Create(isolate_, call).ToV8(),
  };
  on_call_ringing_.Run(sizeof(args) / sizeof(args[0]), args);
}

void PhoneJsWrapper::RunCallEstablished(const scoped_refptr<Call>& call) {
  v8::Handle<v8::Value> args[] = {
    CallJsWrapper::Create(isolate_, call).ToV8(),
  };
  on_call_established_.Run(sizeof(args) / sizeof(args[0]), args);
}

void PhoneJsWrapper::RunCallHungUp(const scoped_refptr<Call>& call) {
  v8::Handle<v8::Value> args[] = {
    CallJsWrapper::Create(isolate_, call).ToV8(),
  };
  on_call_hungup_.Run(sizeof(args) / sizeof(args[0]), args);
}

PhoneJsWrapper::FunctionCall::FunctionCall(
    PhoneJsWrapper* outer, const char *hidden_name) :
  outer_(outer),
  hidden_name_(hidden_name) {
}

PhoneJsWrapper::FunctionCall::~FunctionCall() {
}

void PhoneJsWrapper::FunctionCall::set_runner(
    const base::WeakPtr<gin::Runner>& runner) {
  runner_ = runner;
}

void PhoneJsWrapper::FunctionCall::Run(
    int argc, v8::Handle<v8::Value> *argv) {
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

// PhoneJsModule =============================================================

const char PhoneJsModule::kName[] =
    "sippet/phone";
gin::WrapperInfo PhoneJsModule::kWrapperInfo = {
    gin::kEmbedderNativeGin };

PhoneJsModule::PhoneJsModule() {
}

PhoneJsModule::~PhoneJsModule() {
}

gin::Handle<PhoneJsModule> PhoneJsModule::Create(v8::Isolate* isolate) {
  return gin::CreateHandle(isolate, new PhoneJsModule);
}

v8::Local<v8::Value> PhoneJsModule::GetModule(v8::Isolate* isolate) {
  return Create(isolate)->GetWrapper(isolate);
}

gin::ObjectTemplateBuilder PhoneJsModule::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return gin::Wrappable<PhoneJsModule>::GetObjectTemplateBuilder(isolate)
      .SetMethod("createPhone",
                 &PhoneJsWrapper::Create);
}

} // namespace phone
} // namespace sippet