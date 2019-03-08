/// \file
/// \brief Connections are bidirectional streams.
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

#include "ash/connection.h"
#include <utility>
#include "ash/file.h"
#include "ash/select.h"

namespace ash {

connection::~connection() {}

void connection::connect() {
  throw errors::not_implemented("Constructor-only connection");
}

packet_connection::~packet_connection() {}

void packet_connection::connect() {
  throw errors::not_implemented("Constructor-only connection");
}

channel_connection::channel_lock::channel_lock(channel_connection& conn)
    : conn_(conn) {
  std::scoped_lock lock(conn_.mu_);
  conn_.lock_count_++;
}

channel_connection::channel_lock::~channel_lock() {
  bool last = false;

  {
    std::scoped_lock lock(conn_.mu_);
    if (!--conn_.lock_count_) {
      last = true;
    }
  }

  if (last) conn_.idle_.notify_all();
}

channel_connection::channel_connection(channel&& ch) : channel_(std::move(ch)) {
  channel_.make_non_blocking();
}

channel_connection::~channel_connection() { disconnect(); }

bool channel_connection::connected() {
  std::scoped_lock lock(mu_);
  return !!channel_;
}

void channel_connection::disconnect() {
  std::unique_lock lock(mu_);
  if (!channel_) {
    // Already disconnected.
    return;
  }

  if (!closing_) {
    closing_.set();
  }

  // Wait for all shared owners to go away.
  idle_.wait(lock, [this] { return lock_count_ == 0; });

  if (closing_) {
    channel_.close();
    closing_.reset();
  }
}

void channel_connection::write(const char* data, std::size_t size) {
  channel_lock lock(*this);
  check_connected();

  while (size > 0) {
    auto [written, closing] =
        select(channel_.async_write(data, size), closing_.wait_set());
    if (written) {
      size -= *written;
      data += *written;
    }
    if (closing)
      throw errors::shutting_down("Write interrupted by connection shutdown");
  }
}

void channel_connection::putc(char c) { write(&c, 1); }

void channel_connection::flush() {}

std::size_t channel_connection::read(char* data, std::size_t size) {
  channel_lock lock(*this);
  check_connected();

  std::size_t total_read = 0;
  while (size > 0) {
    auto [read, closing] =
        select(channel_.async_read(data, size), closing_.wait_set());
    if (read) {
      if (*read == 0) break;
      size -= *read;
      data += *read;
      total_read += *read;
    }
    if (closing)
      throw errors::shutting_down("Read interrupted by connection shutdown");
  }
  return total_read;
}

char channel_connection::getc() {
  char c;
  read_fully(&c, 1);
  return c;
}

void channel_connection::check_connected() {
  if (closing_ || !channel_) throw errors::io_error("Connection is closed");
}

char_dev_connection::char_dev_connection(const std::string& path)
    : channel_connection(open_path(path)) {}

channel char_dev_connection::open_path(const std::string& path) {
  return file(path, open_mode::READ_PLUS);
}

client_socket_connection::client_socket_connection(endpoint name)
    : channel_connection(dial(std::move(name))) {}

}  // namespace ash
