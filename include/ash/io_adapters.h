/// \file
/// \brief Adapters to link codecs to streams.
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

#ifndef INCLUDE_ASH_IO_ADAPTERS_H_
#define INCLUDE_ASH_IO_ADAPTERS_H_

#include <cstddef>
#include "ash/errors.h"

namespace ash {
// Input adapter base. Implementations should override at least one of
// read or getc and delegate the other. Throw exceptions for I/O errors.
class input_stream {
 public:
  // Read up to l chars into the buffer pointed at by p. Return the
  // actual amount of bytes read, which could be fewer than l if hitting
  // EOF.
  virtual std::size_t read(char* p, std::size_t l) = 0;

  // Ensure that l chars are read, or return status::END_OF_FILE.
  void read_fully(char* p, std::size_t l) {
    std::size_t r = read(p, l);
    if (r < l) {
      throw errors::eof("EOF");
    }
  }

  // Try to read one more char, or block.
  virtual char getc() = 0;

  // Close the stream so that no further read will succeed, but raise EOF
  // instead. This only "closes" the input_stream, but doesn't necessarily close
  // any underlying file or socket.
  virtual void close() = 0;

  virtual ~input_stream() {}
};

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

class input_adapter {
 public:
  input_adapter(input_stream& in)  // NOLINT(runtime/explicit)
      : in_(in) {}

  std::size_t read(char* p, std::size_t l) { return in_.read(p, l); }

  void read_fully(char* p, std::size_t l) { in_.read_fully(p, l); }

  char getc() { return in_.getc(); }

 private:
  input_stream& in_;
};

// Output adapter base. Implementations should override at least one of
// write or putc and delegate the other. Throw exceptions for I/O errors.
class output_stream {
 public:
  // Write l chars out.
  virtual void write(const char* p, std::size_t l) = 0;

  // Write c out.
  virtual void putc(char c) = 0;

  // Flush any data and close the stream so that no further write will succeed,
  // but raise EOF instead. This only "closes" the output_stream, but doesn't
  // necessarily close any underlying file or socket.
  virtual void close() = 0;

  // Close the stream so that no further write will succeed, but raise EOF
  // instead. This only "closes" the output_stream, but doesn't necessarily
  // close any underlying file or socket.
  virtual void flush() = 0;

  virtual ~output_stream() {}
};

void output_stream::write(const char* p, std::size_t l) {
  while (l-- > 0) {
    putc(*p++);
  }
}

void output_stream::putc(char c) { write(&c, 1); }

class output_adapter {
 public:
  output_adapter(output_stream& out)  // NOLINT(runtime/explicit)
      : out_(out) {}

  void write(const char* p, std::size_t l) { out_.write(p, l); }

  void putc(char c) { out_.putc(c); }

 private:
  output_stream& out_;
};

// Output sizer.
class output_sizer {
 public:
  // Get the total number of bytes written so far.
  std::size_t size() { return size_; }

  // Reset the byte count so that we can reuse the object.
  void reset() { size_ = 0; }

  // Write l chars out.
  void write(const char* p, std::size_t l) { size_ += l; }

  // Write c out.
  void putc(char c) { size_++; }

 private:
  std::size_t size_ = 0;
};

}  // namespace ash

#endif  // INCLUDE_ASH_IO_ADAPTERS_H_
