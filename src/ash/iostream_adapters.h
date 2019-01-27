/// \file
/// \brief Stream implementations delegating to `iostream` stream objects.
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

#ifndef ASH_IOSTREAM_ADAPTERS_H_
#define ASH_IOSTREAM_ADAPTERS_H_

#include <cstddef>
#include <istream>
#include <ostream>
#include "ash/io_adapters.h"

namespace ash {

class istream_input_stream : public input_stream {
 public:
  explicit istream_input_stream(std::istream& is);

  std::size_t read(char* p, std::size_t l) override;

  char getc() override;

 private:
  std::istream& is_;
};

class ostream_output_stream : public output_stream {
 public:
  explicit ostream_output_stream(std::ostream& os);

  ~ostream_output_stream() override;

  void write(const char* p, std::size_t l) override;

  void putc(char c) override;

  void flush() override;

 private:
  std::ostream& os_;
};

}  // namespace ash

#endif  // ASH_IOSTREAM_ADAPTERS_H_
