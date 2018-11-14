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
#include "ash/preprocessor.h"
#include "catch2/catch.hpp"

TEST_CASE("is_bit_transferrable_scalar") {
#define IS_NOT_BIT_TRANSFERRABLE_SCALAR_TYPE_TEST(...)                 \
  SECTION("false for " #__VA_ARGS__) {                                 \
    REQUIRE_FALSE(                                                     \
        ash::traits::is_bit_transferrable_scalar<__VA_ARGS__>::value); \
  }

#define IS_NOT_BIT_TRANSFERRABLE_SCALAR_TYPE_TEST2(...) \
  ASH_NO_PARENS(IS_NOT_BIT_TRANSFERRABLE_SCALAR_TYPE_TEST, __VA_ARGS__)

  ASH_FOREACH(IS_NOT_BIT_TRANSFERRABLE_SCALAR_TYPE_TEST2, ASH_EMPTY,
              (std::string), (std::pair<int, char>));

#define IS_BIT_TRANSFERRABLE_SCALAR_TYPE_TEST(...)                         \
  SECTION("true for " #__VA_ARGS__) {                                      \
    REQUIRE(ash::traits::is_bit_transferrable_scalar<__VA_ARGS__>::value); \
  }

#define IS_BIT_TRANSFERRABLE_SCALAR_TYPE_TEST2(...) \
  ASH_NO_PARENS(IS_BIT_TRANSFERRABLE_SCALAR_TYPE_TEST, __VA_ARGS__)

  ASH_FOREACH(IS_BIT_TRANSFERRABLE_SCALAR_TYPE_TEST2, ASH_EMPTY, (char),
              (signed char), (unsigned char), (int), (signed int),
              (unsigned int), (long), (signed long), (unsigned long), (float),
              (double));
}

TEST_CASE("writable_value_type tests") {
  SECTION("int for const int&") {
    REQUIRE(std::is_same<
            int, ash::traits::writable_value_type<const int&>::type>::value);
  }

  SECTION("std::pair<int, char> for const std::pair<const int, const char>") {
    REQUIRE(
        std::is_same<std::pair<int, char>,
                     ash::traits::writable_value_type<
                         const std::pair<const int, const char>>::type>::value);
  }

  SECTION(
      "std::tuple<int, char, std::string> for const std::tuple<const int, "
      "const char, const std::string>&") {
    REQUIRE(std::is_same<
            std::tuple<int, char, std::string>,
            ash::traits::writable_value_type<const std::tuple<
                const int, const char, const std::string>&>::type>::value);
  }
}
