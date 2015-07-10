// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.sippet.phone;

import org.chromium.base.JNINamespace;
import org.chromium.base.CalledByNative;

/**
 * Base Phone class.
 */
@JNINamespace("sippet::phone::android")
public class Phone extends RunOnUIThread<Delegate> {
    public enum State {
        OFFLINE,
        CONNECTING,
        ONLINE,
    };

    /**
     * Phone delegate.
     */
    public interface Delegate {
      /**
       * Called to inform network errors, at any moment.
       */
      void onNetworkError(int errorCode);

      /**
       * Called to inform completion of the last registration attempt.
       */
      void onRegisterCompleted(int statusCode, String statusText);

      /**
       * Called on incoming calls.
       */
      void onIncomingCall(Call call);
    }

    /**
     * Initialize the |Phone| system.
     */
    public static void initialize() {
        nativeInitialize();
    }

    /**
     * Create a |Phone| instance.
     */
    public Phone() {
        this.instance = nativeCreate();
    }

    /**
     * Initializes the |Phone| instance.
     */
    boolean init(Settings settings) {
        nativeInit(instance, settings);
    }

    /**
     * Get the |Phone| state.
     */
    public State getState() {
        return State.values()[nativeGetState(instance)];
    }

    /**
     * Registers the |Phone| to receive incoming requests.
     * Upon successful registration, the Phone will emit a registered event.
     */
    public void register() {
        nativeRegister(instance);
    }

    /**
     * Unregisters the |Phone|.
     */
    public void unregister() {
        nativeUnregister(instance);
    }

    /**
     * Starts a |Call| to the given destination.
     *
     * @param target   Destination of the call. String representing a
     *                 destination, username, or a complete SIP URI.
     * @return         A call object.
     */
    public Call makeCall(String target) {
        return new Call(nativeMakeCall(instance, target));
    }

    /**
     * Hangs up all active calls.
     */
    public void hangUpAll() {
        nativeHangUpAll(instance);
    }

    /**
     * Disposes the |Phone| inner instance.
     */
    protected void finalize() throws Throwable {
        nativeFinalize(instance);
        super.finalize();
    }

    private long instance;

    @CalledByNative
    private void runOnNetworkError(int errorCode) {
        post(new Runnable<Delegate>() {
            public void run(Delegate delegate) {
                delegate.onNetworkError(errorCode);
            }
        });
    }

    @CalledByNative
    private void runOnRegisterCompleted(int statusCode, String statusText) {
        post(new Runnable<Delegate>() {
            public void run(Delegate delegate) {
                delegate.onRegisterCompleted(statusCode, statusText);
            }
        });
    }

    @CalledByNative
    private void runOnIncomingCall(long instance) {
        post(new Runnable<Delegate>() {
            public void run(Delegate delegate) {
                delegate.onIncomingCall(new Call(instance));
            }
        });
    }

    private static native void nativeInitialize();
    private native long nativeCreate();
    private native boolean nativeInit(long nativeJavaPhone, Settings settings);
    private native long nativeGetState(long nativeJavaPhone);
    private native void nativeRegister(long nativeJavaPhone);
    private native void nativeUnregister(long nativeJavaPhone);
    private native long nativeMakeCall(long nativeJavaPhone, String target);
    private native void nativeHangUpAll(long nativeJavaPhone);
    private native void nativeFinalize(long nativeJavaPhone);
}
