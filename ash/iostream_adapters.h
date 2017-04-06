#ifndef ASH_IOSTREAM_ADAPTERS_H_
#define ASH_IOSTREAM_ADAPTERS_H_

#include <cstddef>
#include <istream>
#include <ostream>
#include <stdexcept>

#include "ash/io_adapters.h"

namespace ash {

class istream_input_stream: public input_stream {
public:
	istream_input_stream(std::istream& is) :
			is_(is) {
		is_.exceptions(std::istream::badbit);
	}

	std::size_t read(char* p, std::size_t l) override {
		is_.read(p, l);
		return is_.gcount();
	}

	int getc() override {
		char c;
		if (is_.get(c)) {
			return c;
		}
		return ASH_EOF;
	}

private:
	std::istream& is_;
};

class ostream_output_stream: public output_stream {
public:
	ostream_output_stream(std::ostream& os) :
			os_(os) {
		os_.exceptions(std::istream::badbit);
	}

	virtual void write(const char* p, std::size_t l) {
		os_.write(p, l);
	}

	virtual void putc(char c) {
		os_.put(c);
	}

private:
	std::ostream& os_;
};

}  // namespace ash

#endif /* ASH_IOSTREAM_ADAPTERS_H_ */
