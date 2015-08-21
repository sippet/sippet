// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.sippet.phone;

import org.chromium.base.annotations.AccessedByNative;

import java.util.List;

/**
 * Phone settings.
 */
public class Settings {
    /**
     * Enable/disable streaming encryption.
     */
    public void setDisableEncryption(boolean value) {
        disableEncryption = value;
    }

    /**
     * Enable/disable SCTP data channels.
     */
    public void setDisableSctpDataChannels(boolean value) {
        disableSctpDataChannels = value;
    }

    /**
     * Set the ICE servers list.
     */
    public void setIceServers(List<IceServer> value) {
        iceServers = value;
    }

    /**
     * Access the list of SIP URIs and its goal is to contain all the proxies
     * that route all requests outside dialogs.
     */
    public void setRouteSet(List<String> value) {
        routeSet = value;
    }

    /**
     * SIP URI associated to the User Agent.
     * This is a SIP address given to you by your provider.
     */
    public void setUri(String value) {
        uri = value;
    }

    /**
     * If this is set then the User-Agent header will have this string appended
     * after sippet name and version.
     */ 
    public void setUserAgent(String value) {
        userAgent = value;
    }

    /**
     * Username to use when generating authentication credentials.
     * If not defined the value in uri parameter is used.
     */
    public void setAuthorizationUser(String value) {
        authorizationUser = value;
    }

    /**
     * Descriptive name to be shown to the called party when calling or
     * sending IM messages.
     * It must NOT be enclosed between double quotes.
     */
    public void setDisplayName(String value) {
        displayName = value;
    }

    /**
     * SIP Authentication password.
     */
    public void setPassword(String value) {
        password = value;
    }

    /**
     * Registration expiry time (in seconds).
     * Default value is 600.
     */
    public void setRegisterExpires(long value) {
        registerExpires = value;
    }

    /**
     * Set the SIP registrar URI.
     * Valid value is a SIP URI without username. If empty (default) then the
     * registrar URI is taken from the uri parameter (by removing the
     * username).
     */
    public void setRegistrarServer(String value) {
        registrarServer = value;
    }

    @AccessedByNative
    private List<IceServer> iceServers;

    @AccessedByNative
    private boolean disableEncryption;

    @AccessedByNative
    private boolean disableSctpDataChannels;

    @AccessedByNative
    private List<String> routeSet;

    @AccessedByNative
    private String uri;

    @AccessedByNative
    private String userAgent;

    @AccessedByNative
    private String authorizationUser;

    @AccessedByNative
    private String displayName;

    @AccessedByNative
    private String password;

    @AccessedByNative
    private long registerExpires;

    @AccessedByNative
    private String registrarServer;
}
