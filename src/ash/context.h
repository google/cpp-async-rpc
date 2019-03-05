/// \file
/// \brief Execution context with deadlines and cancellation.
///
/// \copyright
///   Copyright 2018 by Google Inc. All Rights Reserved.
///
/// \copyright
///   Licensed under the Apache License, Version 2.0 (the "License"); you may
///   not use this channel except in compliance with the License. You may obtain
///   a copy of the License at
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

#ifndef ASH_CONTEXT_H_
#define ASH_CONTEXT_H_

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include "ash/awaitable.h"
#include "ash/container/flat_map.h"
#include "ash/container/flat_set.h"
#include "ash/dynamic_base_class.h"
#include "ash/flag.h"
#include "ash/serializable.h"

namespace ash {

class context : public serializable<context> {
 public:
  ASH_CUSTOM_SERIALIZATION_VERSION(1);

  template <typename E>
  void save(E& e) const {
    std::int64_t millis_left = deadline_left() / std::chrono::milliseconds(1);
    e(millis_left);
    std::vector<std::shared_ptr<const dynamic_base_class>> v;
    v.reserve(data_.size());
    for (auto it : data_) {
      v.push_back(it.second);
    }
    e(v);
  }

  template <typename D>
  void load(D& d) {
    std::int64_t millis_left;
    std::vector<std::shared_ptr<dynamic_base_class>> v;
    d(millis_left);
    d(v);
    deadline_ = std::min(deadline_, std::chrono::system_clock::now() +
                                        std::chrono::milliseconds(millis_left));
    data_.clear();
    for (auto ptr : v) {
      data_[ptr->portable_class_name()] = std::move(ptr);
    }
  }

  using time_point = std::chrono::system_clock::time_point;
  using duration = std::chrono::system_clock::duration;

  ~context();

  void cancel();

  bool is_cancelled() const;

  time_point deadline() const;

  duration deadline_left() const;

  awaitable<void> wait_cancelled();

  awaitable<void> wait_deadline();

  static context& current();

  static context& top();

  static context with_deadline(time_point when);

  static context with_timeout(duration timeout);

  static context with_cancel();

  template <typename T>
  std::shared_ptr<const T> get() const {
    auto it = data_.find(portable_class_name<T>());
    if (it != data_.end()) return std::static_pointer_cast<const T>(it->second);
    return nullptr;
  }

  template <typename T>
  void set(T&& t) {
    data_[t.portable_class_name()] =
        std::make_shared<const std::decay_t<T>>(std::forward<T>(t));
  }

  template <typename T>
  void clear() {
    data_.erase(portable_class_name<T>());
  }

  const auto& data() const { return data_; }

 private:
  explicit context(context* parent = current_,
                   time_point deadline = time_point::max(),
                   bool set_current = true);
  void add_child(context* child);
  void remove_child(context* child);
  context make_child();

  template <bool daemon>
  friend class base_thread;

  std::mutex mu_;
  std::condition_variable child_detached_;
  bool set_current_;
  context* parent_;
  ash::flat_set<context*> children_;
  flag cancelled_;
  time_point deadline_;
  ash::flat_map<std::string_view, std::shared_ptr<const dynamic_base_class>>
      data_;
  static thread_local context* current_;
};

}  // namespace ash

#endif  // ASH_CONTEXT_H_
