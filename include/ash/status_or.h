#ifndef ASH_STATUS_OR_H_
#define ASH_STATUS_OR_H_

#include <utility>

#include "ash/serializable_base.h"
#include "ash/status.h"

namespace ash {

template <typename T>
class status_or : public serializable<status_or<T>> {
public:
	status_or() : status_(::ash::status::UNKNOWN_ERROR) {}
	status_or(T&& t) : status_(::ash::status::OK), t_(std::forward<T>(t)) {}
	status_or(const T& t) : status_(::ash::status::OK), t_(t) {}

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

private:
	::ash::status status_;
	T t_;

public:
	ASH_OWN_TYPE(status_or<T>);
	ASH_FIELDS(status_, t_);
};

template <typename T>
bool ok(const status_or<T>& value) {
	return value.status() == status::OK;
}

template <typename T>
int code(const status_or<T>& value) {
	return static_cast<int>(value.status());
}

template <typename T>
const char* name(const status_or<T>& value) {
	return ::ash::enum_names<status>::name(value.status());
}

#define ASH_ASSIGN_OR_RETURN(LHS, EXPR) do { auto ___result = EXPR; if (!___result.ok()) return ___result.status(); LHS = std::move(___result.value()); } while (false)

}  // namespace ash

#endif /* ASH_STATUS_OR_H_ */
