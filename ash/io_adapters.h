#ifndef ASH_IO_ADAPTERS_H_
#define ASH_IO_ADAPTERS_H_

#include <cstddef>
#include <stdexcept>

#include "ash/status.h"

namespace ash {
// Input adapter base. Implementations should override at least one of
// read or getc and delegate the other. Throw exceptions for I/O errors.
class input_stream {
public:
	// Read up to l chars into the buffer pointed at by p. Return the
	// actual amount of bytes read, which could be fewer than l if hitting
	// EOF.
	virtual status_or<std::size_t> read(char* p, std::size_t l) = 0;

	// Ensure that l chars are read, or return status::END_OF_FILE.
	status read_fully(char* p, std::size_t l) {
		std::size_t r;
		ASH_ASSIGN_OR_RETURN(r, read(p, l));
		if (r < l) {
			return status::END_OF_FILE;
		}
		return status::OK;
	}

	// Try to read one more char, or block.
	virtual status_or<char> getc() = 0;

	virtual ~input_stream() {
	}
};

status_or<std::size_t> input_stream::read(char* p, std::size_t l) {
	std::size_t r = 0;
	while (r < l) {
		auto status_or_char = getc();
		if(status_or_char.status() == status::END_OF_FILE) {
			break;
		}
		ASH_RETURN_IF_ERROR(status_or_char.status());

		r++;
		*p++ = status_or_char.value();
	}
	return r;
}

status_or<char> input_stream::getc() {
	char c;
	ASH_RETURN_IF_ERROR(read_fully(&c, 1));
	return c;
}

class input_adapter {
public:
	input_adapter(input_stream& in) :
			in_(in) {
	}

	status_or<std::size_t> read(char* p, std::size_t l) {
		return in_.read(p, l);
	}

	status read_fully(char* p, std::size_t l) {
		return in_.read_fully(p, l);
	}

	status_or<char> getc() {
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
	virtual status write(const char* p, std::size_t l) = 0;

	// Write c out.
	virtual status putc(char c) = 0;

	virtual ~output_stream() {
	}
};

status output_stream::write(const char* p, std::size_t l) {
	while (l-- > 0) {
		ASH_RETURN_IF_ERROR(putc(*p++));
	}
	return status::OK;
}

status output_stream::putc(char c) {
	ASH_RETURN_IF_ERROR(write(&c, 1));
	return status::OK;
}

class output_adapter {
public:
	output_adapter(output_stream& out) :
			out_(out) {
	}

	status write(const char* p, std::size_t l) {
		return out_.write(p, l);
	}

	status putc(char c) {
		return out_.putc(c);
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
	status write(const char* p, std::size_t l) {
		size_ += l;
		return status::OK;
	}

	// Write c out.
	status putc(char c) {
		size_++;
		return status::OK;
	}

private:
	std::size_t size_ = 0;
};

}  // namespace ash

#endif /* ASH_IO_ADAPTERS_H_ */
