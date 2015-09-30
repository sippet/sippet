// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.sippet.phone;

import org.chromium.base.ThreadUtils;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.base.annotations.CalledByNative;

/**
 * Callbacks executed on async network executions.
 */
@JNINamespace("sippet::phone::android")
abstract public class CompletionCallback {
    /**
     * Called to inform completion of the last async attempt.
     */
    abstract public void onCompleted(int statusCode);

    @CalledByNative
    private void runOnCompleted(final int statusCode) {
        ThreadUtils.postOnUiThread(new Runnable() {
            @Override
            public void run() {
                onCompleted(statusCode);
            }
        });
    }
}
