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
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <utility>
#include "ash/errors.h"

namespace ash {

namespace detail {

void throw_io_error(const std::string& message) {
  if (errno == EAGAIN || errno == EWOULDBLOCK) {
    throw errors::try_again("Try again");
  }
  throw errors::io_error(message + std::string(": ") + std::to_string(errno));
}

}  // namespace detail

class channel {
 public:
  constexpr channel() noexcept : fd_(-1) {}

  explicit channel(int fd) noexcept : fd_(fd) {}

  channel(channel&& fd) noexcept : fd_(-1) { swap(fd); }

  ~channel() noexcept {
    if (fd_ >= 0) {
      ::close(fd_);  // Ignore close-time errors to prevent exceptions.
    }
  }

  void swap(channel& fd) noexcept { std::swap(fd_, fd.fd_); }
  channel& operator=(channel&& fd) noexcept {
    channel tmp;
    tmp.swap(fd);
    swap(tmp);
    return *this;
  }

  int release() noexcept {
    int res = -1;
    std::swap(res, fd_);
    return res;
  }

  int get() const noexcept { return fd_; }

  int operator*() const noexcept { return fd_; }

  void reset(int fd = -1) noexcept {
    channel tmp(fd);
    swap(tmp);
  }

  explicit operator bool() const noexcept { return (fd_ >= 0); }

  void close() noexcept { reset(); }

  std::size_t read(void* buf, std::size_t len) {
    auto num = ::read(fd_, buf, len);
    if (num < 0) detail::throw_io_error("Error reading");
    return num;
  }

  std::size_t write(const void* buf, std::size_t len) {
    auto num = ::write(fd_, buf, len);
    if (num < 0) detail::throw_io_error("Error writing");
    return num;
  }

  void make_blocking() {
    if (int flags; (flags = fcntl(fd_, F_GETFL)) < 0 ||
                   fcntl(fd_, F_SETFL, flags & ~O_NONBLOCK) < 0)
      detail::throw_io_error("Error making channel descriptor blocking");
  }

  void make_non_blocking() {
    if (int flags; (flags = fcntl(fd_, F_GETFL)) < 0 ||
                   fcntl(fd_, F_SETFL, flags | O_NONBLOCK) < 0)
      detail::throw_io_error("Error making channel descriptor non-blocking");
  }

  channel dup() const {
    if (!*this)
      detail::throw_io_error("Trying to duplicate an empty channel descriptor");
    channel res(::dup(fd_));
    if (!res)
      detail::throw_io_error("Error duplicating the channel descriptor");
    return res;
  }

 private:
  int fd_;
};

bool operator==(const channel& a, const channel& b) {
  return a.get() == b.get();
}
bool operator!=(const channel& a, const channel& b) {
  return a.get() != b.get();
}
bool operator<(const channel& a, const channel& b) { return a.get() < b.get(); }
bool operator<=(const channel& a, const channel& b) {
  return a.get() <= b.get();
}
bool operator>(const channel& a, const channel& b) { return a.get() > b.get(); }
bool operator>=(const channel& a, const channel& b) {
  return a.get() >= b.get();
}

void pipe(channel fds[2]) {
  int fd[2];
  if (::pipe(fd)) detail::throw_io_error("Error creating pipe pair");
  fds[0].reset(fd[0]);
  fds[1].reset(fd[1]);
}

enum class open_mode : int {
  READ = O_RDONLY,
  WRITE = O_WRONLY | O_CREAT | O_TRUNC,
  APPEND = O_WRONLY | O_CREAT | O_APPEND,
  READ_PLUS = O_RDWR,
  WRITE_PLUS = O_RDWR | O_CREAT | O_TRUNC,
  APPEND_PLUS = O_RDWR | O_CREAT | O_APPEND,
};

channel file(const std::string& path, open_mode mode = open_mode::READ) {
  channel res(::open(path.c_str(), static_cast<int>(mode)));
  if (!res) detail::throw_io_error("Error opening channel");
  return res;
}

}  // namespace ash

#endif  // INCLUDE_ASH_POSIX_IO_H_
