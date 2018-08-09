// Copyright (c) 2018 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIPPET_BASE_THREAD_UTIL_H_
#define SIPPET_BASE_THREAD_UTIL_H_

#include "base/bind.h"
#include "base/callback.h"
#include "base/synchronization/waitable_event.h"

namespace sippet {

namespace internal {

template <typename R> inline
void PostTaskAndWaitHelper(base::WaitableEvent* event,
                           R* result,
                           const base::Callback<R(void)>& task) {
  *result = task.Run();
  event->Signal();
}

inline
void PostClosureAndWaitHelper(base::WaitableEvent* event,
                              const base::Closure& task) {
  task.Run();
  event->Signal();
}

}  // namespace internal

template <typename R> inline
R PostTaskAndWait(base::TaskRunner* task_runner,
                  const tracked_objects::Location& from_here,
                  const base::Callback<R(void)>& task) {
  R result;
  base::WaitableEvent event(
      base::WaitableEvent::ResetPolicy::AUTOMATIC,
      base::WaitableEvent::InitialState::NOT_SIGNALED);
  task_runner->PostTask(from_here,
      base::Bind(&internal::PostTaskAndWaitHelper<R>,
          &event, &result, task));
  event.Wait();
  return result;
}

template <> inline
void PostTaskAndWait<void>(base::TaskRunner* task_runner,
                           const tracked_objects::Location& from_here,
                           const base::Closure& closure) {
  base::WaitableEvent event(
      base::WaitableEvent::ResetPolicy::AUTOMATIC,
      base::WaitableEvent::InitialState::NOT_SIGNALED);
  task_runner->PostTask(from_here,
      base::Bind(&internal::PostClosureAndWaitHelper, &event, closure));
  event.Wait();
}

template <typename R> inline
R PostTaskAndWait(
    const scoped_refptr<base::SingleThreadTaskRunner>& task_runner,
    const tracked_objects::Location& from_here,
    const base::Callback<R(void)>& task) {
  return PostTaskAndWait(task_runner.get(), from_here, task);
}

template <> inline
void PostTaskAndWait<void>(
    const scoped_refptr<base::SingleThreadTaskRunner>& task_runner,
    const tracked_objects::Location& from_here,
    const base::Closure& closure) {
  PostTaskAndWait(task_runner.get(), from_here, closure);
}

}  // namespace sippet

#endif  // SIPPET_BASE_THREAD_UTIL_H_
