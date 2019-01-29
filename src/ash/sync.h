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
#include "ash/io.h"

namespace ash {

class mutex {
 public:
  mutex();

  void lock();
  void lock_nowait();
  bool try_lock();
  void unlock();
  awaitable wait();

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
  awaitable wait();

 private:
  mutable std::mutex mu_;
  bool set_ = false;
  channel pipe_[2];
};

class notification {
 public:
  notification();

  void notify_one();
  void notify_all();
  awaitable wait();

 private:
  std::mutex mu_;
  std::size_t num_waiters_ = 0;
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
  void try_put(const T& t) {
    std::scoped_lock lock(mu_);
    if (size_ == data_.size()) throw errors::try_again("Queue is full");
    data_[head_++] = t;
    head_ %= data_.size();
    size_++;
    update_flags();
  }
  void try_put(T&& t) {
    std::scoped_lock lock(mu_);
    if (size_ == data_.size()) throw errors::try_again("Queue is full");
    data_[head_++] = std::move(t);
    head_ %= data_.size();
    size_++;
    update_flags();
  }
  value_type try_get() {
    std::scoped_lock lock(mu_);
    value_type result;
    if (size_ == 0) throw errors::try_again("Queue is empty");
    std::swap(data_[tail_++], result);
    tail_ %= data_.size();
    size_--;
    update_flags();
    return result;
  }
  void put(const T& t) {
    do {
      try {
        select(has_space());
        try_put(t);
        return;
      } catch (const errors::try_again&) {
      }
    } while (true);
  }
  void put(T&& t) {
    do {
      try {
        select(has_space());
        try_put(std::move(t));
        return;
      } catch (const errors::try_again&) {
      }
    } while (true);
  }
  value_type get() {
    do {
      try {
        select(has_data());
        return try_get();
      } catch (const errors::try_again&) {
      }
    } while (true);
  }
  awaitable wait_put(const T& t) {
    return has_space_.wait().then([&t, this]() {
      try {
        try_put(t);
        return true;
      } catch (const errors::try_again&) {
        return false;
      }
    });
  }
  awaitable wait_put(T&& t) {
    return has_space_.wait().then([t(std::move(t)), this]() {
      try {
        try_put(t);
        return true;
      } catch (const errors::try_again&) {
        return false;
      }
    });
  }
  awaitable wait_get(T& t) {
    return has_data_.wait().then([&t, this]() {
      try {
        t = try_get();
        return true;
      } catch (const errors::try_again&) {
        return false;
      }
    });
  }
  awaitable has_space() { return has_space_.wait(); }
  awaitable has_data() { return has_data_.wait(); }

 private:
  void update_flags() {
    if (size_ == 0) {
      has_data_.reset();
    } else {
      has_data_.set();
    }
    if (size_ == data_.size()) {
      has_space_.reset();
    } else {
      has_space_.set();
    }
  }

  mutable std::mutex mu_;
  std::size_t head_ = 0, tail_ = 0, size_ = 0;
  std::vector<T> data_;
  flag has_data_, has_space_;
};

}  // namespace ash

#endif  // ASH_SYNC_H_
