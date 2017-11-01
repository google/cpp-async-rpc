#ifndef INCLUDE_ASH_IOSTREAM_ADAPTERS_H_
#define INCLUDE_ASH_IOSTREAM_ADAPTERS_H_

#include <cstddef>
#include <istream>
#include <ostream>
#include <stdexcept>
#include "ash/io_adapters.h"

namespace ash {

class istream_input_stream : public input_stream {
 public:
  explicit istream_input_stream(std::istream& is) : is_(is) {}

  status_or<std::size_t> read(char* p, std::size_t l) override {
    is_.read(p, l);
    if (is_.bad()) return status::IO_ERROR;
    return static_cast<std::size_t>(is_.gcount());
  }

  status_or<char> getc() override {
    char c;
    if (is_.get(c)) {
      return c;
    }
    if (is_.bad()) return status::IO_ERROR;
    return status::END_OF_FILE;
  }

 private:
  std::istream& is_;
};

class ostream_output_stream : public output_stream {
 public:
  explicit ostream_output_stream(std::ostream& os) : os_(os) {}

  status write(const char* p, std::size_t l) override {
    os_.write(p, l);
    if (os_.bad()) return status::IO_ERROR;
    return status::OK;
  }

  status putc(char c) override {
    os_.put(c);
    if (os_.bad()) return status::IO_ERROR;
    return status::OK;
  }

 private:
  std::ostream& os_;
};

}  // namespace ash

#endif  // INCLUDE_ASH_IOSTREAM_ADAPTERS_H_
