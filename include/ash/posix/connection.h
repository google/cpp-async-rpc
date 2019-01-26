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

#include <condition_variable>
#include <mutex>
#include <string>
#include "ash/connection_common.h"
#include "ash/posix/io.h"

namespace ash {
namespace posix {

class fd_connection : public connection {
 protected:
  class fd_lock {
   public:
    explicit fd_lock(fd_connection& conn);
    ~fd_lock();

   private:
    fd_connection& conn_;
  };

 public:
  explicit fd_connection(channel&& fd);
  ~fd_connection() override;
  bool connected() override;
  void disconnect() override;
  void write(const char* data, std::size_t size) override;
  void putc(char c) override;
  void flush() override;
  std::size_t read(char* data, std::size_t size) override;
  char getc() override;

 protected:
  void check_connected();

  std::mutex mu_;
  std::condition_variable idle_;
  channel fd_;
  channel pipe_[2];
  bool closing_ = false;
  int lock_count_ = 0;
};  // namespace ash

class char_dev_connection : public fd_connection {
 public:
  explicit char_dev_connection(const std::string& path);

 protected:
  static channel open_path(const std::string& path);
};

}  // namespace posix
}  // namespace ash

#endif  // INCLUDE_ASH_POSIX_CONNECTION_H_
