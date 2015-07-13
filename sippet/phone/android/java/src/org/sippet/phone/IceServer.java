// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.sippet.phone;

import org.chromium.base.AccessedByNative;

public class IceServer {
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
    public String getUri() { return this.uri; }
    public void setUri(String uri) { this.uri = uri; }

    /**
     * STUN/TURN username.
     */
    public String getUsername() { return this.username; }
    public void setUsername(String value) { this.username = value; }

    /**
     * STUN/TURN password.
     */
    public String getPassword() { return this.password; }
    public void setPassword(String value) { this.password = value; }

    @AccessedByNative
    private String uri;

    @AccessedByNative
    private String username;

    @AccessedByNative
    private String password;
}
