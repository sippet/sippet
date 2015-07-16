// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/v8/phone_js_wrapper.h"
#include "sippet/phone/v8/call_js_wrapper.h"

namespace gin {

// Extend Converter to type GURL
template<>
struct Converter<GURL> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
      const GURL &val) {
    v8::Handle<v8::String> result(
        v8::String::NewFromUtf8(isolate, val.spec().c_str()));
    return result;
  }

  static bool FromV8(v8::Isolate* isolate,
      v8::Handle<v8::Value> val,
      GURL* out) {
    if (!val->IsString())
      return false;
    *out = GURL(gin::V8ToString(val));
    return true;
  }
};

// Extend Converter to our type Phone::State
template<>
struct Converter<sippet::phone::PhoneState> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
      sippet::phone::PhoneState val) {
    v8::Handle<v8::Integer> result(v8::Int32::New(isolate,
      static_cast<int32_t>(val)));
    return result;
  }

  static bool FromV8(v8::Isolate* isolate,
      v8::Handle<v8::Value> val,
    sippet::phone::PhoneState* out) {
    if (!val->IsInt32())
      return false;

    v8::Handle<v8::Integer> result(v8::Handle<v8::Integer>::Cast(val));
    *out = static_cast<sippet::phone::PhoneState>(result->Int32Value());
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
  static const char kRouteSet[];
  static const char kUri[];
  static const char kUserAgent[];
  static const char kAuthorizationUser[];
  static const char kDisplayName[];
  static const char kPassword[];
  static const char kRegisterExpires[];
  static const char kRegistrarServer[];

  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
      const sippet::phone::Settings &val) {
    v8::Handle<v8::Object> result(v8::Object::New(isolate));
    result->Set(v8::String::NewFromUtf8(isolate, kDisableEncryption),
        ConvertToV8(isolate, val.disable_encryption()));
    if (!val.ice_servers().empty()) {
      result->Set(v8::String::NewFromUtf8(isolate, kIceServers),
          ConvertToV8(isolate, val.ice_servers()));
    }
    if (!val.route_set().empty()) {
      result->Set(v8::String::NewFromUtf8(isolate, kRouteSet),
          ConvertToV8(isolate, val.route_set()));
    }
    if (!val.uri().is_empty()) {
      result->Set(v8::String::NewFromUtf8(isolate, kUri),
          ConvertToV8(isolate, val.uri()));
    }
    if (!val.user_agent().empty()) {
      result->Set(v8::String::NewFromUtf8(isolate, kUserAgent),
          ConvertToV8(isolate, val.user_agent()));
    }
    if (!val.authorization_user().empty()) {
      result->Set(v8::String::NewFromUtf8(isolate, kAuthorizationUser),
          ConvertToV8(isolate, val.authorization_user()));
    }
    if (!val.display_name().empty()) {
      result->Set(v8::String::NewFromUtf8(isolate, kDisplayName),
          ConvertToV8(isolate, val.display_name()));
    }
    if (!val.password().empty()) {
      result->Set(v8::String::NewFromUtf8(isolate, kPassword),
          ConvertToV8(isolate, val.password()));
    }
    result->Set(v8::String::NewFromUtf8(isolate, kRegisterExpires),
        ConvertToV8(isolate, val.register_expires()));
    if (!val.registrar_server().is_empty()) {
      result->Set(v8::String::NewFromUtf8(isolate, kRegistrarServer),
          ConvertToV8(isolate, val.registrar_server()));
    }
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
    if (input->Has(v8::String::NewFromUtf8(isolate, kRouteSet))) {
      sippet::phone::Settings::RouteSet route_set;
      ConvertFromV8(isolate,
        input->Get(v8::String::NewFromUtf8(isolate, kRouteSet)),
        &route_set);
      settings.route_set().swap(route_set);
    }
    if (input->Has(v8::String::NewFromUtf8(isolate, kUri))) {
      GURL uri;
      ConvertFromV8(isolate,
        input->Get(v8::String::NewFromUtf8(isolate, kUri)),
        &uri);
      settings.set_uri(uri);
    }
    if (input->Has(v8::String::NewFromUtf8(isolate, kUserAgent))) {
      std::string user_agent;
      ConvertFromV8(isolate,
        input->Get(v8::String::NewFromUtf8(isolate, kUserAgent)),
        &user_agent);
      settings.set_user_agent(user_agent);
    }
    if (input->Has(v8::String::NewFromUtf8(isolate, kAuthorizationUser))) {
      std::string authorization_user;
      ConvertFromV8(isolate,
        input->Get(v8::String::NewFromUtf8(isolate, kAuthorizationUser)),
        &authorization_user);
      settings.set_authorization_user(authorization_user);
    }
    if (input->Has(v8::String::NewFromUtf8(isolate, kDisplayName))) {
      std::string display_name;
      ConvertFromV8(isolate,
        input->Get(v8::String::NewFromUtf8(isolate, kDisplayName)),
        &display_name);
      settings.set_display_name(display_name);
    }
    if (input->Has(v8::String::NewFromUtf8(isolate, kPassword))) {
      std::string password;
      ConvertFromV8(isolate,
        input->Get(v8::String::NewFromUtf8(isolate, kPassword)),
        &password);
      settings.set_password(password);
    }
    if (input->Has(v8::String::NewFromUtf8(isolate, kRegisterExpires))) {
      unsigned register_expires;
      ConvertFromV8(isolate,
        input->Get(v8::String::NewFromUtf8(isolate, kRegisterExpires)),
        &register_expires);
      settings.set_register_expires(register_expires);
    }
    if (input->Has(v8::String::NewFromUtf8(isolate, kRegistrarServer))) {
      GURL registrar_server;
      ConvertFromV8(isolate,
        input->Get(v8::String::NewFromUtf8(isolate, kRegistrarServer)),
        &registrar_server);
      settings.set_registrar_server(registrar_server);
    }
    *out = settings;
    return true;
  }
};

const char Converter<sippet::phone::Settings>::kDisableEncryption[] =
    "disable_encryption";
const char Converter<sippet::phone::Settings>::kIceServers[] =
    "ice_servers";
const char Converter<sippet::phone::Settings>::kRouteSet[] =
    "route_set";
const char Converter<sippet::phone::Settings>::kUserAgent[] =
    "user_agent";
const char Converter<sippet::phone::Settings>::kUri[] =
    "uri";
const char Converter<sippet::phone::Settings>::kAuthorizationUser[] =
    "authorization_user";
const char Converter<sippet::phone::Settings>::kDisplayName[] =
    "display_name";
const char Converter<sippet::phone::Settings>::kPassword[] =
    "password";
const char Converter<sippet::phone::Settings>::kRegisterExpires[] =
    "register_expires";
const char Converter<sippet::phone::Settings>::kRegistrarServer[] =
    "registrar_server";

} // namespace gin

namespace sippet {
namespace phone {

namespace {

const char kOnNetworkError[] = "::sippet::phone::OnNetworkError";
const char kOnRegisterCompleted[] = "::sippet::phone::OnRegisterCompleted";
const char kOnRefreshError[] = "::sippet::phone::OnRefreshError";
const char kOnUnregisterCompleted[] = "::sippet::phone::OnUnregisterCompleted";
const char kOnIncomingCall[] = "::sippet::phone::OnIncomingCall";

}  // namespace

gin::WrapperInfo PhoneJsWrapper::kWrapperInfo = {
    gin::kEmbedderNativeGin };

PhoneJsWrapper::PhoneJsWrapper(
    v8::Isolate* isolate) :
  isolate_(isolate),
  phone_(Phone::Create(this)),
  message_loop_(base::MessageLoop::current()),
  on_network_error_(this, kOnNetworkError),
  on_register_completed_(this, kOnRegisterCompleted),
  on_refresh_error_(this, kOnRefreshError),
  on_unregister_completed_(this, kOnUnregisterCompleted),
  on_incoming_call_(this, kOnIncomingCall) {
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
  builder.SetMethod("register", &PhoneJsWrapper::Register);
  builder.SetMethod("unregister", &PhoneJsWrapper::Unregister);
  builder.SetMethod("unregisterAll", &PhoneJsWrapper::UnregisterAll);
  builder.SetMethod("hangupAll", &PhoneJsWrapper::HangUpAll);
  builder.SetMethod("makeCall", &PhoneJsWrapper::MakeCall);
  builder.SetMethod("on", &PhoneJsWrapper::On);
  return builder;
}

PhoneState PhoneJsWrapper::state() const {
  return phone_->state();
}

bool PhoneJsWrapper::Init(gin::Arguments args) {
  if (args.Length() != 1)
    return false;

  sippet::phone::Settings settings;
  if (!args.GetNext(&settings))
    return false;

  return phone_->Init(settings);
}

bool PhoneJsWrapper::Register() {
  return phone_->Register();
}

bool PhoneJsWrapper::Unregister() {
  return phone_->Unregister();
}

bool PhoneJsWrapper::UnregisterAll() {
  return phone_->UnregisterAll();
}

gin::Handle<CallJsWrapper> PhoneJsWrapper::MakeCall(
    const std::string& destination) {
  scoped_refptr<Call> call = phone_->MakeCall(destination);
  return CallJsWrapper::Create(isolate_, call);
}

void PhoneJsWrapper::HangUpAll() {
  phone_->HangUpAll();
}

void PhoneJsWrapper::On(const std::string& key,
                        v8::Handle<v8::Function> function) {
  struct {
    const char *key;
    JsFunctionCall<PhoneJsWrapper> &function_call;
  } pairs[] = {
    { "networkError", on_network_error_ },
    { "registerCompleted", on_register_completed_ },
    { "refreshError", on_refresh_error_ },
    { "unregisterCompleted", on_unregister_completed_ },
    { "incomingCall", on_incoming_call_ },
  };

  for (int i = 0; i < arraysize(pairs); i++) {
    if (pairs[i].key == key) {
      pairs[i].function_call.SetFunction(isolate_, function);
      break;
    }
  }
}

void PhoneJsWrapper::OnNetworkError(int error_code) {
  message_loop_->PostTask(FROM_HERE,
      base::Bind(&PhoneJsWrapper::RunNetworkError, base::Unretained(this),
          error_code));
}

void PhoneJsWrapper::OnRegisterCompleted(int status_code,
    const std::string& status_text) {
  message_loop_->PostTask(FROM_HERE,
      base::Bind(&PhoneJsWrapper::RunRegisterCompleted, base::Unretained(this),
          status_code, status_text));
}

void PhoneJsWrapper::OnRefreshError(int status_code,
    const std::string& status_text) {
  message_loop_->PostTask(FROM_HERE,
      base::Bind(&PhoneJsWrapper::RunRefreshError, base::Unretained(this),
          status_code, status_text));
}

void PhoneJsWrapper::OnUnregisterCompleted(int status_code,
    const std::string& status_text) {
  message_loop_->PostTask(FROM_HERE,
      base::Bind(&PhoneJsWrapper::RunUnregisterCompleted, base::Unretained(this),
          status_code, status_text));
}

void PhoneJsWrapper::OnIncomingCall(const scoped_refptr<Call>& call) {
  message_loop_->PostTask(FROM_HERE,
      base::Bind(&PhoneJsWrapper::RunIncomingCall, base::Unretained(this),
          call));
}

void PhoneJsWrapper::RunNetworkError(int error_code) {
  v8::Handle<v8::Value> args[] = {
    gin::ConvertToV8(isolate_, error_code),
  };
  on_network_error_.Run(arraysize(args), args);
}

void PhoneJsWrapper::RunRegisterCompleted(int status_code,
    const std::string& status_text) {
  v8::Handle<v8::Value> args[] = {
    gin::ConvertToV8(isolate_, status_code),
    gin::ConvertToV8(isolate_, status_text),
  };
  on_register_completed_.Run(arraysize(args), args);
}

void PhoneJsWrapper::RunRefreshError(int status_code,
    const std::string& status_text) {
  v8::Handle<v8::Value> args[] = {
    gin::ConvertToV8(isolate_, status_code),
    gin::ConvertToV8(isolate_, status_text),
  };
  on_refresh_error_.Run(arraysize(args), args);
}

void PhoneJsWrapper::RunUnregisterCompleted(int status_code,
    const std::string& status_text) {
  v8::Handle<v8::Value> args[] = {
    gin::ConvertToV8(isolate_, status_code),
    gin::ConvertToV8(isolate_, status_text),
  };
  on_unregister_completed_.Run(arraysize(args), args);
}

void PhoneJsWrapper::RunIncomingCall(const scoped_refptr<Call>& call) {
  v8::Handle<v8::Value> args[] = {
    CallJsWrapper::Create(isolate_, call).ToV8(),
  };
  on_incoming_call_.Run(arraysize(args), args);
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
      .SetMethod("Phone",
                 &PhoneJsWrapper::Create);
}

} // namespace phone
} // namespace sippet
