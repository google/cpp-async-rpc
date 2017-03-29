#ifndef ASH_IOSTREAM_ADAPTERS_H_
#define ASH_IOSTREAM_ADAPTERS_H_

#include <cstddef>
#include <istream>
#include <ostream>
#include <stdexcept>

#include "ash/io_adapters.h"

namespace ash {

class IStreamAdapter: public InputAdapter {
public:
	IStreamAdapter(std::istream& is) :
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

class OStreamAdapter: public OutputAdapter {
public:
	OStreamAdapter(std::ostream& os) :
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
