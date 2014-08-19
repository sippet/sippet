// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_UA_DIALOG_CONTROLLER_H_
#define SIPPET_UA_DIALOG_CONTROLLER_H_

#include "base/memory/ref_counted.h"

namespace sippet {

class Dialog;
class Request;
class Response;
class DialogStore;

class DialogController {
 public:
  virtual ~DialogController() {}

  virtual scoped_refptr<Dialog> HandleRequest(
      DialogStore *dialog_store, const scoped_refptr<Request> &request) = 0;

  virtual scoped_refptr<Dialog> HandleResponse(
      DialogStore *dialog_store, const scoped_refptr<Response> &response) = 0;

  virtual scoped_refptr<Dialog> HandleRequestError(
      DialogStore *dialog_store, const scoped_refptr<Request> &request) = 0;

  // The default implementation is compliant with RFC 3261 only. That means it
  // will create dialogs based on INVITE responses, and destroy them based on
  // BYE requests.
  static DialogController *GetDefaultDialogController();
};

} // End of sippet namespace

#endif // SIPPET_UA_DIALOG_CONTROLLER_H_
