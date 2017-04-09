#ifndef ASH_STATUS_H_
#define ASH_STATUS_H_

#include <cstdint>
#include <exception>
#include <utility>

#include "ash/enum.h"

namespace ash {
ASH_ENUM(status, uint8_t, "Unknown status code",
		(OK, 0),
		(END_OF_FILE, 1),
		(INVALID_ARGUMENT, 2),
		(NOT_FOUND, 3),
		(OUT_OF_RANGE, 4),
		(FAILED_PRECONDITION, 5),
		(NOT_IMPLEMENTED, 6),
		(IO_ERROR, 7),
		(DEADLINE_EXCEEDED, 8),
		(UNKNOWN, 9));

constexpr bool ok(status value) {
	return value == status::OK;
}

constexpr int code(status value) {
	return static_cast<int>(value);
}

const char* name(status value) {
	return ::ash::enum_names<status>::name(value);
}

#define ASH_CHECK(x) do { if (!(x)) std::terminate(); } while (false);

template <typename T>
class status_or {
public:
	status_or(T&& t) : status_(::ash::status::OK), t_(std::forward<T>(t)) {}

	status_or(::ash::status s) : status_(s) {
		ASH_CHECK(!ok());
	}

	bool ok() const {
		return status_ == ::ash::status::OK;
	}

	::ash::status status() const {
		return status_;
	}

	T& value() {
		ASH_CHECK(ok());
		return t_;
	}

	const T& value() const {
		ASH_CHECK(ok());
		return t_;
	}

	// TODO: Make this class serializable.

private:
	::ash::status status_;
	T t_;
};

#define ASH_RETURN_IF_ERROR(EXPR) do { const auto& ___result = EXPR; if (!::ash::ok(___result)) return ___result; } while (false)
#define ASH_ASSIGN_OR_RETURN(LHS, EXPR) do { const auto& ___result = EXPR; if (!___result.ok()) return ___result.status(); LHS = ___result.value(); } while (false)

}  // namespace ash

#endif /* ASH_STATUS_H_ */
