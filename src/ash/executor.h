/// \file
/// \brief Synchronous and asynchronous function runners.
///
/// \copyright
///   Copyright 2019 by Google LLC. All Rights Reserved.
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

#ifndef ASH_EXECUTOR_H_
#define ASH_EXECUTOR_H_

#include <thread>
#include <utility>
#include <vector>
#include "ash/queue.h"
#include "ash/thread.h"
#include "function2/function2.hpp"

namespace ash {

class synchronous_executor {
 public:
  template <typename F>
  void run(F&& f) const {
    f();
  }
};

class thread_pool {
 public:
  explicit thread_pool(
      unsigned int num_worker_threads = thread::hardware_concurrency(),
      int queue_size = -1);
  ~thread_pool();

  template <typename F>
  void run(F&& f) {
    pending_.put(std::move(fn_type(std::forward<F>(f))));
  }

 private:
  using fn_type = fu2::unique_function<void()>;
  queue<fn_type> pending_;
  std::vector<daemon_thread> threads_;
};

}  // namespace ash

#endif  // ASH_EXECUTOR_H_
