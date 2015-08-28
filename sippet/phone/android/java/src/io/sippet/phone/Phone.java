// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.sippet.phone;

import android.content.Context;

import org.chromium.base.Log;
import org.chromium.base.ThreadUtils;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.SuppressFBWarnings;
import org.chromium.base.library_loader.LibraryLoader;
import org.chromium.base.library_loader.LibraryProcessType;
import org.chromium.base.library_loader.ProcessInitException;

/**
 * Base Phone class.
 */
@JNINamespace("sippet::phone::android")
public class Phone {
    private static final String TAG = "Phone";

    private long mInstance;
    private Delegate mDelegate;
    private Context mContext;

    /**
     * Phone mDelegate.
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
       * Called when the internal refresh registration fails.
       */
      void onRefreshError(int statusCode, String statusText);

      /**
       * Called to inform completion of the last unregistration attempt.
       */
      void onUnregisterCompleted(int statusCode, String statusText);

      /**
       * Called on incoming calls.
       */
      void onIncomingCall(Call call);
    }

    /**
     * Create a |Phone| mInstance.
     * @param context The context to pull the application context from.
     * @param mDelegate The |Delegate| mInstance that will run phone callbacks.
     */
    public Phone(Context context, Delegate mDelegate) {
        mContext = context;
        mDelegate = mDelegate;
    }

    /**
     * Initializes the |Phone| mInstance.
     */
    @SuppressFBWarnings("DM_EXIT")
    public boolean init(Settings settings) {
        try {
            LibraryLoader libraryLoader =
                    LibraryLoader.get(LibraryProcessType.PROCESS_BROWSER);
            libraryLoader.ensureInitialized(mContext.getApplicationContext());
            // The prefetch is done after the library load for two reasons:
            // - It is easier to know the library location after it has
            //   been loaded.
            // - Testing has shown that this gives the best compromise,
            //   by avoiding performance regression on any tested
            //   device, and providing performance improvement on
            //   some. Doing it earlier delays UI inflation and more
            //   generally startup on some devices, most likely by
            //   competing for IO.
            // For experimental results, see http://crbug.com/460438.
            libraryLoader.asyncPrefetchLibrariesToMemory();
        } catch (ProcessInitException e) {
            Log.e(TAG, "Unable to load native library.", e);
            System.exit(-1);
        }
        nativeInitApplicationContext(mContext.getApplicationContext());
        mInstance = nativeCreate();
        return nativeInit(mInstance, settings);
    }

    /**
     * Get the |Phone| state.
     */
    public int getState() {
        return nativeGetState(mInstance);
    }

    /**
     * Registers the |Phone| to receive incoming requests.
     * Upon successful registration, the Phone will emit a registered event.
     */
    public boolean register() {
        return nativeRegister(mInstance);
    }

    /**
     * Unregisters the |Phone|.
     */
    public boolean unregister() {
        return nativeUnregister(mInstance);
    }

    /**
     * Unregisters all instances that registered in registrar.
     */
    public boolean unregisterAll() {
        return nativeUnregisterAll(mInstance);
    }

    /**
     * Starts a |Call| to the given destination.
     *
     * @param target   Destination of the call. String representing a
     *                 destination, username, or a complete SIP URI.
     * @return         A call object.
     */
    public Call makeCall(String target) {
        return new Call(nativeMakeCall(mInstance, target));
    }

    /**
     * Hangs up all active calls.
     */
    public void hangUpAll() {
        nativeHangUpAll(mInstance);
    }

    /**
     * Disposes the |Phone| inner mInstance.
     */
    protected void finalize() throws Throwable {
        nativeFinalize(mInstance);
        super.finalize();
    }

    @CalledByNative
    private void runOnNetworkError(final int errorCode) {
        ThreadUtils.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mDelegate.onNetworkError(errorCode);
            }
        });
    }

    @CalledByNative
    private void runOnRegisterCompleted(final int statusCode,
                                        final String statusText) {
        ThreadUtils.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mDelegate.onRegisterCompleted(statusCode, statusText);
            }
        });
    }

    @CalledByNative
    private void runOnRefreshError(final int statusCode,
                                   final String statusText) {
        ThreadUtils.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mDelegate.onRefreshError(statusCode, statusText);
            }
        });
    }

    @CalledByNative
    private void runOnUnregisterCompleted(final int statusCode,
                                          final String statusText) {
        ThreadUtils.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mDelegate.onUnregisterCompleted(statusCode, statusText);
            }
        });
    }

    @CalledByNative
    private void runOnIncomingCall(final long mInstance) {
        ThreadUtils.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mDelegate.onIncomingCall(new Call(mInstance));
            }
        });
    }

    private native void nativeInitApplicationContext(Context context);
    private native long nativeCreate();
    private native boolean nativeInit(long nativeJavaPhone, Settings settings);
    private native int nativeGetState(long nativeJavaPhone);
    private native boolean nativeRegister(long nativeJavaPhone);
    private native boolean nativeUnregister(long nativeJavaPhone);
    private native boolean nativeUnregisterAll(long nativeJavaPhone);
    private native long nativeMakeCall(long nativeJavaPhone, String target);
    private native void nativeHangUpAll(long nativeJavaPhone);
    private native void nativeFinalize(long nativeJavaPhone);
}
