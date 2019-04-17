/// \file
/// \brief Reference-counting construct to for quiescent-state cleanup.
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

#ifndef ASH_USAGE_LOCK_H_
#define ASH_USAGE_LOCK_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string_view>
#include "ash/errors.h"

namespace ash {

template <typename T, typename E>
class usage_lock {
 public:
  explicit usage_lock(const char* exception_message = "")
      : message_(exception_message) {}

  ~usage_lock() { drop(); }

  void arm(T* ptr) {
    std::scoped_lock arm_drop_lock(arm_drop_mu_);
    ptr_.reset(ptr, [this](T*) {
      {
        std::scoped_lock lock(mu_);
        done_ = true;
      }
      done_cond_.notify_all();
    });
    std::scoped_lock lock(mu_);
    done_ = false;
  }

  void drop() {
    std::scoped_lock arm_drop_lock(arm_drop_mu_);
    ptr_.reset();
    std::unique_lock lock(mu_);
    done_cond_.wait(lock, [this]() { return done_; });
  }

  auto get() {
    auto res = ptr_;
    if (!res) {
      throw E(message_);
    }
    return res;
  }

  auto get_or_null() { return ptr_; }

 private:
  const char* const message_;
  std::shared_ptr<T> ptr_;
  std::mutex mu_, arm_drop_mu_;
  std::condition_variable done_cond_;
  bool done_ = true;
};

}  // namespace ash

#endif  // ASH_USAGE_LOCK_H_
