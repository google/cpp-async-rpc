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

#ifndef INCLUDE_ASH_POSIX_CONNECTION_H_
#define INCLUDE_ASH_POSIX_CONNECTION_H_

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <string>
#include <utility>
#include "ash/connection.h"
#include "ash/errors.h"
#include "ash/posix/io.h"

namespace ash {

class fd_connection : public connection {
 protected:
  class fd_lock {
   public:
    explicit fd_lock(fd_connection& conn) : conn_(conn) {
      std::scoped_lock lock(conn_.mu_);
      conn_.lock_count_++;
    }

    ~fd_lock() {
      bool last = false;

      {
        std::scoped_lock lock(conn_.mu_);
        if (!--conn_.lock_count_) {
          last = true;
        }
      }

      if (last) conn_.idle_.notify_all();
    }

   private:
    fd_connection& conn_;
  };

 public:
  explicit fd_connection(channel&& fd) : fd_(std::move(fd)) {
    pipe(pipe_);
    fd_.make_non_blocking();
  }

  ~fd_connection() override { disconnect(); }

  bool connected() override {
    std::scoped_lock lock(mu_);
    return !!fd_;
  }

  void disconnect() override {
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

  void write(const char* data, std::size_t size) override {
    fd_lock lock(*this);
    check_connected();

    while (size > 0) {
      try {
        auto s = select(fd_.write(), pipe_[0].read());

        if (s[1])
          throw errors::shutting_down(
              "Write interrupted by connection shutdown");

        if (s[0]) {
          auto written = fd_.write(data, size);
          size -= written;
          data += written;
        }
      } catch (const errors::try_again&) {
      }
    }
  }

  void putc(char c) override { write(&c, 1); }

  void flush() override {}

  std::size_t read(char* data, std::size_t size) override {
    fd_lock lock(*this);
    check_connected();

    std::size_t total_read = 0;
    while (size > 0) {
      try {
        auto s = select(fd_.read(), pipe_[0].read());

        if (s[1])
          throw errors::shutting_down(
              "Read interrupted by connection shutdown");

        if (s[0]) {
          auto read = fd_.read(data, size);
          if (read == 0) break;
          size -= read;
          data += read;
          total_read += read;
        }
      } catch (const errors::try_again&) {
      }
    }
    return total_read;
  }

  char getc() override {
    char c;
    read_fully(&c, 1);
    return c;
  }

 protected:
  void check_connected() {
    if (closing_ || !fd_) throw errors::io_error("Connection is closed");
  }
  std::mutex mu_;
  std::condition_variable idle_;
  channel fd_;
  channel pipe_[2];
  bool closing_ = false;
  int lock_count_ = 0;
};  // namespace ash

class char_dev_connection : public fd_connection {
 public:
  explicit char_dev_connection(const std::string& path)
      : fd_connection(open_path(path)) {}

 protected:
  static channel open_path(const std::string& path) {
    return file(path, open_mode::READ_PLUS);
  }
};

}  // namespace ash

#endif  // INCLUDE_ASH_POSIX_CONNECTION_H_
