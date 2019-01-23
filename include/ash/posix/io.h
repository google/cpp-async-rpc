/// \file
/// \brief POSIX-specific file descriptor wrapper.
///
/// \copyright
///   Copyright 2018 by Google Inc. All Rights Reserved.
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

class file_descriptor {
 public:
  constexpr file_descriptor() noexcept : fd_(-1) {}
  explicit file_descriptor(int fd) noexcept : fd_(fd) {}
  file_descriptor(file_descriptor&& fd) noexcept : fd_(-1) { swap(fd); }
  ~file_descriptor() noexcept {
    if (fd_ >= 0) {
      ::close(fd_);  // Ignore close-time errors to prevent exceptions.
    }
  }
  void swap(file_descriptor& fd) noexcept { std::swap(fd_, fd.fd_); }
  file_descriptor& operator=(file_descriptor&& fd) noexcept {
    file_descriptor tmp;
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
    file_descriptor tmp(fd);
    swap(tmp);
  }
  explicit operator bool() const noexcept { return (fd_ >= 0); }

  std::size_t read(void* buf, std::size_t len) {
    auto num = ::read(fd_, buf, len);
    if (num < 0) throw_with_errno<errors::io_error>("Error reading");
    return num;
  }

  std::size_t write(const void* buf, std::size_t len) {
    auto num = ::write(fd_, buf, len);
    if (num < 0) throw_with_errno<errors::io_error>("Error writing");
    return num;
  }

  void make_blocking() {
    if (int flags; (flags = fcntl(fd_, F_GETFL)) < 0 ||
                   fcntl(fd_, F_SETFL, flags & ~O_NONBLOCK) < 0)
      throw_with_errno<errors::io_error>(
          "Error making file descriptor blocking");
  }

  void make_non_blocking() {
    if (int flags; (flags = fcntl(fd_, F_GETFL)) < 0 ||
                   fcntl(fd_, F_SETFL, flags | O_NONBLOCK) < 0)
      throw_with_errno<errors::io_error>(
          "Error making file descriptor non-blocking");
  }

  file_descriptor dup() const {
    if (!*this)
      throw_with_errno<errors::io_error>(
          "Trying to duplicate an empty file descriptor");
    file_descriptor res(::dup(fd_));
    if (!res)
      throw_with_errno<errors::io_error>(
          "Error duplicating the file descriptor");
    return res;
  }

 private:
  int fd_;
};

bool operator==(const file_descriptor& a, const file_descriptor& b) {
  return a.get() == b.get();
}
bool operator!=(const file_descriptor& a, const file_descriptor& b) {
  return a.get() != b.get();
}
bool operator<(const file_descriptor& a, const file_descriptor& b) {
  return a.get() < b.get();
}
bool operator<=(const file_descriptor& a, const file_descriptor& b) {
  return a.get() <= b.get();
}
bool operator>(const file_descriptor& a, const file_descriptor& b) {
  return a.get() > b.get();
}
bool operator>=(const file_descriptor& a, const file_descriptor& b) {
  return a.get() >= b.get();
}

void pipe(file_descriptor fds[2]) {
  int fd[2];
  if (::pipe(fd))
    throw_with_errno<errors::io_error>("Error creating pipe pair");
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

file_descriptor open(const std::string& path,
                     open_mode mode = open_mode::READ) {
  file_descriptor res(::open(path.c_str(), static_cast<int>(mode)));
  if (!res) throw_with_errno<errors::io_error>("Error opening file");
  return res;
}

}  // namespace ash

#endif  // INCLUDE_ASH_POSIX_IO_H_
