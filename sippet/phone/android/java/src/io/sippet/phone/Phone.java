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

    private static boolean mInitialized = false;
    private long mInstance;
    private OnIncomingCallListener mListener;

    /**
     * Load Sippet libraries on memory.
     * It should be called at least once before creating a |Phone| instance.
     * @param context The context to pull the application context from.
     */
    @SuppressFBWarnings("DM_EXIT")
    public static void loadLibraries(Context context) {
        if (!mInitialized) {
            try {
                LibraryLoader libraryLoader =
                        LibraryLoader.get(LibraryProcessType.PROCESS_BROWSER);
                libraryLoader.ensureInitialized(context.getApplicationContext());
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
            nativeInitApplicationContext(context.getApplicationContext());
            mInitialized = true;
        }
    }

    /**
     * Create a |Phone| instance.
     * @param listener The |OnIncomingCallListener| instance that will handle
     *        the incoming calls.
     */
    public Phone(OnIncomingCallListener listener) {
        assert listener != null;
        mListener = listener;
        mInstance = nativeCreate();
    }

    /**
     * Initializes the |Phone| instance.
     */
    public boolean init(Settings settings) {
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
    public void register(CompletionCallback callback) {
        assert callback != null;
        nativeRegister(mInstance, callback);
    }

    /**
     * Unregisters the |Phone|.
     */
    public void unregister(CompletionCallback callback) {
        assert callback != null;
        nativeUnregister(mInstance, callback);
    }

    /**
     * Unregisters all instances registered on the current Registrar with the
     * same identity of the |Phone|.
     */
    public void unregisterAll(CompletionCallback callback) {
        assert callback != null;
        nativeUnregisterAll(mInstance, callback);
    }

    /**
     * Starts a |Call| to the given destination.
     *
     * @param target   Destination of the call. String representing a
     *                 destination, username, or a complete SIP URI.
     * @return         A call object.
     */
    public Call makeCall(String target, CompletionCallback callback) {
        assert callback != null;
        return new Call(nativeMakeCall(mInstance, target, callback));
    }

    /**
     * Disposes the |Phone| inner instance.
     */
    protected void finalize() throws Throwable {
        nativeFinalize(mInstance);
        super.finalize();
    }

    @CalledByNative
    private void runOnIncomingCall(final long nativeCall) {
        ThreadUtils.postOnUiThread(new Runnable() {
            @Override
            public void run() {
                mListener.onIncomingCall(new Call(nativeCall));
            }
        });
    }

    private static native void nativeInitApplicationContext(Context context);
    private native long nativeCreate();
    private native boolean nativeInit(long nativeJavaPhone, Settings settings);
    private native int nativeGetState(long nativeJavaPhone);
    private native void nativeRegister(long nativeJavaPhone,
                                       CompletionCallback callback);
    private native void nativeStartRefreshRegister(long nativeJavaPhone,
                                                   CompletionCallback callback);
    private native void nativeStopRefreshRegister(long nativeJavaPhone);
    private native void nativeUnregister(long nativeJavaPhone,
                                         CompletionCallback callback);
    private native void nativeUnregisterAll(long nativeJavaPhone,
                                            CompletionCallback callback);
    private native long nativeMakeCall(long nativeJavaPhone, String target,
                                       CompletionCallback callback);
    private native void nativeFinalize(long nativeJavaPhone);
}
