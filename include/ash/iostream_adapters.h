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

#ifndef INCLUDE_ASH_IOSTREAM_ADAPTERS_H_
#define INCLUDE_ASH_IOSTREAM_ADAPTERS_H_

#include <cstddef>
#include <istream>
#include <ostream>
#include "ash/errors.h"
#include "ash/io_adapters.h"

namespace ash {

class istream_input_stream : public input_stream {
 public:
  explicit istream_input_stream(std::istream& is) : is_(is) {}

  std::size_t read(char* p, std::size_t l) override {
    is_.read(p, l);
    if (is_.bad()) throw errors::io_error("Bad input stream");
    return static_cast<std::size_t>(is_.gcount());
  }

  char getc() override {
    char c;
    if (is_.get(c)) {
      return c;
    }
    if (is_.eof()) throw errors::eof("EOF");
    throw errors::io_error("Bad input stream");
  }

  void close() override {
    throw errors::not_implemented("Close not implemented");
  }

 private:
  std::istream& is_;
};

class ostream_output_stream : public output_stream {
 public:
  explicit ostream_output_stream(std::ostream& os) : os_(os) {}

  void write(const char* p, std::size_t l) override {
    if (!os_.write(p, l)) throw errors::io_error("Bad output stream");
  }

  void putc(char c) override {
    if (!os_.put(c)) throw errors::io_error("Bad output stream");
  }

  void close() override {
    flush();
    throw errors::not_implemented("Close not implemented");
  }

  void flush() override { os_.flush(); }

 private:
  std::ostream& os_;
};

}  // namespace ash

#endif  // INCLUDE_ASH_IOSTREAM_ADAPTERS_H_
