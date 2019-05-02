/// \file
/// \brief Synchronous and asynchronous function runners.
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

#ifndef LASR_EXECUTOR_H_
#define LASR_EXECUTOR_H_

#include <algorithm>
#include <deque>
#include <mutex>
#include <utility>
#include <vector>
#include "function2/function2.hpp"
#include "lasr/errors.h"
#include "lasr/future.h"
#include "lasr/queue.h"
#include "lasr/thread.h"

namespace lasr {

class synchronous_executor {
 public:
  template <typename F>
  void run(F&& f) const {
    f();
  }
};

class thread_pool {
 public:
  explicit thread_pool(unsigned int num_worker_threads =
                           std::max(thread::hardware_concurrency(), 1U),
                       int queue_size = -1);
  ~thread_pool();

  template <typename F>
  void run(F&& f) {
    while (true) {
      auto [have_slot, have_pending_space] =
          select(slots_.can_get(), pending_.can_put());
      try {
        std::scoped_lock lock(mu_);
        if (have_slot) {
          *have_slot;
          slots_.maybe_get().set_value(std::move(fn_type(std::forward<F>(f))));
          return;
        }
        if (have_pending_space) {
          *have_pending_space;
          pending_.maybe_put(std::move(fn_type(std::forward<F>(f))));
          return;
        }
      } catch (const errors::try_again&) {
        // Just try again.
      }
    }
  }

 private:
  using fn_type = fu2::unique_function<void()>;

  void request_work(promise<fn_type> slot);

  std::mutex mu_;
  queue<fn_type> pending_;
  queue<promise<fn_type>> slots_;
  std::vector<daemon_thread> threads_;
};

}  // namespace lasr

#endif  // LASR_EXECUTOR_H_
