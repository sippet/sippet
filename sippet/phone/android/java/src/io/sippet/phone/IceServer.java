// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.sippet.phone;

import org.chromium.base.CalledByNative;

public class IceServer {
    private String uri;
    private String username;
    private String password;

    /**
     * Create an ICE server (STUN/TURN server).
     * It has at least a server URI.
     */
    public IceServer(String uri) {
        this.uri = uri;
    }

    /**
     * Create an ICE server (STUN/TURN server).
     * You can also pass a username and password along the server URI.
     */
    public IceServer(String uri,
                     String username,
                     String password) {
        this.uri = uri;
        this.username = username;
        this.password = password;
    }

    /**
     * ICE Server URI.
     * Example: "stun:stun.l.google.com:19302".
     */
    @CalledByNative
    public String getUri() { return this.uri; }
    public void setUri(String uri) { this.uri = uri; }

    /**
     * STUN/TURN username.
     */
    @CalledByNative
    public String getUsername() { return this.username; }
    public void setUsername(String value) { this.username = value; }

    /**
     * STUN/TURN password.
     */
    @CalledByNative
    public String getPassword() { return this.password; }
    public void setPassword(String value) { this.password = value; }
}
