/// \channel
/// \brief POSIX-specific channel descriptor wrapper.
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

#ifndef ASH_POSIX_IO_H_
#define ASH_POSIX_IO_H_

#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <string>
#include <utility>
#include "ash/common/io.h"

namespace ash {
namespace posix {
namespace detail {

void throw_io_error(const std::string& message);

}  // namespace detail

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
  awaitable read() const;
  awaitable write() const;

 private:
  int fd_;
};

bool operator==(const channel& a, const channel& b);
bool operator!=(const channel& a, const channel& b);
bool operator<(const channel& a, const channel& b);
bool operator<=(const channel& a, const channel& b);
bool operator>(const channel& a, const channel& b);
bool operator>=(const channel& a, const channel& b);

void pipe(channel fds[2]);

channel file(const std::string& path, open_mode mode = open_mode::READ);

template <typename... Args>
std::array<bool, sizeof...(Args)> select(const Args&... args) {
  constexpr std::size_t n = sizeof...(Args);
  pollfd fds[n] = {
      {.fd = *args.channel(),
       .events = static_cast<short>(args.for_write() ? (POLLOUT | POLLERR)
                                                     : (POLLIN | POLLHUP))}...};
  std::chrono::milliseconds timeouts[n] = {args.timeout()...};

  std::chrono::milliseconds timeout = std::chrono::milliseconds(-1);
  constexpr auto zero = std::chrono::milliseconds ::zero();
  for (std::size_t i = 0; i < n; i++) {
    if (timeouts[i] >= zero && (timeout < zero || timeout > timeouts[i])) {
      timeout = timeouts[i];
    }
  }

  int pres = poll(fds, n, timeout / std::chrono::milliseconds(1));
  if (pres < 0) detail::throw_io_error("Error in select");

  std::array<bool, n> res;
  if (pres > 0) {
    for (std::size_t i = 0; i < n; i++) {
      res[i] = (fds[i].revents & fds[i].events) != 0;
    }
  } else {
    for (std::size_t i = 0; i < n; i++) {
      res[i] = timeouts[i] >= zero && timeouts[i] <= timeout;
    }
  }
  return res;
}
}  // namespace posix
}  // namespace ash

#endif  // ASH_POSIX_IO_H_
