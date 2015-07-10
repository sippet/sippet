// Copyright 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.sippet.phone;

/**
 * Provides a generic interface for running functions on an encapsulated
 * delegate instance.
 */
public interface Runnable<Delegate> {
    void run(Delegate delegate);
}
