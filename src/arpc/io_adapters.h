/// \file
/// \brief Adapters to link codecs to streams.
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

#ifndef ARPC_IO_ADAPTERS_H_
#define ARPC_IO_ADAPTERS_H_

#include <cstddef>
#include "arpc/errors.h"

namespace arpc {
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

}  // namespace arpc

#endif  // ARPC_IO_ADAPTERS_H_
