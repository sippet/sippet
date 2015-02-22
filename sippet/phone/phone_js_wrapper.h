// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_PHONE_JS_WRAPPER_H_
#define SIPPET_PHONE_PHONE_JS_WRAPPER_H_

#include "sippet/phone/phone.h"

#include "gin/handle.h"
#include "gin/object_template_builder.h"
#include "gin/wrappable.h"

namespace sippet {
namespace phone {

class CallJsWrapper;

class PhoneJsWrapper
    : public gin::Wrappable<PhoneJsWrapper> {
 public:
  ~PhoneJsWrapper() override;
  static gin::Handle<PhoneJsWrapper> Create(
      v8::Isolate* isolate,
      Phone *phone);

  // gin::Wrappable<PhoneJsWrapper> overrides
  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) override;

  // JS interface implementation.
  Phone::State state() const;
  bool Init(const Settings& settings);
  bool Login(const Account &account);
  gin::Handle<CallJsWrapper> MakeCall(const std::string& destination);
  void HangUpAll();
  void Logout();

  static gin::WrapperInfo kWrapperInfo;
  static const char kModuleName[];

 private:
  explicit PhoneJsWrapper(
      v8::Isolate* isolate,
      const scoped_refptr<Phone>& phone);

  v8::Isolate* isolate_;
  scoped_refptr<Phone> phone_;

  DISALLOW_COPY_AND_ASSIGN(PhoneJsWrapper);
};

} // namespace phone
} // namespace sippet

#endif // SIPPET_PHONE_PHONE_JS_WRAPPER_H_
