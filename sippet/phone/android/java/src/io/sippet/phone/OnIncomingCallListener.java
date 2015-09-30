// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.sippet.phone;

/**
 * Callback executed when receiving calls.
 * The function onIncomingCall will be called on the main UI thread.
 */
public interface OnIncomingCallListener {
    /**
     * Executed on incoming calls.
     */
    void onIncomingCall(Call call);
}
