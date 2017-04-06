#ifndef ASH_IO_ADAPTERS_H_
#define ASH_IO_ADAPTERS_H_

#include <cstddef>
#include <stdexcept>

namespace ash {
// Adapter classes for I/O.

constexpr int ASH_EOF = -1;

// Input adapter base. Implementations should override at least one of
// read or getc and delegate the other. Throw exceptions for I/O errors.
class input_stream {
public:
	// Read up to l chars into the buffer pointed at by p. Return the
	// actual amount of bytes read, which could be fewer than l if hitting
	// EOF.
	virtual std::size_t read(char* p, std::size_t l) = 0;

	// Ensure that l chars are read, or throw.
	void read_fully(char* p, std::size_t l) {
		auto r = read(p, l);
		if (r < l) {
			throw std::runtime_error("EOF before full read.");
		}
	}

	// Read one char out, or -1 on EOF.
	virtual int getc() = 0;

	virtual ~input_stream() {
	}
};

std::size_t input_stream::read(char* p, std::size_t l) {
	int c;
	std::size_t r = 0;
	while (r < l && (c = getc() != ASH_EOF)) {
		r++;
		*p++ = c;
	}
	return r;
}

int input_stream::getc() {
	char c;
	if (read(&c, 1) < 1) {
		return ASH_EOF;
	}
	return c;
}

class input_adapter {
public:
	input_adapter(input_stream& in) :
			in_(in) {
	}

	std::size_t read(char* p, std::size_t l) {
		return in_.read(p, l);
	}

	void readFully(char* p, std::size_t l) {
		in_.read_fully(p, l);
	}

	int getc() {
		return in_.getc();
	}

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

	virtual ~output_stream() {
	}
};

void output_stream::write(const char* p, std::size_t l) {
	while (l-- > 0) {
		putc(*p++);
	}
}

void output_stream::putc(char c) {
	write(&c, 1);
}

class output_adapter {
public:
	output_adapter(output_stream& out) :
			out_(out) {
	}

	void write(const char* p, std::size_t l) {
		out_.write(p, l);
	}

	void putc(char c) {
		out_.putc(c);
	}
private:
	output_stream& out_;
};

// Output sizer.
class output_sizer {
public:
	// Get the total number of bytes written so far.
	std::size_t size() {
		return size_;
	}

	// Reset the byte count so that we can reuse the object.
	void reset() {
		size_ = 0;
	}

	// Write l chars out.
	void write(const char* p, std::size_t l) {
		size_ += l;
	}

	// Write c out.
	void putc(char c) {
		size_++;
	}

private:
	std::size_t size_ = 0;
};

}  // namespace ash

#endif /* ASH_IO_ADAPTERS_H_ */
