// Copyright (c) 2014 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/basictypes.h"
#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/threading/thread.h"
#include "base/threading/thread_checker.h"
#include "base/files/file_path.h"
#include "sippet/message/protocol.h"
#include "url/gurl.h"

extern "C" {
struct pjsip_rx_data;
}

namespace sippet {

class StandaloneTestServer {
 public:
  // Container for various options to control how the SIPS or WSS server is
  // initialized.
  struct SSLOptions {
    // Initialize a new empty SSLOptions.
    SSLOptions();
    ~SSLOptions();

    // Returns the relative filename of the file that contains the
    // |server_certificate|.
    base::FilePath certificate_file;

    // Optional private key of the endpoint certificate to be used.
    base::FilePath privatekey_file;

    // Optional password to open the private key.
    std::string password;

    // True if a CertificateRequest should be sent to the client during
    // handshaking.
    bool request_client_certificate;
  };

  // Creates a SIP test server. InitializeAndWaitUntilReady() must be called
  // to start the server.
  StandaloneTestServer(const Protocol &protocol, int port = 0);
  StandaloneTestServer(const Protocol &protocol,
      const SSLOptions &ssl_options, int port = 0);
  virtual ~StandaloneTestServer();

  // Initializes and waits until the server is ready to accept requests.
  bool InitializeAndWaitUntilReady() WARN_UNUSED_RESULT;

  // Shuts down the http server and waits until the shutdown is complete.
  bool ShutdownAndWaitUntilComplete() WARN_UNUSED_RESULT;

  // Checks if the server is started.
  bool Started() const;

  // Returns the base URL to the server, which looks like:
  //
  // - sip:127.0.0.1:<port> for UDP transport
  // - sip:127.0.0.1:<port>;transport=TCP for TCP transport
  // - sips:127.0.0.1:<port> for TLS transport
  // - sip:127.0.0.1:<port>;transport=WS for Websockets transport
  //
  // where <port> is the actual port number used by the server. If <port> is
  // omited, then it's assuming the default port for the specified protocol.
  const GURL& base_uri() const { return base_uri_; }

  // Returns the port number used by the server.
  int port() const { return port_; }

  // Returns the protocol used by the server.
  const Protocol &protocol() const { return protocol_; }

 private:
  friend struct StandaloneTestServerCallbacks;

  // Initializes the StandaloneTestServer.
  void Init(const Protocol &protocol, int port,
      const SSLOptions *ssl_options);

  // Initializes and starts the server. If initialization succeeds, Starts()
  // will return true.
  void InitializeOnIOThread();

  // Shuts down the server.
  void ShutdownOnIOThread();

  // Posts a task to the |io_thread_| and waits for a reply.
  bool PostTaskToIOThreadAndWait(
      const base::Closure& closure) WARN_UNUSED_RESULT;

  // Called when a request is received.
  void OnReceiveRequest(pjsip_rx_data *rdata);

  // Verify an incoming SIP request.
  bool VerifyRequest(pjsip_rx_data *rdata);

  // Called when a response is received.
  void OnReceiveResponse(pjsip_rx_data *rdata);

  scoped_ptr<base::Thread> io_thread_;
  Protocol protocol_;
  int port_;
  GURL base_uri_;

  base::ThreadChecker thread_checker_;

  struct ControlStruct;
  scoped_ptr<ControlStruct> control_struct_;
  SSLOptions ssl_options_;

  DISALLOW_COPY_AND_ASSIGN(StandaloneTestServer);
};

} // namespace sippet
