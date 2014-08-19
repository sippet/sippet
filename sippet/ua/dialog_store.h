// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_UA_DIALOG_STORE_H_
#define SIPPET_UA_DIALOG_STORE_H_

#include <map>
#include "base/basictypes.h"
#include "base/memory/ref_counted.h"

namespace sippet {

class Dialog;
class Message;
class Request;
class Response;

// The |DialogStore| is responsible for generating dialogs, as well as
// terminating them. It also stores them, providing the ability to retrieve
// them whenever required.
class DialogStore {
 public:
  DialogStore();
  virtual ~DialogStore();

  // Generates a dialog through the generation or reception of non-failure
  // responses to requests with specific methods, and returns the created
  // dialog. Provisional responses generate 'early' dialogs, and final
  // non-failure responses generate 'confirmed' dialogs.
  scoped_refptr<Dialog> GenerateDialog(
      const scoped_refptr<Response> &response);

  // Terminates and returns any existing dialog matching the given request.
  // The returned dialog will be in |STATE_TERMINATED| state, if available.
  scoped_refptr<Dialog> TerminateDialog(const scoped_refptr<Request> &request);

  // Terminates an existing dialog.
  void TerminateDialog(const scoped_refptr<Dialog> &dialog);

  // Confirms an existing dialog.
  void ConfirmDialog(const scoped_refptr<Dialog> &dialog);

  // Given any message, retrieves the matching dialog.
  scoped_refptr<Dialog> GetDialog(const Message *message);

 private:
  typedef std::map<std::string, scoped_refptr<Dialog> > DialogMapType;

  DialogMapType dialogs_;

  DISALLOW_COPY_AND_ASSIGN(DialogStore);
};

} // End of sippet namespace

#endif // SIPPET_UA_DIALOG_STORE_H_
