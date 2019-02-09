/// \channel
/// \brief select-friendly synchronization objects.
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

#ifndef ASH_SYNC_H_
#define ASH_SYNC_H_

#include <mutex>
#include <utility>
#include <vector>
#include "ash/errors.h"
#include "io.h"

namespace ash {

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

class flag {
 public:
  flag();

  void set();
  void reset();
  bool is_set() const;
  explicit operator bool() const;
  awaitable<void> wait_set();

 private:
  mutable std::mutex mu_;
  bool set_ = false;
  channel pipe_[2];
};

template <typename T>
class queue {
 public:
  using size_type = typename std::vector<T>::size_type;
  using value_type = T;

  explicit queue(size_type size) : data_(size) { update_flags(); }
  size_type size() const {
    std::scoped_lock lock(mu_);
    return size_;
  }
  size_type max_size() const { return data_.size(); }
  size_type capacity() const { return data_.size(); }
  bool empty() const {
    std::scoped_lock lock(mu_);
    return size_ == 0;
  }
  bool full() const {
    std::scoped_lock lock(mu_);
    return size_ == data_.size();
  }
  template <typename U>
  void maybe_put(U&& u) {
    std::scoped_lock lock(mu_);
    if (size_ == data_.size()) throw errors::try_again("Queue is full");
    data_[head_++] = std::forward<U>(u);
    head_ %= data_.size();
    size_++;
    update_flags();
  }
  value_type maybe_get() {
    std::scoped_lock lock(mu_);
    value_type result;
    if (size_ == 0) throw errors::try_again("Queue is empty");
    std::swap(data_[tail_++], result);
    tail_ %= data_.size();
    size_--;
    update_flags();
    return result;
  }
  template <typename U>
  void put(U&& u) {
    select(async_put(std::forward<U>(u)));
  }
  value_type get() {
    value_type res;
    select(async_get(res));
    return res;
  }
  template <typename U>
  awaitable<void> async_put(U&& u) {
    return std::move(
        can_put().then(std::move([u(std::forward<U>(u)), this]() mutable {
          maybe_put(std::forward<U>(u));
        })));
  }
  awaitable<value_type> async_get() {
    return std::move(
        can_get().then(std::move([this]() { return std::move(maybe_get()); })));
  }
  awaitable<void> can_put() { return can_put_.wait_set(); }
  awaitable<void> can_get() { return can_get_.wait_set(); }

 private:
  void update_flags() {
    if (size_ == 0) {
      can_get_.reset();
    } else {
      can_get_.set();
    }
    if (size_ == data_.size()) {
      can_put_.reset();
    } else {
      can_put_.set();
    }
  }

  mutable std::mutex mu_;
  std::size_t head_ = 0, tail_ = 0, size_ = 0;
  std::vector<T> data_;
  flag can_get_, can_put_;
};

}  // namespace ash

#endif  // ASH_SYNC_H_
