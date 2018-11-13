#ifndef INCLUDE_ASH_ERRORS_H_
#define INCLUDE_ASH_ERRORS_H_

#include <stdexcept>

#define ERROR_CLASS(NAME)                    \
  class NAME : public std::runtime_error {   \
    using std::runtime_error::runtime_error; \
  }

namespace ash {
namespace errors {
ERROR_CLASS(eof);
ERROR_CLASS(io_error);
ERROR_CLASS(key_error);
ERROR_CLASS(invalid_state);
ERROR_CLASS(data_mismatch);
}  // namespace errors
}  // namespace ash

#endif  // INCLUDE_ASH_ERRORS_H_
