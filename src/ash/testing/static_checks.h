/// \file
/// \brief Helpers for doing compile-time testing on trait templates.
///
/// \copyright
///   Copyright 2019 by Google LLC. All Rights Reserved.
///
/// \copyright
///   Licensed under the Apache License, Version 2.0 (the "License"); you may
///   not use this file except in compliance with the License. You may obtain a
///   copy of the License at
///
/// \copyright
///   http://www.apache.org/licenses/LICENSE-2.0
///
/// \copyright
///   Unless required by applicable law or agreed to in writing, software
///   distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
///   WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
///   License for the specific language governing permissions and limitations
///   under the License.

#ifndef LASR_TESTING_STATIC_CHECKS_H_
#define LASR_TESTING_STATIC_CHECKS_H_

#include <type_traits>

namespace ash {
namespace testing {

template <typename T, T v1, T v2>
struct check_value {
  static_assert(v1 == v2, "check failed");
};

template <typename T1, typename T2>
struct check_type {
  static_assert(std::is_same_v<T1, T2>, "check failed");
};

}  // namespace testing
}  // namespace ash

#endif  // LASR_TESTING_STATIC_CHECKS_H_
