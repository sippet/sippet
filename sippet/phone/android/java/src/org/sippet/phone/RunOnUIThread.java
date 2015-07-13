// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.sippet.phone;

import android.os.Handler;
import android.os.Looper;

/**
 * Provides a |post| method that delegate executions to the UI thread.
 * The delegated instance will be checked before executing.
 */
abstract class RunOnUIThread<Delegate> {
    /**
     * Set the |Delegate| instance that will run callbacks on the UI thread.
     */
    public void setDelegate(Delegate delegate) {
        this.delegate = delegate;
    }

    private Delegate getDelegate() {
        return delegate;
    }

    private Delegate delegate; // The Delegate instance that runs callbacks

    /**
     * Use this function to delegate the |Runnable| execution to the UI thread.
     */
    protected void post(final Runnable<Delegate> runnable) {
        new Handler(Looper.getMainLooper()).post(new java.lang.Runnable() {
            public void run() {
                Delegate delegate = getDelegate();
                if (delegate != null)
                    runnable.run(delegate);
            }
        });
    }
}
