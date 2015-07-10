// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.sippet.phone;

import org.chromium.base.JNINamespace;
import org.chromium.base.CalledByNative;

import java.util.Date;

/**
 * Base Phone class.
 */
@JNINamespace("sippet::phone::android")
public class Call extends RunOnUIThread<Delegate> {
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
    Call(long instance) {
        this.instance = instance;
    };

    /**
     * Gets the current |Call| direction.
     */
    public Direction getDirection() {
        return Direction.fromInteger(nativeGetDirection(instance));
    }

    /**
     * Get the current |Call| state.
     */
    public State getState() {
        return State.fromInteger(nativeGetState(instance));
    }

    /**
     * Get the |Call| URI.
     */
    public String getUri() {
        return nativeGetUri(instance);
    }
 
    /**
     * Get the callee username or number.
     */
    public String getName() {
        return nativeGetName(instance);
    }
 
    /**
     * Get the time when the |Call| was created.
     */
    public Date getCreationTime() {
        return new Date(nativeGetCreationTime(instance));
    }
 
    /**
     * Get the time when the |Call| has started (established).
     */
    public Date getStartTime() {
        return new Date(nativeGetStartTime(instance));
    }
 
    /**
     * Get the time when the |Call| was hung up.
     */
    public Date getEndTime() {
        return new Date(nativeGetEndTime(instance));
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
        return nativePickUp(instance);
    }

    /**
     * Reject the call (only for incoming calls).
     * No effect if not in |State.RINGING| state.
     */
    public boolean reject() {
        if (getState() != State.RINGING)
            return false;
        return nativeReject(instance);
    }

    /**
     * Hang up the call.
     * No effect if not in |State.ESTABLISHED| state.
     */
    public boolean hangUp() {
        if (getState() != State.ESTABLISHED)
            return false;
        return nativeHangUp(instance);
    }    
 
    /**
     * Send DTMF digits.
     * No effect if not in |State.ESTABLISHED| state.
     */
    public void sendDtmf(String digits) {
        if (getState() != State.ESTABLISHED)
            return;
        nativeSendDtmf(instance, digits);
    }

    /**
     * Disposes the |Call| inner instance.
     */
    protected void finalize() throws Throwable {
        nativeFinalize(instance);
        super.finalize();
    }

    private long instance;

    @CalledByNative
    private void runOnError(int statusCode, String statusText) {
        post(new Runnable<Delegate>() {
            public void run(Delegate delegate) {
                delegate.onError(statusCode, statusText);
            }
        });
    }

    @CalledByNative
    private void runOnRinging() {
        post(new Runnable<Delegate>() {
            public void run(Delegate delegate) {
                delegate.onRinging();
            }
        });
    }

    @CalledByNative
    private void runOnEstablished() {
        post(new Runnable<Delegate>() {
            public void run(Delegate delegate) {
                delegate.onEstablished();
            }
        });
    }

    @CalledByNative
    private void runOnHungUp() {
        post(new Runnable<Delegate>() {
            public void run(Delegate delegate) {
                delegate.onHungUp();
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
