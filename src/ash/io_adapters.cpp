/// \file
/// \brief Adapters to link codecs to streams.
///
/// \copyright
///   Copyright 2019 by Google LLC. All Rights Reserved.
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

#include "ash/io_adapters.h"

namespace ash {

void input_stream::read_fully(char* p, std::size_t l) {
  std::size_t r = read(p, l);
  if (r < l) {
    throw errors::eof("EOF");
  }
}

input_stream::~input_stream() {}

std::size_t input_stream::read(char* p, std::size_t l) {
  std::size_t r = 0;
  while (r < l) {
    try {
      auto c = getc();
      r++;
      *p++ = c;
    } catch (const errors::eof&) {
      break;
    }
  }
  return r;
}

char input_stream::getc() {
  char c;
  read_fully(&c, 1);
  return c;
}

input_adapter::input_adapter(input_stream& in)  // NOLINT(runtime/explicit)
    : in_(in) {}

std::size_t input_adapter::read(char* p, std::size_t l) {
  return in_.read(p, l);
}

void input_adapter::read_fully(char* p, std::size_t l) { in_.read_fully(p, l); }

char input_adapter::getc() { return in_.getc(); }

output_stream::~output_stream() {}

void output_stream::write(const char* p, std::size_t l) {
  while (l-- > 0) {
    putc(*p++);
  }
}

void output_stream::putc(char c) { write(&c, 1); }

output_adapter::output_adapter(output_stream& out)  // NOLINT(runtime/explicit)
    : out_(out) {}

void output_adapter::write(const char* p, std::size_t l) { out_.write(p, l); }

void output_adapter::putc(char c) { out_.putc(c); }

std::size_t output_sizer::size() { return size_; }

void output_sizer::reset() { size_ = 0; }

void output_sizer::write(const char* p, std::size_t l) { size_ += l; }

void output_sizer::putc(char c) { size_++; }

}  // namespace ash
