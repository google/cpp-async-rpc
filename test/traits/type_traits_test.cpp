/// \file
/// \brief Test for the `ash/container_traits.h` header.
///
/// \copyright
///   Copyright 2018 by Google Inc. All Rights Reserved.
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

#include "ash/traits/type_traits.h"
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include "ash/testing/static_checks.h"
#include "catch2/catch.hpp"

template <typename T, bool v>
using check_is_bit_transferrable_scalar =
    ash::testing::check_value<bool,
                              ash::traits::is_bit_transferrable_scalar_v<T>, v>;

TEST_CASE("is_bit_transferrable_scalar") {
  check_is_bit_transferrable_scalar<std::string, false>();
  check_is_bit_transferrable_scalar<std::pair<int, char>, false>();

  check_is_bit_transferrable_scalar<char, true>();
  check_is_bit_transferrable_scalar<signed char, true>();
  check_is_bit_transferrable_scalar<unsigned char, true>();
  check_is_bit_transferrable_scalar<bool, true>();
  check_is_bit_transferrable_scalar<int, true>();
  check_is_bit_transferrable_scalar<signed int, true>();
  check_is_bit_transferrable_scalar<unsigned int, true>();
  check_is_bit_transferrable_scalar<long, true>();  // NOLINT(runtime/int)
  check_is_bit_transferrable_scalar<signed long,    // NOLINT(runtime/int)
                                    true>();
  check_is_bit_transferrable_scalar<unsigned long,  // NOLINT(runtime/int)
                                    true>();
  check_is_bit_transferrable_scalar<long long, true>();  // NOLINT(runtime/int)
  check_is_bit_transferrable_scalar<signed long long,    // NOLINT(runtime/int)
                                    true>();
  check_is_bit_transferrable_scalar<unsigned long long,  // NOLINT(runtime/int)
                                    true>();
}

template <typename T1, typename T2>
using check_writable_value_type =
    ash::testing::check_type<typename ash::traits::writable_value_type_t<T1>,
                             T2>;

TEST_CASE("writable_value_type") {
  check_writable_value_type<const int&, int>();
  check_writable_value_type<const std::pair<const int, const char>,
                            std::pair<int, char>>();
  check_writable_value_type<
      const std::tuple<const int, const char, const std::string>&,
      std::tuple<int, char, std::string>>();
}
