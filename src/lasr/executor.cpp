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

#include "lasr/executor.h"

namespace lasr {

thread_pool::thread_pool(unsigned int num_worker_threads, int queue_size)
    : pending_(queue_size == 0 ? num_worker_threads : queue_size),
      slots_(num_worker_threads) {
  threads_.reserve(num_worker_threads);
  for (unsigned int i = 0; i < num_worker_threads; i++) {
    threads_.emplace_back([this]() {
      while (true) {
        promise<fn_type> fn_promise;
        future<fn_type> fn_future = fn_promise.get_future();
        request_work(std::move(fn_promise));
        auto f = fn_future.get();
        try {
          f();
        } catch (...) {
          // Log the exception?
        }
      }
    });
  }
}

void thread_pool::request_work(promise<fn_type> slot) {
  while (true) {
    auto [have_pending_fn, have_slot_space] =
        select(pending_.can_get(), slots_.can_put());
    try {
      std::scoped_lock lock(mu_);
      if (have_pending_fn) {
        *have_pending_fn;
        slot.set_value(pending_.maybe_get());
        return;
      }
      if (have_slot_space) {
        *have_slot_space;
        slots_.maybe_put(std::move(slot));
        return;
      }
    } catch (const errors::try_again&) {
      // Just try again.
    }
  }
}

thread_pool::~thread_pool() {
  for (auto& t : threads_) {
    t.get_context().cancel();
  }
  for (auto& t : threads_) {
    t.join();
  }
}

}  // namespace lasr
