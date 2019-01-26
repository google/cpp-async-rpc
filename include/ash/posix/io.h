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

#ifndef INCLUDE_ASH_POSIX_IO_H_
#define INCLUDE_ASH_POSIX_IO_H_

#include <fcntl.h>
#include <poll.h>
#include <array>
#include <chrono>
#include <string>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace ash {
namespace posix {
namespace detail {

void throw_io_error(const std::string& message);

}  // namespace detail

class awaitable;

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
class awaitable {
 public:
  awaitable(int fd, short events);
  explicit awaitable(int timeout_ms);

 private:
  int timeout() const;
  pollfd event() const;

  template <typename... Args>
  friend std::array<bool, sizeof...(Args)> select(const Args&... args);

  int fd_ = -1;
  short events_ = 0;
  int timeout_ms_ = -1;
};

template <typename Duration>
awaitable timeout(Duration duration) {
  return awaitable(duration / std::chrono::milliseconds(1));
}

template <typename Timepoint>
awaitable deadline(Timepoint when) {
  std::chrono::milliseconds delta =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          when - std::chrono::system_clock::now());
  return awaitable(std::max(std::chrono::milliseconds::zero(), delta) /
                   std::chrono::milliseconds(1));
}

void pipe(channel fds[2]);

enum class open_mode : int {
  READ = O_RDONLY,
  WRITE = O_WRONLY | O_CREAT | O_TRUNC,
  APPEND = O_WRONLY | O_CREAT | O_APPEND,
  READ_PLUS = O_RDWR,
  WRITE_PLUS = O_RDWR | O_CREAT | O_TRUNC,
  APPEND_PLUS = O_RDWR | O_CREAT | O_APPEND,
};

channel file(const std::string& path, open_mode mode = open_mode::READ);

template <typename... Args>
std::array<bool, sizeof...(Args)> select(const Args&... args) {
  constexpr std::size_t n = sizeof...(Args);
  pollfd fds[n] = {args.event()...};
  int timeouts[n] = {args.timeout()...};

  int timeout_ms = -1;
  for (std::size_t i = 0; i < n; i++) {
    if (timeouts[i] >= 0 && (timeout_ms < 0 || timeout_ms > timeouts[i])) {
      timeout_ms = timeouts[i];
    }
  }

  int pres = poll(fds, n, timeout_ms);
  if (pres < 0) detail::throw_io_error("Error in select");

  std::array<bool, n> res;
  if (pres > 0) {
    for (std::size_t i = 0; i < n; i++) {
      res[i] = (fds[i].revents & fds[i].events) != 0;
    }
  } else {
    for (std::size_t i = 0; i < n; i++) {
      res[i] = timeouts[i] >= 0 && timeouts[i] <= timeout_ms;
    }
  }
  return res;
}
}  // namespace posix
}  // namespace ash

#endif  // INCLUDE_ASH_POSIX_IO_H_
