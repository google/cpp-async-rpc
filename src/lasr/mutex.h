/// \file
/// \brief select-friendly mutex objects.
///
/// \copyright
///   Copyright 2019 by Google LLC.
///
/// \copyright
///   Licensed under the Apache License, Version 2.0 (the "License"); you may
///   not use this file except in compliance with the License. You may obtain a
///   copy of the License at
///
/// \copyright
///   http://www.apache.org/licenses/LICENSE-2.0
///
/// \copyright
///   Unless required by applicable law or agreed to in writing, software
///   distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
///   WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
///   License for the specific language governing permissions and limitations
///   under the License.

#ifndef LASR_MUTEX_H_
#define LASR_MUTEX_H_

#include "lasr/awaitable.h"
#include "lasr/channel.h"

namespace lasr {

class mutex {
 public:
  mutex();

  void lock();
  void maybe_lock();
  bool try_lock();
  void unlock();
  awaitable<void> can_lock();
  awaitable<void> async_lock();

 private:
  channel pipe_[2];
};

}  // namespace lasr

#endif  // LASR_MUTEX_H_
