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
public class Phone {
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
        nativePhoneInitialize();
    }

    /**
     * Create a |Phone| instance.
     */
    public Phone(Delegate delegate) {
        this.delegate = delegate;
        this.instance = nativePhoneCreate();
    }

    /**
     * Initializes the |Phone| instance.
     */
    boolean init(Settings settings) {
        nativePhoneInit(instance, settings);
    }

    /**
     * Get the current phone state.
     */
    public State getState() {
        return State.values()[nativePhoneGetState(instance)];
    }

    /**
     * Registers the Phone to receive incoming requests.
     * Upon successful registration, the Phone will emit a registered event.
     */
    public void register() {
        nativePhoneRegister(instance);
    }

    /**
     * Unregisters the UA.
     */
    public void unregister() {
        nativePhoneUnregister(instance);
    }

    /**
     * Starts a call to the given destination.
     *
     * @param target   Destination of the call. String representing a
     *                 destination, username, or a complete SIP URI.
     * @return         A call object.
     */
    public Call makeCall(String target) {
        return new Call(nativePhoneMakeCall(instance, target));
    }

    /**
     * Hangs up all active calls.
     */
    public void hangUpAll() {
        nativePhoneHangUpAll(instance);
    }

    /**
     * Disposes the |Phone| inner instance.
     */
    protected void finalize() throws Throwable {
        nativePhoneFinalize(instance);
        super.finalize();
    }

    private long instance;
    private Delegate delegate;

    @AccessedByNative
    private void runOnNetworkError(int errorCode) {
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            public void run() {
                delegate.onNetworkError(errorCode);
            }
        });
    }

    @AccessedByNative
    private void runOnRegisterCompleted(int statusCode, String statusText) {
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            public void run() {
                delegate.onRegisterCompleted(statusCode, statusText);
            }
        });
    }

    @AccessedByNative
    private void runOnIncomingCall(long instance) {
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            public void run() {
                delegate.onIncomingCall(new Call(instance));
            }
        });
    }

    private static native void nativePhoneInitialize();
    private native long nativePhoneCreate();
    private native boolean nativePhoneInit(long instance, Settings settings);
    private native long nativePhoneGetState(long instance);
    private native void nativePhoneRegister(long instance);
    private native void nativePhoneUnregister(long instance);
    private native long nativePhoneMakeCall(long instance, String target);
    private native void nativePhoneHangUpAll(long instance);
    private native void nativePhoneFinalize(long instance);
}
