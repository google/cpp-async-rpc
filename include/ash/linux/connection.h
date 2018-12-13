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
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <condition_variable>
#include <mutex>
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

class fd_connection : public connection {
 protected:
  class fd_lock {
   public:
    explicit fd_lock(fd_connection& conn) : conn_(conn) {
      std::lock_guard<std::mutex> lock(conn_.mu_);
      conn_.lock_count_++;
    }

    ~fd_lock() {
      bool last = false;

      {
        std::lock_guard<std::mutex> lock(conn_.mu_);
        if (!--conn_.lock_count_) {
          last = true;
        }
      }

      if (last) conn_.idle_.notify_all();
    }

   private:
    fd_connection& conn_;
  };

  virtual int make_fd() = 0;

 public:
  ~fd_connection() override { disconnect(); }

  bool connected() override {
    std::lock_guard<std::mutex> lock(mu_);
    return (fd_ >= 0);
  }

  void connect() override {
    std::lock_guard<std::mutex> lock(mu_);
    if (fd_ >= 0) {
      // Already connected.
      return;
    }

    fd_ = make_fd();
    if (::pipe(pipe_)) {
      ::close(fd_);
      fd_ = -1;
      detail::throw_io_error("Error creating control pipe");
    }
  }

  void disconnect() override {
    std::unique_lock<std::mutex> lock(mu_);
    if (fd_ < 0) {
      // Already disconnected.
      return;
    }

    if (!closing_) {
      closing_ = true;
      if (::close(pipe_[1]))
        detail::throw_io_error("Error signaling control pipe");
    }

    // Wait for all shared owners to go away.
    idle_.wait(lock, [this] { return lock_count_ == 0; });

    if (closing_) {
      if (::close(fd_) | ::close(pipe_[0]))
        detail::throw_io_error("Error closing file descriptor");
      fd_ = -1;
      closing_ = false;
    }
  }

  void write(const char* data, std::size_t size) override {
    fd_lock lock(*this);
    check_connected();

    fd_set fdrs, fdws;

    while (size > 0) {
      FD_ZERO(&fdrs);
      FD_ZERO(&fdws);
      FD_SET(pipe_[0], &fdrs);
      FD_SET(fd_, &fdws);
      if (::select(1 + std::max(fd_, pipe_[0]), &fdrs, &fdws, nullptr,
                   nullptr) < 0)
        detail::throw_io_error("Error in select");
      if (FD_ISSET(pipe_[0], &fdrs))
        throw errors::shutting_down("Write interrupted by connection shutdown");

      if (FD_ISSET(fd_, &fdws)) {
        auto written = ::write(fd_, data, size);
        if (written < 0 && errno == EAGAIN) continue;
        if (written < 0) detail::throw_io_error("Error writing");
        size -= written;
        data += written;
      }
    }
  }

  void putc(char c) override { write(&c, 1); }

  void flush() override {}

  std::size_t read(char* data, std::size_t size) override {
    fd_lock lock(*this);
    check_connected();

    std::size_t total_read = 0;
    fd_set fdrs;
    while (size > 0) {
      FD_ZERO(&fdrs);
      FD_SET(pipe_[0], &fdrs);
      FD_SET(fd_, &fdrs);
      if (::select(1 + std::max(fd_, pipe_[0]), &fdrs, nullptr, nullptr,
                   nullptr) < 0)
        detail::throw_io_error("Error in select");
      if (FD_ISSET(pipe_[0], &fdrs))
        throw errors::shutting_down("Read interrupted by connection shutdown");

      if (FD_ISSET(fd_, &fdrs)) {
        auto read = ::read(fd_, data, size);
        if (read < 0 && errno == EAGAIN) continue;
        if (read < 0) detail::throw_io_error("Error reading");
        if (read == 0) return total_read;
        size -= read;
        data += read;
        total_read += read;
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
    if (closing_ || fd_ < 0) throw errors::io_error("Connection is closed");
  }
  std::mutex mu_;
  std::condition_variable idle_;
  int fd_ = -1;
  int pipe_[2];
  bool closing_ = false;
  int lock_count_ = 0;
};  // namespace ash

class char_dev_connection : public fd_connection {
 public:
  explicit char_dev_connection(std::string path) : path_(std::move(path)) {}

  int make_fd() override {
    int fd = open(path_.c_str(), O_RDWR | O_NONBLOCK);
    if (fd < 0) detail::throw_io_error(std::string("Error opening ") + path_);
    return fd;
  }

 private:
  std::string path_;
};

}  // namespace ash

#endif  // INCLUDE_ASH_LINUX_CONNECTION_H_
