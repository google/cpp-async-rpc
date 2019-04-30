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
    : pending_(queue_size == 0 ? num_worker_threads : queue_size) {
  threads_.reserve(num_worker_threads);
  for (unsigned int i = 0; i < num_worker_threads; i++) {
    threads_.emplace_back([this]() {
      while (true) {
        auto f = pending_.get();
        try {
          f();
        } catch (...) {
          // Log the exception?
        }
      }
    });
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
