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

#include "ash/posix/io.h"
#include <algorithm>
#include <cerrno>
#include <utility>
#include "ash/errors.h"

namespace ash {
namespace posix {
namespace detail {

void throw_io_error(const std::string& message) {
  if (errno == EAGAIN || errno == EWOULDBLOCK) {
    throw errors::try_again("Try again");
  }
  throw errors::io_error(message + std::string(": ") + std::to_string(errno));
}

}  // namespace detail

channel::channel() noexcept : fd_(-1) {}

channel::channel(int fd) noexcept : fd_(fd) {}

channel::channel(channel&& fd) noexcept : fd_(-1) { swap(fd); }

channel::~channel() noexcept {
  if (fd_ >= 0) {
    ::close(fd_);  // Ignore close-time errors to prevent exceptions.
  }
}

void channel::swap(channel& fd) noexcept { std::swap(fd_, fd.fd_); }
channel& channel::operator=(channel&& fd) noexcept {
  channel tmp;
  tmp.swap(fd);
  swap(tmp);
  return *this;
}

int channel::release() noexcept {
  int res = -1;
  std::swap(res, fd_);
  return res;
}

int channel::get() const noexcept { return fd_; }

int channel::operator*() const noexcept { return fd_; }

void channel::reset(int fd) noexcept {
  channel tmp(fd);
  swap(tmp);
}

channel::operator bool() const noexcept { return (fd_ >= 0); }

void channel::close() noexcept { reset(); }

std::size_t channel::read(void* buf, std::size_t len) {
  auto num = ::read(fd_, buf, len);
  if (num < 0) detail::throw_io_error("Error reading");
  return num;
}

std::size_t channel::write(const void* buf, std::size_t len) {
  auto num = ::write(fd_, buf, len);
  if (num < 0) detail::throw_io_error("Error writing");
  return num;
}

void channel::make_blocking() {
  if (int flags; (flags = fcntl(fd_, F_GETFL)) < 0 ||
                 fcntl(fd_, F_SETFL, flags & ~O_NONBLOCK) < 0)
    detail::throw_io_error("Error making channel descriptor blocking");
}

void channel::make_non_blocking() {
  if (int flags; (flags = fcntl(fd_, F_GETFL)) < 0 ||
                 fcntl(fd_, F_SETFL, flags | O_NONBLOCK) < 0)
    detail::throw_io_error("Error making channel descriptor non-blocking");
}

channel channel::dup() const {
  if (!*this)
    detail::throw_io_error("Trying to duplicate an empty channel descriptor");
  channel res(::dup(fd_));
  if (!res) detail::throw_io_error("Error duplicating the channel descriptor");
  return res;
}

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

awaitable::awaitable(int fd, short events)  // NOLINT(runtime/int)
    : fd_(fd), events_(events) {}

awaitable::awaitable(int timeout_ms) : timeout_ms_(timeout_ms) {}

int awaitable::timeout() const { return timeout_ms_; }
pollfd awaitable::event() const { return {.fd = fd_, .events = events_}; }

awaitable channel::read() const { return awaitable(fd_, POLLIN | POLLHUP); }

awaitable channel::write() const { return awaitable(fd_, POLLOUT | POLLERR); }

void pipe(channel fds[2]) {
  int fd[2];
  if (::pipe(fd)) detail::throw_io_error("Error creating pipe pair");
  fds[0].reset(fd[0]);
  fds[1].reset(fd[1]);
}

channel file(const std::string& path, open_mode mode) {
  channel res(::open(path.c_str(), static_cast<int>(mode)));
  if (!res) detail::throw_io_error("Error opening channel");
  return res;
}

}  // namespace posix
}  // namespace ash
