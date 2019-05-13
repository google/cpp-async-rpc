/// \file
/// \brief Stream implementations delegating to `iostream` stream objects.
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

#include "arpc/iostream_adapters.h"
#include "arpc/errors.h"

namespace arpc {

istream_input_stream::istream_input_stream(std::istream& is) : is_(is) {}

std::size_t istream_input_stream::read(char* p, std::size_t l) {
  is_.read(p, l);
  if (is_.bad()) throw errors::io_error("Bad input stream");
  return static_cast<std::size_t>(is_.gcount());
}

char istream_input_stream::getc() {
  char c;
  if (is_.get(c)) {
    return c;
  }
  if (is_.eof()) throw errors::eof("EOF");
  throw errors::io_error("Bad input stream");
}

ostream_output_stream::ostream_output_stream(std::ostream& os) : os_(os) {}

ostream_output_stream::~ostream_output_stream() { flush(); }

void ostream_output_stream::write(const char* p, std::size_t l) {
  if (!os_.write(p, l)) throw errors::io_error("Bad output stream");
}

void ostream_output_stream::putc(char c) {
  if (!os_.put(c)) throw errors::io_error("Bad output stream");
}

void ostream_output_stream::flush() { os_.flush(); }

}  // namespace arpc
