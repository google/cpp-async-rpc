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

#include <ash/channel.h>
#include <ash/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <utility>
#include "ash/errors.h"

namespace ash {

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
  if (num < 0) throw_io_error("Error reading");
  return num;
}

awaitable<std::size_t> channel::async_read(void* buf, std::size_t len) {
  return can_read().then(
      std::move([this, buf, len]() { return read(buf, len); }));
}

std::size_t channel::write(const void* buf, std::size_t len) {
  auto num = ::write(fd_, buf, len);
  if (num < 0) throw_io_error("Error writing");
  return num;
}

awaitable<std::size_t> channel::async_write(const void* buf, std::size_t len) {
  return can_write().then(
      std::move([this, buf, len]() { return write(buf, len); }));
}

void channel::make_blocking() {
  if (int flags; (flags = fcntl(fd_, F_GETFL)) < 0 ||
                 fcntl(fd_, F_SETFL, flags & ~O_NONBLOCK) < 0)
    throw_io_error("Error making channel descriptor blocking");
}

void channel::make_non_blocking() {
  if (int flags; (flags = fcntl(fd_, F_GETFL)) < 0 ||
                 fcntl(fd_, F_SETFL, flags | O_NONBLOCK) < 0)
    throw_io_error("Error making channel descriptor non-blocking");
}

channel channel::dup() const {
  channel res(::dup(fd_));
  if (!res) throw_io_error("Error duplicating the channel descriptor");
  return res;
}

awaitable<void> channel::can_read() { return awaitable<void>(fd_, false); }

awaitable<void> channel::can_write() { return awaitable<void>(fd_, true); }

void channel::shutdown(bool read, bool write) {
  int how;
  if (read && write) {
    how = SHUT_RDWR;
  } else if (read) {
    how = SHUT_RD;
  } else if (write) {
    how = SHUT_WR;
  } else {
    return;
  }
  if (::shutdown(fd_, how)) throw_io_error("Error in socket shutdown");
}

awaitable<void> channel::async_connect(const struct sockaddr* addr,
                                       socklen_t addrlen) {
  if (::connect(fd_, addr, addrlen))
    throw_io_error("Error when connecting socket");
  return can_write().then(std::move([this]() {
    int err;
    socklen_t err_size = sizeof(err);
    ::getsockopt(fd_, SOL_SOCKET, SO_ERROR, &err, &err_size);
    if (err) throw_io_error("Connection error", err);
  }));
}

void channel::connect(const struct sockaddr* addr, socklen_t addrlen) {
  select(async_connect(addr, addrlen));
}

}  // namespace ash
