// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_SIPPET_EXPORT_H_
#define SIPPET_SIPPET_EXPORT_H_

#if defined(COMPONENT_BUILD)
#if defined(WIN32)

#if defined(SIPPET_IMPLEMENTATION)
#define SIPPET_EXPORT __declspec(dllexport)
#define SIPPET_EXPORT_PRIVATE __declspec(dllexport)
#else
#define SIPPET_EXPORT __declspec(dllimport)
#define SIPPET_EXPORT_PRIVATE __declspec(dllimport)
#endif  // defined(SIPPET_IMPLEMENTATION)

#else  // defined(WIN32)
#if defined(SIPPET_IMPLEMENTATION)
#define SIPPET_EXPORT __attribute__((visibility("default")))
#define SIPPET_EXPORT_PRIVATE __attribute__((visibility("default")))
#else
#define SIPPET_EXPORT
#define SIPPET_EXPORT_PRIVATE
#endif  // defined(SIPPET_IMPLEMENTATION)
#endif

#else  // defined(COMPONENT_BUILD)
#define SIPPET_EXPORT
#define SIPPET_EXPORT_EXPORT
#endif

#endif  // SIPPET_SIPPET_EXPORT_H_
