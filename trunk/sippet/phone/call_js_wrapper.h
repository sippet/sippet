// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_PHONE_CALL_JS_WRAPPER_H_
#define SIPPET_PHONE_CALL_JS_WRAPPER_H_

#include "sippet/phone/call.h"

#include "gin/handle.h"
#include "gin/object_template_builder.h"
#include "gin/wrappable.h"

namespace sippet {
namespace phone {

class CallJsWrapper
  : public gin::Wrappable<CallJsWrapper> {
public:
  static gin::WrapperInfo kWrapperInfo;
  ~CallJsWrapper() override;
  static gin::Handle<CallJsWrapper> Create(
      v8::Isolate* isolate,
      const scoped_refptr<Call>& call);

  // gin::Wrappable<PhoneJsWrapper> overrides
  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) override;

  // JS interface implementation.
  Call::Type type() const;
  Call::State state() const;
  std::string uri() const;
  std::string name() const;
  base::Time creation_time() const;
  base::Time start_time() const;
  base::Time end_time() const;
  base::TimeDelta duration() const;
  bool Answer(int code);
  bool HangUp();
  void SendDtmf(const std::string& digits);

private:
  explicit CallJsWrapper(
      const scoped_refptr<Call>& call);

  scoped_refptr<Call> call_;

  DISALLOW_COPY_AND_ASSIGN(CallJsWrapper);
};

} // namespace phone
} // namespace sippet

#endif // SIPPET_PHONE_CALL_JS_WRAPPER_H_
