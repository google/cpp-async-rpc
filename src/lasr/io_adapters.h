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

#ifndef LASR_IO_ADAPTERS_H_
#define LASR_IO_ADAPTERS_H_

#include <cstddef>

#include "lasr/errors.h"

namespace lasr {
// Input adapter base. Implementations should override at least one of
// read or getc and delegate the other. Throw exceptions for I/O errors.
class input_stream {
 public:
  // Read up to l chars into the buffer pointed at by p. Return the
  // actual amount of bytes read, which could be fewer than l if hitting
  // EOF.
  virtual std::size_t read(char* p, std::size_t l) = 0;

  // Ensure that l chars are read, or raise errors::eof.
  void read_fully(char* p, std::size_t l);

  // Try to read one more char, or block.
  virtual char getc() = 0;

  virtual ~input_stream();
};

class input_adapter {
 public:
  input_adapter(input_stream& in);  // NOLINT(runtime/explicit)

  std::size_t read(char* p, std::size_t l);

  void read_fully(char* p, std::size_t l);

  char getc();

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

  // Close the stream so that no further write will succeed.
  virtual void flush() = 0;

  virtual ~output_stream();
};

class output_adapter {
 public:
  output_adapter(output_stream& out);  // NOLINT(runtime/explicit)

  void write(const char* p, std::size_t l);

  void putc(char c);

 private:
  output_stream& out_;
};

// Output sizer.
class output_sizer {
 public:
  // Get the total number of bytes written so far.
  std::size_t size();

  // Reset the byte count so that we can reuse the object.
  void reset();

  // Write l chars out.
  void write(const char* p, std::size_t l);

  // Write c out.
  void putc(char c);

 private:
  std::size_t size_ = 0;
};

}  // namespace ash

#endif  // LASR_IO_ADAPTERS_H_
