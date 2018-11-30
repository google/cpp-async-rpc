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

  virtual void close() {
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

  virtual void close() {
    flush();
    throw errors::not_implemented("Close not implemented");
  }

  virtual void flush() { os_.flush(); }

 private:
  std::ostream& os_;
};

}  // namespace ash

#endif  // INCLUDE_ASH_IOSTREAM_ADAPTERS_H_
