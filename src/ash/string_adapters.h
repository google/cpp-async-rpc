/// \file
/// \brief Stream implementations using `std::string` objects as buffers.
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

#ifndef ASH_STRING_ADAPTERS_H_
#define ASH_STRING_ADAPTERS_H_

#include <algorithm>
#include <cstddef>
#include <string>
#include "ash/errors.h"
#include "ash/io_adapters.h"

namespace ash {

class string_input_stream : public input_stream {
 public:
  explicit string_input_stream(const std::string& is,
                               std::size_t initial_pos = 0);

  std::size_t read(char* p, std::size_t l) override;

  char getc() override;

  std::size_t pos() const;

  void pos(std::size_t new_pos);

 private:
  const std::string& is_;
  std::size_t pos_;
};

class string_output_stream : public output_stream {
 public:
  explicit string_output_stream(std::string& os);

  void write(const char* p, std::size_t l) override;

  void putc(char c) override;

  void flush() override;

 private:
  std::string& os_;
};

}  // namespace ash

#endif  // ASH_STRING_ADAPTERS_H_
