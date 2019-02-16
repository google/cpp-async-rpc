/// \file
/// \brief Channel descriptor wrapper.
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

#ifndef ASH_CHANNEL_H_
#define ASH_CHANNEL_H_

#include <cstddef>
#include "ash/awaitable.h"

namespace ash {

class channel {
 public:
  channel() noexcept;
  explicit channel(int fd) noexcept;
  channel(channel&& fd) noexcept;
  ~channel() noexcept;
  void swap(channel& fd) noexcept;
  channel& operator=(channel&& fd) noexcept;
  int release() noexcept;
  int get() const noexcept;
  int operator*() const noexcept;
  void reset(int fd = -1) noexcept;
  explicit operator bool() const noexcept;
  void close() noexcept;
  std::size_t read(void* buf, std::size_t len);
  std::size_t write(const void* buf, std::size_t len);
  void make_blocking();
  void make_non_blocking();
  channel dup() const;
  awaitable<void> can_read();
  awaitable<void> can_write();
  awaitable<std::size_t> async_read(void* buf, std::size_t len);
  awaitable<std::size_t> async_write(const void* buf, std::size_t len);

 private:
  int fd_;
};

}  // namespace ash

#endif  // ASH_CHANNEL_H_
