// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/ua/dialog_store.h"

#include "sippet/base/stl_extras.h"
#include "sippet/message/message.h"
#include "sippet/ua/dialog.h"

namespace sippet {

DialogStore::DialogStore() {
}

DialogStore::~DialogStore() {
}

scoped_refptr<Dialog> DialogStore::GenerateDialog(
    const scoped_refptr<Response> &response) {
  DialogMapType::iterator i = dialogs_.find(response->GetDialogId());
  if (dialogs_.end() != i)
    return i->second;
  scoped_refptr<Dialog> dialog(Dialog::Create(response));
  if (dialog)
    dialogs_.insert(std::make_pair(dialog->id(), dialog));
  return dialog;
}

scoped_refptr<Dialog> DialogStore::TerminateDialog(
    const scoped_refptr<Request> &request) {
  DialogMapType::iterator i = dialogs_.find(request->GetDialogId());
  if (dialogs_.end() == i)
    return NULL;
  scoped_refptr<Dialog> dialog(i->second);
  dialog->set_state(Dialog::STATE_TERMINATED);
  dialogs_.erase(i);
  return dialog;
}

void DialogStore::TerminateDialog(const scoped_refptr<Dialog> &dialog) {
  DialogMapType::iterator i = dialogs_.find(dialog->id());
  if (dialogs_.end() != i) {
    dialog->set_state(Dialog::STATE_TERMINATED);
    dialogs_.erase(i);
  }
}

void DialogStore::ConfirmDialog(const scoped_refptr<Dialog> &dialog) {
  DCHECK(dialogs_.find(dialog->id()) != dialogs_.end());
  dialog->set_state(Dialog::STATE_CONFIRMED);
}

scoped_refptr<Dialog> DialogStore::GetDialog(const Message *message) {
  DialogMapType::iterator i = dialogs_.find(message->GetDialogId());
  if (dialogs_.end() == i)
    return NULL;
  return i->second;
}

} // namespace sippet
