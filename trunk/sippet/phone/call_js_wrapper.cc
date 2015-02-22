// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/phone/call_js_wrapper.h"

namespace sippet {
namespace phone {

gin::WrapperInfo CallJsWrapper::kWrapperInfo = {
    gin::kEmbedderNativeGin };
const char CallJsWrapper::kModuleName[] =
    "sippet/phone/call";

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
#if 0
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
#endif
  return builder;
}

} // namespace phone
} // namespace sippet