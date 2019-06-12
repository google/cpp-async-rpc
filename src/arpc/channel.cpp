/// \file
/// \brief Channel descriptor wrapper.
///
/// \copyright
///   Copyright 2019 by Google LLC.
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

#include "arpc/channel.h"
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <utility>
#ifndef ESP_PLATFORM
#include <netinet/in.h>
#include <netinet/tcp.h>
#else  // ESP_PLATFORM
#include <sys/socket.h>
#endif  // ESP_PLATFORM
#include "arpc/errors.h"
#include "arpc/select.h"

namespace arpc {

#ifndef ESP_PLATFORM
namespace {
struct ignore_sigpipe {
  ignore_sigpipe() { signal(SIGPIPE, SIG_IGN); }
} ignore_sigpipe_instance;
}  // namespace
#endif  // ESP_PLATFORM

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
  auto [num] = select(async_read(buf, len));
  return *num;
}

std::size_t channel::maybe_read(void* buf, std::size_t len) {
  auto num = ::read(fd_, buf, len);
  if (num < 0) throw_io_error("Error reading");
  if (num == 0) throw errors::eof("End of channel");
  return static_cast<std::size_t>(num);
}

awaitable<std::size_t> channel::async_read(void* buf, std::size_t len) {
  return can_read().then([this, buf, len]() { return maybe_read(buf, len); });
}

std::size_t channel::write(const void* buf, std::size_t len) {
  auto [num] = select(async_write(buf, len));
  return *num;
}

std::size_t channel::maybe_write(const void* buf, std::size_t len) {
  auto num = ::write(fd_, buf, len);
  if (num < 0) throw_io_error("Error writing");
  return static_cast<std::size_t>(num);
}

awaitable<std::size_t> channel::async_write(const void* buf, std::size_t len) {
  return can_write().then([this, buf, len]() { return maybe_write(buf, len); });
}

channel& channel::make_non_blocking(bool non_blocking) {
  if (int flags;
      (flags = fcntl(fd_, F_GETFL)) < 0 ||
      fcntl(fd_, F_SETFL,
            non_blocking ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK)) < 0)
    throw_io_error("Error making channel descriptor non-blocking");
  return *this;
}

channel channel::dup() const {
  channel res(::dup(fd_));
  if (!res) throw_io_error("Error duplicating the channel descriptor");
  return res;
}

awaitable<void> channel::can_read() { return awaitable<void>(fd_, false); }

awaitable<void> channel::can_write() { return awaitable<void>(fd_, true); }

channel& channel::shutdown(bool read, bool write) {
  int how;
  if (read && write) {
    how = SHUT_RDWR;
  } else if (read) {
    how = SHUT_RD;
  } else if (write) {
    how = SHUT_WR;
  } else {
    return *this;
  }
  if (::shutdown(fd_, how)) throw_io_error("Error in socket shutdown");
  return *this;
}

awaitable<void> channel::async_connect(const address& addr) {
  if (::connect(fd_, addr.address_data(), addr.address_size()))
    throw_io_error("Error when connecting socket");
  return can_write().then(std::move([this]() {
    int err;
    socklen_t err_size = sizeof(err);
    ::getsockopt(fd_, SOL_SOCKET, SO_ERROR, &err, &err_size);
    if (err) throw_io_error("Connection error", err);
  }));
}

channel& channel::connect(const address& addr) {
  auto [res] = select(async_connect(addr));
  *res;
  return *this;
}

channel& channel::bind(const address& addr) {
  if (addr.address_data()->sa_family == AF_INET6) {
    int ipv6_v6only = true;
    if (::setsockopt(fd_, IPPROTO_IPV6, IPV6_V6ONLY, &ipv6_v6only, sizeof(int)))
      throw_io_error("Error when setting socket to bind only on ipv6");
  }
  if (::bind(fd_, addr.address_data(), addr.address_size()))
    throw_io_error("Error when binding socket");
  return *this;
}

channel& channel::listen(int backlog) {
  if (::listen(fd_, backlog))
    throw_io_error("Error when putting socket into listen state");
  return *this;
}

awaitable<channel> channel::async_accept() {
  return can_read().then(std::move([this]() { return maybe_accept(); }));
}

channel channel::accept() {
  auto [c] = select(async_accept());
  return std::move(*c);
}

channel channel::maybe_accept() {
  channel c(::accept(fd_, nullptr, nullptr));
  if (!c) throw_io_error("Accept error");
  return c;
}

awaitable<channel> channel::async_accept(address& addr) {
  return can_read().then(
      std::move([this, &addr]() { return maybe_accept(addr); }));
}

channel channel::accept(address& addr) {
  auto [c] = select(async_accept(addr));
  return std::move(*c);
}

channel channel::maybe_accept(address& addr) {
  channel c(::accept(fd_, addr.address_data(), &addr.address_size()));
  if (!c) throw_io_error("Accept error");
  return c;
}

channel& channel::keep_alive(bool keep_alive) {
  int value = keep_alive;
  if (::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(value)))
    throw_io_error("Error setting keep_alive");
  return *this;
}

channel& channel::reuse_addr(bool reuse) {
  int value = reuse;
  if (::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)))
    throw_io_error("Error setting reuse_addr");
  return *this;
}

channel& channel::reuse_port(bool reuse) {
  int value = reuse;
  if (::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &value, sizeof(value)))
    throw_io_error("Error setting reuse_port");
  return *this;
}

channel& channel::flush() {
  constexpr int on = 1, off = 0;
  if (!::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on))) {
    ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &off, sizeof(off));
  }
  return *this;
}

channel& channel::no_delay(bool no_delay) {
  int value = no_delay;
  if (::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value)))
    throw_io_error("Error setting no_delay");
  return *this;
}

channel& channel::linger(bool do_linger, std::chrono::seconds linger_time) {
  struct linger value {
    do_linger, static_cast<int>(linger_time / std::chrono::seconds(1))
  };
  if (::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &value, sizeof(value)))
    throw_io_error("Error setting linger");
  return *this;
}

address channel::own_addr() const {
  address res;
  if (::getsockname(fd_, res.address_data(), &res.address_size()))
    throw_io_error("Can't get socket name");
  res.family() = res.address_data()->sa_family;
  socklen_t buf_size;
  buf_size = sizeof(res.socket_type());
  if (::getsockopt(fd_, SOL_SOCKET, SO_TYPE, &res.socket_type(), &buf_size))
    throw_io_error("Can't get socket type");
#ifndef ESP_PLATFORM
  buf_size = sizeof(res.protocol());
  if (::getsockopt(fd_, SOL_SOCKET, SO_PROTOCOL, &res.protocol(), &buf_size))
    throw_io_error("Can't get socket protocol");
#endif  // ESP_PLATFORM
  return res;
}

address channel::peer_addr() const {
  address res;
  if (::getpeername(fd_, res.address_data(), &res.address_size()))
    throw_io_error("Can't get socket name");
  res.family() = res.address_data()->sa_family;
  socklen_t buf_size;
  buf_size = sizeof(res.socket_type());
  if (::getsockopt(fd_, SOL_SOCKET, SO_TYPE, &res.socket_type(), &buf_size))
    throw_io_error("Can't get socket type");
#ifndef ESP_PLATFORM
  buf_size = sizeof(res.protocol());
  if (::getsockopt(fd_, SOL_SOCKET, SO_PROTOCOL, &res.protocol(), &buf_size))
    throw_io_error("Can't get socket protocol");
#endif  // ESP_PLATFORM
  return res;
}

}  // namespace arpc
