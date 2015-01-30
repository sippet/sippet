// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sippet/examples/common/url_request_context_getter.h"

#include <string>

#include "net/proxy/proxy_config_service.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_builder.h"

namespace {

// Config getter that always returns direct settings.
class ProxyConfigServiceDirect : public net::ProxyConfigService {
 public:
  // Overridden from ProxyConfigService:
  virtual void AddObserver(Observer* observer) override {}
  virtual void RemoveObserver(Observer* observer) override {}
  virtual ConfigAvailability GetLatestProxyConfig(
      net::ProxyConfig* config) override {
    *config = net::ProxyConfig::CreateDirect();
    return CONFIG_VALID;
  }
};

}  // namespace

URLRequestContextGetter::URLRequestContextGetter(
    scoped_refptr<base::SingleThreadTaskRunner> network_task_runner)
    : network_task_runner_(network_task_runner) {
}

net::URLRequestContext* URLRequestContextGetter::GetURLRequestContext() {
  CHECK(network_task_runner_->BelongsToCurrentThread());
  if (!url_request_context_) {
    net::URLRequestContextBuilder builder;
    // net::HttpServer fails to parse headers if user-agent header is blank.
    builder.set_user_agent("sippet-test-client");
    builder.DisableHttpCache();
#if defined(OS_LINUX) || defined(OS_ANDROID)
    builder.set_proxy_config_service(new ProxyConfigServiceDirect());
#endif
    url_request_context_.reset(builder.Build());
  }
  return url_request_context_.get();
}

scoped_refptr<base::SingleThreadTaskRunner>
    URLRequestContextGetter::GetNetworkTaskRunner() const {
  return network_task_runner_;
}

URLRequestContextGetter::~URLRequestContextGetter() {}
