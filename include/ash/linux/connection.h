/// \file
/// \brief Linux-specific connection implementations.
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

#ifndef INCLUDE_ASH_LINUX_CONNECTION_H_
#define INCLUDE_ASH_LINUX_CONNECTION_H_

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <utility>
#include "ash/connection.h"
#include "ash/errors.h"

namespace ash {

namespace detail {
void throw_io_error(const std::string& message) {
  throw errors::io_error(message + std::string(": ") + std::to_string(errno));
}
}  // namespace detail

class char_dev_connection : public connection {
 public:
  explicit char_dev_connection(std::string path) : path_(std::move(path)) {}
  ~char_dev_connection() override { disconnect(); }

  void connect() override {
    disconnect();
    fd_ = open(path_.c_str(), O_RDWR);
    if (fd_ < 0) detail::throw_io_error(std::string("Error opening ") + path_);
  }

  void disconnect() override {
    if (fd_ >= 0) {
      if (::close(fd_)) detail::throw_io_error("Error closing file descriptor");
      fd_ = -1;
    }
  }

  void write(const char* data, std::size_t size) override {
    check_open();
    while (size > 0) {
      auto written = ::write(fd_, data, size);
      if (written < 0) detail::throw_io_error("Error writing");
      size -= written;
      data += written;
    }
  }

  void putc(char c) override { write(&c, 1); }

  void flush() override {}

  std::size_t read(char* data, std::size_t size) override {
    check_open();
    std::size_t total_read = 0;
    while (size > 0) {
      auto read = ::read(fd_, data, size);
      if (read < 0) detail::throw_io_error("Error writing");
      if (read == 0) return total_read;
      size -= read;
      data += read;
      total_read += read;
    }
    return total_read;
  }

  char getc() override {
    char c;
    read_fully(&c, 1);
    return c;
  }

 private:
  void check_open() {
    if (fd_ < 0) throw errors::io_error("Connection is closed");
  }
  std::string path_;
  int fd_ = -1;
};

}  // namespace ash

#endif  // INCLUDE_ASH_LINUX_CONNECTION_H_
