/// \file
/// \brief POSIX-specific connection implementations.
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

#include "ash/posix/connection.h"

#include <algorithm>
#include <utility>
#include "ash/errors.h"

namespace ash {
namespace posix {

fd_connection::fd_lock::fd_lock(fd_connection& conn) : conn_(conn) {
  std::scoped_lock lock(conn_.mu_);
  conn_.lock_count_++;
}

fd_connection::fd_lock::~fd_lock() {
  bool last = false;

  {
    std::scoped_lock lock(conn_.mu_);
    if (!--conn_.lock_count_) {
      last = true;
    }
  }

  if (last) conn_.idle_.notify_all();
}

fd_connection::fd_connection(channel&& fd) : fd_(std::move(fd)) {
  pipe(pipe_);
  fd_.make_non_blocking();
}

fd_connection::~fd_connection() { disconnect(); }

bool fd_connection::connected() {
  std::scoped_lock lock(mu_);
  return !!fd_;
}

void fd_connection::disconnect() {
  std::unique_lock lock(mu_);
  if (!fd_) {
    // Already disconnected.
    return;
  }

  if (!closing_) {
    closing_ = true;
    pipe_[1].close();
  }

  // Wait for all shared owners to go away.
  idle_.wait(lock, [this] { return lock_count_ == 0; });

  if (closing_) {
    pipe_[0].close();
    fd_.close();
    closing_ = false;
  }
}

void fd_connection::write(const char* data, std::size_t size) {
  fd_lock lock(*this);
  check_connected();

  while (size > 0) {
    try {
      auto written = fd_.write(data, size);
      size -= written;
      data += written;
    } catch (const errors::try_again&) {
      auto s = select(fd_.write(), pipe_[0].read());
      if (s[1])
        throw errors::shutting_down("Write interrupted by connection shutdown");
    }
  }
}

void fd_connection::putc(char c) { write(&c, 1); }

void fd_connection::flush() {}

std::size_t fd_connection::read(char* data, std::size_t size) {
  fd_lock lock(*this);
  check_connected();

  std::size_t total_read = 0;
  while (size > 0) {
    try {
      auto read = fd_.read(data, size);
      if (read == 0) break;
      size -= read;
      data += read;
      total_read += read;
    } catch (const errors::try_again&) {
      auto s = select(fd_.read(), pipe_[0].read());
      if (s[1])
        throw errors::shutting_down("Read interrupted by connection shutdown");
    }
  }
  return total_read;
}

char fd_connection::getc() {
  char c;
  read_fully(&c, 1);
  return c;
}

void fd_connection::check_connected() {
  if (closing_ || !fd_) throw errors::io_error("Connection is closed");
}

char_dev_connection::char_dev_connection(const std::string& path)
    : fd_connection(open_path(path)) {}

channel char_dev_connection::open_path(const std::string& path) {
  return file(path, open_mode::READ_PLUS);
}

}  // namespace posix
}  // namespace ash
