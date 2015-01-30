// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_EXAMPLES_COMMON_URL_REQUEST_CONTEXT_GETTER_H_
#define SIPPET_EXAMPLES_COMMON_URL_REQUEST_CONTEXT_GETTER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "net/url_request/url_request_context_getter.h"

namespace base {
class SingleThreadTaskRunner;
}

namespace net {
class URLRequestContext;
}

class URLRequestContextGetter : public net::URLRequestContextGetter {
 public:
  explicit URLRequestContextGetter(
      scoped_refptr<base::SingleThreadTaskRunner> network_task_runner);

  // Overridden from net::URLRequestContextGetter:
  virtual net::URLRequestContext* GetURLRequestContext() OVERRIDE;
  virtual scoped_refptr<base::SingleThreadTaskRunner>
      GetNetworkTaskRunner() const OVERRIDE;

 private:
  virtual ~URLRequestContextGetter();

  scoped_refptr<base::SingleThreadTaskRunner> network_task_runner_;

  // Only accessed on the IO thread.
  scoped_ptr<net::URLRequestContext> url_request_context_;

  DISALLOW_COPY_AND_ASSIGN(URLRequestContextGetter);
};

#endif  // SIPPET_EXAMPLES_COMMON_URL_REQUEST_CONTEXT_GETTER_H_

