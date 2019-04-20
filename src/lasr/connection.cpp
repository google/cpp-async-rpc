/// \file
/// \brief Connections are bidirectional streams.
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

#include "lasr/connection.h"
#include <utility>
#include "lasr/file.h"
#include "lasr/select.h"

namespace lasr {

connection::~connection() {}

void connection::connect() {
  throw errors::not_implemented("Constructor-only connection");
}

packet_connection::~packet_connection() {}

void packet_connection::connect() {
  throw errors::not_implemented("Constructor-only connection");
}

channel_connection::channel_connection(channel&& ch) : channel_(std::move(ch)) {
  channel_.make_non_blocking();
  locked_.arm(this);
}

channel_connection::~channel_connection() { disconnect(); }

bool channel_connection::connected() {
  return static_cast<bool>(locked_.get_or_null());
}

void channel_connection::disconnect() {
  std::unique_lock lock(mu_);
  closing_.set();
  locked_.drop();
  channel_.close();
}

void channel_connection::write(const char* data, std::size_t size) {
  auto lock = locked_.get();

  while (size > 0) {
    auto [written, closing] =
        select(channel_.async_write(data, size), closing_.wait_set());
    if (written) {
      size -= *written;
      data += *written;
    }
    if (closing)
      throw errors::io_error("Write interrupted by connection shutdown");
  }
}

void channel_connection::putc(char c) { connection::putc(c); }

void channel_connection::flush() { channel_.flush(); }

std::size_t channel_connection::read(char* data, std::size_t size) {
  auto lock = locked_.get();

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
      throw errors::io_error("Read interrupted by connection shutdown");
  }
  return total_read;
}

char channel_connection::getc() { return connection::getc(); }

awaitable<void> channel_connection::data_available() {
  return channel_.can_read();
}

char_dev_connection::char_dev_connection(const std::string& path)
    : channel_connection(open_path(path)) {}

channel char_dev_connection::open_path(const std::string& path) {
  return file(path, open_mode::READ_PLUS);
}

client_socket_connection::client_socket_connection(endpoint name)
    : channel_connection(dial(std::move(name))) {}

}  // namespace lasr
