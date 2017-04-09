#ifndef ASH_CODES_H_
#define ASH_CODES_H_

#include <cstdint>

#include "ash/singleton.h"
#include "ash/vector_assoc.h"

namespace ash {
/// A signed type that can be use to signal errors (as negative numbers) as well as success/byte count
/// (non-negative numbers). This is purposefully defined as a signed 32 bit integer to reduce the
/// storage costs, and given the fact that it's unlikely that any single data transfer in an embedded
/// environment will reach 2GiB.
using e_size_t = int32_t;

enum class status : e_size_t {
	OK                    =    0,  /// Operation was successful.
	END_OF_FILE           =   -1,  /// End of file reached while reading.
	INVALID_ARGUMENT      =   -2,  /// Some operation argument was wrong.
	NOT_FOUND             =   -3,  /// No data found for the key that was searched.
	OUT_OF_RANGE          =   -4,  /// Some index was out of range.
	FAILED_PRECONDITION   =   -5,  /// Previous state did not support the current operation.
	NOT_IMPLEMENTED       =   -6,  /// Operation not implemented.
	IO_ERROR              =   -7,  /// Unrecoverable error when performing I/O.
	DEADLINE_EXCEEDED     =   -8,  /// Timeout.
	UNKNOWN               = -128,  /// Unknown error.
};

constexpr status to_status(e_size_t value) {
	return (value < 0 ? status(value) : status::OK);
}

constexpr bool error(status value) {
	return value != status::OK;
}

constexpr bool ok(status value) {
	return value == status::OK;
}

constexpr e_size_t code(status value) {
	return static_cast<e_size_t>(value);
}

constexpr bool error(e_size_t value) {
	return error(to_status(value));
}

constexpr bool ok(e_size_t value) {
	return ok(to_status(value));
}

constexpr e_size_t code(e_size_t value) {
	return code(to_status(value));
}

namespace detail {
class status_description : public singleton<status_description>, private vector_map<status, const char*> {
public:
	const char* description(status value) const {
		const auto it = find(value);
		if (it == end()) {
			return description(status::UNKNOWN);
		}
		else {
			return it->second;
		}
	}

private:
	friend class singleton<status_description>;

	status_description() : vector_map<status, const char*> {
		{status::OK,  "OK"},
		{status::END_OF_FILE, "End of file"},
		{status::INVALID_ARGUMENT, "Invalid argument"},
		{status::NOT_FOUND, "Key not found"},
		{status::OUT_OF_RANGE, "Index out of range"},
		{status::FAILED_PRECONDITION, "Failed precondition"},
		{status::NOT_IMPLEMENTED, "Not implemented"},
		{status::IO_ERROR, "I/O error"},
		{status::DEADLINE_EXCEEDED, "Timeout"},
		{status::UNKNOWN, "Unknown error"},
	} {}

};
}  // namespace detail

const char* description(status value) {
	return detail::status_description::get().description(value);
}
const char* description(e_size_t value) {
	return description(to_status(value));
}

}  // namespace ash

#endif /* ASH_CODES_H_ */
