// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.sippet.phone;

import org.chromium.base.annotations.CalledByNative;

import java.util.List;

/**
 * Phone settings.
 */
public class Settings {
    private List<IceServer> mIceServers;
    private boolean mDisableEncryption = false;
    private boolean mDisableSctpDataChannels = false;
    private List<String> mRouteSet;
    private String mUri;
    private String mUserAgent;
    private String mAuthorizationUser;
    private String mDisplayName;
    private String mPassword;
    private long mRegisterExpires = 600;
    private String mRegistrarServer;

    /**
     * Enable/disable streaming encryption.
     */
    @CalledByNative
    public boolean getDisableEncryption() {
        return mDisableEncryption;
    }
    public void setDisableEncryption(boolean value) {
        mDisableEncryption = value;
    }

    /**
     * Enable/disable SCTP data channels.
     */
    @CalledByNative
    public boolean getDisableSctpDataChannels() {
        return mDisableSctpDataChannels;
    }
    public void setDisableSctpDataChannels(boolean value) {
        mDisableSctpDataChannels = value;
    }

    /**
     * Set the ICE servers list.
     */
    @CalledByNative
    public List<IceServer> getIceServers() {
        return mIceServers;
    }
    public void setIceServers(List<IceServer> value) {
        mIceServers = value;
    }

    /**
     * Access the list of SIP URIs and its goal is to contain all the proxies
     * that route all requests outside dialogs.
     */
    @CalledByNative
    public List<String> getRouteSet() {
        return mRouteSet;
    }
    public void setRouteSet(List<String> value) {
        mRouteSet = value;
    }

    /**
     * SIP URI associated to the User Agent.
     * This is a SIP address given to you by your provider.
     */
    @CalledByNative
    public String getUri() {
        return mUri;
    }
    public void setUri(String value) {
        mUri = value;
    }

    /**
     * If this is set then the User-Agent header will have this string appended
     * after sippet name and version.
     */ 
    @CalledByNative
    public String getUserAgent() {
        return mUserAgent;
    }
    public void setUserAgent(String value) {
        mUserAgent = value;
    }

    /**
     * Username to use when generating authentication credentials.
     * If not defined the value in uri parameter is used.
     */
    @CalledByNative
    public String getAuthorizationUser() {
        return mAuthorizationUser;
    }
    public void setAuthorizationUser(String value) {
        mAuthorizationUser = value;
    }

    /**
     * Descriptive name to be shown to the called party when calling or
     * sending IM messages.
     * It must NOT be enclosed between double quotes.
     */
    @CalledByNative
    public String getDisplayName() {
        return mDisplayName;
    }
    public void setDisplayName(String value) {
        mDisplayName = value;
    }

    /**
     * SIP Authentication password.
     */
    @CalledByNative
    public String getPassword() {
        return mPassword;
    }
    public void setPassword(String value) {
        mPassword = value;
    }

    /**
     * Registration expiry time (in seconds).
     * Default value is 600.
     */
    @CalledByNative
    public long getRegisterExpires() {
        return mRegisterExpires;
    }
    public void setRegisterExpires(long value) {
        mRegisterExpires = value;
    }

    /**
     * Set the SIP registrar URI.
     * Valid value is a SIP URI without username. If empty (default) then the
     * registrar URI is taken from the uri parameter (by removing the
     * username).
     */
    @CalledByNative
    public String getRegistrarServer() {
        return mRegistrarServer;
    }
    public void setRegistrarServer(String value) {
        mRegistrarServer = value;
    }
}
