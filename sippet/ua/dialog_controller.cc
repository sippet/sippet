// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/ua/dialog_controller.h"

#include "base/lazy_instance.h"
#include "sippet/message/message.h"
#include "sippet/ua/dialog.h"
#include "sippet/ua/dialog_store.h"

namespace sippet {

namespace {

class DefaultDialogController : public DialogController {
 public:
  DefaultDialogController() {}
  virtual ~DefaultDialogController() {}

  virtual scoped_refptr<Dialog> HandleRequest(
      DialogStore *store, const scoped_refptr<Request> &request) OVERRIDE;

  virtual scoped_refptr<Dialog> HandleResponse(
      DialogStore *store, const scoped_refptr<Response> &response) OVERRIDE;

  virtual scoped_refptr<Dialog> HandleRequestError(
      DialogStore *store, const scoped_refptr<Request> &request) OVERRIDE;
};

scoped_refptr<Dialog> DefaultDialogController::HandleRequest(
    DialogStore *store, const scoped_refptr<Request> &request) {
  scoped_refptr<Dialog> dialog;
  if (Method::BYE == request->method()) {
    // Terminate dialog on BYE requests
    dialog = store->TerminateDialog(request);
  }
  return dialog;
}

scoped_refptr<Dialog> DefaultDialogController::HandleResponse(
    DialogStore *store, const scoped_refptr<Response> &response) {
  Message::iterator i = response->find_first<Cseq>();
  if (response->end() == i)
    return NULL;
  Method method(dyn_cast<Cseq>(i)->method());
  scoped_refptr<Dialog> dialog;
  int response_code = response->response_code();
  // Create dialog on response_code > 100 for INVITE requests with to-tag
  if (Method::INVITE == method
      && response_code > 100
      && response->get<To>()->HasTag()) {
    dialog = store->GetDialog(response);
    if (!dialog) {
      switch (response_code/100) {
        case 1:
        case 2:
          dialog = store->GenerateDialog(response);
          break;
      }
    } else if (Dialog::STATE_CONFIRMED != dialog->state()) {
      switch (response_code/100) {
        case 1:
          break;
        case 2:
          store->ConfirmDialog(dialog);
          break;
        default:
          store->TerminateDialog(dialog);
          break;
      }
    }
  }
  return dialog;
}

scoped_refptr<Dialog> DefaultDialogController::HandleRequestError(
    DialogStore *store, const scoped_refptr<Request> &request) {
  scoped_refptr<Dialog> dialog;
  if (Message::Outgoing == request->direction()) {
    // UAC timeout or transport error
    dialog = store->TerminateDialog(request);
  }
  return dialog;
}

static base::LazyInstance<DefaultDialogController>::Leaky
  g_default_dialog_controller = LAZY_INSTANCE_INITIALIZER;

} // End of empty namespace

DialogController *DialogController::GetDefaultDialogController() {
  return g_default_dialog_controller.Pointer();
}

} // End of sippet namespace
