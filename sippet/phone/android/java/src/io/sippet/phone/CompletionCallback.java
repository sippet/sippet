// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.sippet.phone;

/**
 * Callbacks executed on async network executions.
 */
public interface CompletionCallback {
    /**
     * Called to inform completion of the last async attempt.
     */
    void onCompleted(int statusCode);
}
