#ifndef INCLUDE_ASH_STATUS_H_
#define INCLUDE_ASH_STATUS_H_

#include <cstdint>
#include <exception>
#include <utility>
#include "ash/enum.h"
#include "ash/mpt.h"

namespace ash {
ASH_ENUM(status, uint8_t, "Unknown status code", (OK, 0), (END_OF_FILE, 1),
         (INVALID_ARGUMENT, 2), (NOT_FOUND, 3), (OUT_OF_RANGE, 4),
         (FAILED_PRECONDITION, 5), (NOT_IMPLEMENTED, 6), (IO_ERROR, 7),
         (DEADLINE_EXCEEDED, 8), (UNKNOWN_ERROR, 9));

constexpr bool ok(status value) { return value == status::OK; }

constexpr int code(status value) { return static_cast<int>(value); }

const char* name(status value) {
  return ::ash::enum_names<status>::name(value);
}

#if 1
#define ASH_CHECK(x)                                       \
  do {                                                     \
    if (!(x)) {                                            \
      std::cerr << "ERROR(" << __FILE__ << ":" << __LINE__ \
                << "): Check failed: " << #x << std::endl; \
      std::terminate();                                    \
    }                                                      \
  } while (false)
#define ASH_CHECK_OK(x)                                                 \
  do {                                                                  \
    if (!::ash::ok(x)) {                                                \
      std::cerr << "ERROR(" << __FILE__ << ":" << __LINE__              \
                << "): Status not OK: " << ::ash::name(x) << std::endl; \
      std::terminate();                                                 \
    }                                                                   \
  } while (false)
#else
#define ASH_CHECK(x)            \
  do {                          \
    if (!(x)) std::terminate(); \
  } while (false)
#define ASH_CHECK_OK(x) ASH_CHECK(::ash::ok(x))
#endif

#define ASH_RETURN_IF_ERROR(EXPR)                \
  do {                                           \
    const auto& ___result = EXPR;                \
    if (!::ash::ok(___result)) return ___result; \
  } while (false)

namespace detail {
template <typename O>
struct apply_until_first_error {
  explicit apply_until_first_error(O o) : o_(o) {}

  template <typename Current, typename... Args>
  status operator()(status previous_status, Current, Args&&... args) {
    ASH_RETURN_IF_ERROR(previous_status);
    return o_(Current{}, std::forward<Args>(args)...);
  }

  O o_;
};
}  // namespace detail

/// Apply o (which should return ash::status) on every element of t, stopping at
/// the first error.
/// status::OK is returned if everyhting went well; otherwise the first error
/// status is reported back.
template <typename T, typename O, typename... Args>
::ash::status apply_until_first_error(T&& t, O o, Args&&... args) {
  return ::ash::mpt::accumulate(status::OK, t,
                                detail::apply_until_first_error<O>(o),
                                std::forward<Args>(args)...);
}

}  // namespace ash

#endif  // INCLUDE_ASH_STATUS_H_
