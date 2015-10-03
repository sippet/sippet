// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.sippet.phone;

import org.chromium.base.ThreadUtils;
import org.chromium.base.annotations.JNINamespace;

import java.util.Date;

/**
 * Base Phone class.
 */
@JNINamespace("sippet::phone::android")
public class Call {
    private long mInstance;

    /**
     * Create a |Call| instance.
     */
    Call(long instance) {
        this.mInstance = instance;
    };

    /**
     * Gets the current |Call| direction.
     */
    public int getDirection() {
        return nativeGetDirection(mInstance);
    }

    /**
     * Get the current |Call| state.
     */
    public int getState() {
        return nativeGetState(mInstance);
    }

    /**
     * Get the |Call| URI.
     */
    public String getUri() {
        return nativeGetUri(mInstance);
    }

    /**
     * Get the callee username or number.
     */
    public String getName() {
        return nativeGetName(mInstance);
    }

    /**
     * Get the time when the |Call| was created.
     */
    public Date getCreationTime() {
        return new Date(nativeGetCreationTime(mInstance));
    }

    /**
     * Get the time when the |Call| has started (established).
     */
    public Date getStartTime() {
        return new Date(nativeGetStartTime(mInstance));
    }

    /**
     * Get the time when the |Call| was hung up.
     */
    public Date getEndTime() {
        return new Date(nativeGetEndTime(mInstance));
    }

    /**
     * Get the duration of the |Call| in milliseconds.
     */
    public long duration() {
        return getEndTime().getTime() - getStartTime().getTime();
    }

    /**
     * Pick up the call (only for incoming calls).
     * No effect if not in |CallState.RINGING| state.
     */
    public void pickUp(CompletionCallback callback) {
        assert callback != null;
        nativePickUp(mInstance, callback);
    }

    /**
     * Reject the call (only for incoming calls).
     * No effect if not in |CallState.RINGING| state. The call is rejected with
     * status |CompletionStatus.ERR_SIP_BUSY_HERE|.
     */
    public void reject() {
        nativeReject(mInstance);
    }

    /**
     * Hang up the call.
     * No effect if not in |CallState.ESTABLISHED| state.
     */
    public void hangUp(CompletionCallback callback) {
        assert callback != null;
        nativeHangUp(mInstance, callback);
    }

    /**
     * Send DTMF digits.
     * No effect if not in |CallState.ESTABLISHED| state.
     */
    public void sendDtmf(String digits) {
        nativeSendDtmf(mInstance, digits);
    }

    /**
     * Disposes the |Call| inner instance.
     */
    protected void finalize() throws Throwable {
        nativeFinalize(mInstance);
        super.finalize();
    }

    private native int nativeGetDirection(long nativeJavaCall);
    private native int nativeGetState(long nativeJavaCall);
    private native String nativeGetUri(long nativeJavaCall);
    private native String nativeGetName(long nativeJavaCall);
    private native long nativeGetCreationTime(long nativeJavaCall);
    private native long nativeGetStartTime(long nativeJavaCall);
    private native long nativeGetEndTime(long nativeJavaCall);
    private native void nativePickUp(long nativeJavaCall,
                                     CompletionCallback callback);
    private native void nativeReject(long nativeJavaCall);
    private native void nativeHangUp(long nativeJavaCall,
                                     CompletionCallback callback);
    private native void nativeSendDtmf(long nativeJavaCall, String digits);
    private native void nativeFinalize(long nativeJavaCall);
}
