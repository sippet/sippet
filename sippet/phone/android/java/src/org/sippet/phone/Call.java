// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.sippet.phone;

import org.chromium.base.ThreadUtils;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.base.annotations.CalledByNative;

import java.util.Date;

/**
 * Base Phone class.
 */
@JNINamespace("sippet::phone::android")
public class Call {
    private long mInstance;
    private Delegate mDelegate;

    /**
     * Call direction: incoming or outgoing.
     */
    enum Direction {
        INCOMING,
        OUTGOING;

        public static Direction fromInteger(int i) {
            switch (i) {
                case 0:
                    return INCOMING;
                case 1:
                    return OUTGOING;
            }
            return null;
        }
    };

    /**
     * Call state: corresponds to the |Call| lifecycle.
     */
    enum State {
        CALLING,
        RINGING,
        ESTABLISHED,
        HUNGUP,
        ERROR;

        public static State fromInteger(int i) {
            switch (i) {
                case 0:
                    return CALLING;
                case 1:
                    return RINGING;
                case 2:
                    return ESTABLISHED;
                case 3:
                    return HUNGUP;
                case 4:
                    return ERROR;
            }
            return null;
        }
    };

    /**
     * Call mDelegate.
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
     * Create a |Call| mInstance.
     */
    Call(long mInstance) {
        this.mInstance = mInstance;
    };

    /**
     * Set the |Delegate| mInstance that will run callbacks on the UI thread.
     */
    public void setDelegate(Delegate mDelegate) {
        this.mDelegate = mDelegate;
    }

    /**
     * Gets the current |Call| direction.
     */
    public Direction getDirection() {
        return Direction.fromInteger(nativeGetDirection(mInstance));
    }

    /**
     * Get the current |Call| state.
     */
    public State getState() {
        return State.fromInteger(nativeGetState(mInstance));
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
     * No effect if not in |State.RINGING| state.
     */
    public boolean pickUp() {
        if (getState() != State.RINGING)
            return false;
        return nativePickUp(mInstance);
    }

    /**
     * Reject the call (only for incoming calls).
     * No effect if not in |State.RINGING| state.
     */
    public boolean reject() {
        if (getState() != State.RINGING)
            return false;
        return nativeReject(mInstance);
    }

    /**
     * Hang up the call.
     * No effect if not in |State.ESTABLISHED| state.
     */
    public boolean hangUp() {
        if (getState() != State.ESTABLISHED)
            return false;
        return nativeHangUp(mInstance);
    }    
 
    /**
     * Send DTMF digits.
     * No effect if not in |State.ESTABLISHED| state.
     */
    public void sendDtmf(String digits) {
        if (getState() != State.ESTABLISHED)
            return;
        nativeSendDtmf(mInstance, digits);
    }

    /**
     * Disposes the |Call| inner mInstance.
     */
    protected void finalize() throws Throwable {
        nativeFinalize(mInstance);
        super.finalize();
    }

    @CalledByNative
    private void runOnError(final int statusCode, final String statusText) {
        ThreadUtils.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mDelegate.onError(statusCode, statusText);
            }
        });
    }

    @CalledByNative
    private void runOnRinging() {
        ThreadUtils.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mDelegate.onRinging();
            }
        });
    }

    @CalledByNative
    private void runOnEstablished() {
        ThreadUtils.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mDelegate.onEstablished();
            }
        });
    }

    @CalledByNative
    private void runOnHungUp() {
        ThreadUtils.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mDelegate.onHungUp();
            }
        });
    }

    private native int nativeGetDirection(long nativeJavaCall);
    private native int nativeGetState(long nativeJavaCall);
    private native String nativeGetUri(long nativeJavaCall);
    private native String nativeGetName(long nativeJavaCall);
    private native long nativeGetCreationTime(long nativeJavaCall);
    private native long nativeGetStartTime(long nativeJavaCall);
    private native long nativeGetEndTime(long nativeJavaCall);
    private native boolean nativePickUp(long nativeJavaCall);
    private native boolean nativeReject(long nativeJavaCall);
    private native boolean nativeHangUp(long nativeJavaCall);
    private native void nativeSendDtmf(long nativeJavaCall, String digits);
    private native void nativeFinalize(long nativeJavaCall);
}
