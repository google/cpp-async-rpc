/// \file
/// \brief Stream implementations using `std::string` objects as buffers.
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

#include "ash/string_adapters.h"

namespace ash {

string_input_stream::string_input_stream(const std::string& is,
                                         std::size_t initial_pos)
    : is_(is), pos_(0) {
  pos(initial_pos);
}

std::size_t string_input_stream::read(char* p, std::size_t l) {
  l = std::min(l, is_.size() - pos_);
  std::copy(is_.begin() + pos_, is_.begin() + pos_ + l, p);
  pos_ += l;
  return l;
}

char string_input_stream::getc() {
  if (pos_ >= is_.size()) throw errors::eof("EOF");
  return is_[pos_++];
}

std::size_t string_input_stream::pos() const { return pos_; }

void string_input_stream::pos(std::size_t new_pos) {
  if (new_pos > is_.size()) {
    throw errors::out_of_range("Positioning past end of string");
  }
  pos_ = new_pos;
}

string_output_stream::string_output_stream(std::string& os) : os_(os) {}

void string_output_stream::write(const char* p, std::size_t l) {
  os_.append(p, l);
}

void string_output_stream::putc(char c) { os_.push_back(c); }

void string_output_stream::flush() {}

}  // namespace ash
