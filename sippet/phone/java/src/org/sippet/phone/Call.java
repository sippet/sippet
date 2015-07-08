// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.sippet.phone;

import org.chromium.base.JNINamespace;
import org.chromium.base.AccessedByNative;

/**
 * Base Phone class.
 */
@JNINamespace("sippet::phone")
public class Call {
    /**
     * Call direction: incoming or outgoing.
     */
    enum Direction {
        INCOMING = 0,
        OUTGOING = 1
    };

    /**
     * Call state: corresponds to the |Call| lifecycle.
     */
    enum State {
        CALLING = 0,
        RINGING = 1,
        ESTABLISHED = 2,
        HUNGUP = 3,
        ERROR = 4
    };

    /**
     * Call delegate.
     */
    public interface Delegate {
      /**
       * Called to inform a |Call| error.
       */
      void onError(int statusCode, String statusText);

      /**
       * Called when callee phone starts ringing.
       */
      void onRinging();

      /**
       * Called when callee picks up the phone.
       */
      void onEstablished();

      /**
       * Called when callee hangs up.
       */
      void onHungUp();
    }

    /**
     * Create a |Call| instance.
     */
    protected Call(long instance) {
        this.instance = instance;
    };

    /**
     * Sets the |Delegate| instance that will receive the callbacks.
     */
    public void setDelegate(Delegate delegate) {
        this.delegate = delegate;
    }

    /**
     * Gets the current |Call| direction.
     */
    public Direction getDirection() {
        return Direction.values()[nativeCallGetDirection(instance)];
    }

    /**
     * Get the current |Call| state.
     */
    public State getState() {
        return State.values()[nativeCallGetState(instance)];
    }

    /**
     * Get the |Call| URI.
     */
    public String getUri() {
        return nativeCallGetUri(instance);
    }
 
    /**
     * Get the callee username or number.
     */
    public String getName() {
        return nativeCallGetName(instance);
    }
 
    /**
     * Get the time when the |Call| was created.
     */
    public Date getCreationTime() {
        return new Date(nativeCallGetCreationTime(instance));
    }
 
    /**
     * Get the when the |Call| has started (established).
     */
    public Date getStartTime() {
        return new Date(nativeCallGetStartTime(instance));
    }
 
    /**
     * Get the time when the |Call| was hung up.
     */
    public Date getEndTime() {
        return new Date(nativeCallGetEndTime(instance));
    }
 
    /**
     * Get the duration of the |Call| in milliseconds.
     */
    public long duration() {
        return getEndTime().getTime() - getStartTime().getTime();
    }
 
    /**
     * Pick up the call (only for incoming calls).
     * No effect if not in |State.RINGING| state.
     */
    public boolean pickUp() {
        if (getState() != State.RINGING)
            return false;
        return nativeCallPickUp(instance);
    }

    /**
     * Reject the call (only for incoming calls).
     * No effect if not in |State.RINGING| state.
     */
    public boolean reject() {
        if (getState() != State.RINGING)
            return false;
        return nativeCallReject(instance);
    }

    /**
     * Hang up the call.
     * No effect if not in |State.ESTABLISHED| state.
     */
    public boolean hangUp() {
        if (getState() != State.ESTABLISHED)
            return false;
        return nativeCallHangUp(instance);
    }    
 
    /**
     * Send DTMF digits.
     * No effect if not in |State.ESTABLISHED| state.
     */
    public void sendDtmf(String digits) {
        if (getState() != State.ESTABLISHED)
            return;
        nativeCallSendDtmf(instance, digits);
    }

    /**
     * Disposes the |Call| inner instance.
     */
    protected void finalize() throws Throwable {
        nativeCallFinalize(instance);
        super.finalize();
    }

    private long instance;
    private Delegate delegate;

    @AccessedByNative
    private void runOnError(int statusCode, String statusText) {
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            public void run() {
                delegate.onError(statusCode, statusText);
            }
        });
    }

    @AccessedByNative
    private void runOnRinging() {
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            public void run() {
                delegate.onRinging();
            }
        });
    }

    @AccessedByNative
    private void runOnEstablished() {
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            public void run() {
                delegate.onEstablished();
            }
        });
    }

    @AccessedByNative
    private void runOnHungUp() {
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            public void run() {
                delegate.onHungUp();
            }
        });
    }

    private native long nativeCallGetDirection(long instance);
    private native long nativeCallGetState(long instance);
    private native String nativeCallGetUri(long instance);
    private native String nativeCallGetName(long instance);
    private native long nativeCallGetCreationTime(long instance);
    private native long nativeCallGetStartTime(long instance);
    private native long nativeCallGetEndTime(long instance);
    private native boolean nativeCallPickUp(long instance);
    private native boolean nativeCallReject(long instance);
    private native boolean nativeCallHangUp(long instance);
    private native void nativeCallSendDtmf(long instance, String digits);
    private native void nativeCallFinalize(long instance);
}
