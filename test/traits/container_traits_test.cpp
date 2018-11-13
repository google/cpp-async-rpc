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

#include "ash/traits/container_traits.h"
#include <array>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "catch2/catch.hpp"

TEST_CASE("is_iterable tests") {
  SECTION("false for int") {
    REQUIRE_FALSE(ash::traits::is_iterable<int>::value);
  }

  SECTION("false for double") {
    REQUIRE_FALSE(ash::traits::is_iterable<double>::value);
  }

  SECTION("false for const char pointer") {
    REQUIRE_FALSE(ash::traits::is_iterable<const char *>::value);
  }

  SECTION("true for char array reference") {
    REQUIRE(ash::traits::is_iterable<char(&)[10]>::value);
  }

  SECTION("false for char array value") {
    REQUIRE_FALSE(ash::traits::is_iterable<char[10]>::value);
  }

  SECTION("true for std::array<char, 10>") {
    REQUIRE(ash::traits::is_iterable<std::array<char, 10>>::value);
  }

  SECTION("true for std::vector<char>") {
    REQUIRE(ash::traits::is_iterable<std::vector<char>>::value);
  }

  SECTION("true for std::set<char>") {
    REQUIRE(ash::traits::is_iterable<std::set<char>>::value);
  }

  SECTION("true for std::map<int, char>") {
    REQUIRE(ash::traits::is_iterable<std::map<int, char>>::value);
  }

  SECTION("true for std::string") {
    REQUIRE(ash::traits::is_iterable<std::string>::value);
  }

  SECTION("true for const char array reference") {
    REQUIRE(ash::traits::is_iterable<const char(&)[10]>::value);
  }

  SECTION("true for const char array value") {
    REQUIRE(ash::traits::is_iterable<const char[10]>::value);
  }

  SECTION("true for const std::array<char, 10>") {
    REQUIRE(ash::traits::is_iterable<const std::array<char, 10>>::value);
  }

  SECTION("true for std::array<const char, 10>") {
    REQUIRE(ash::traits::is_iterable<std::array<const char, 10>>::value);
  }

  SECTION("true for const std::vector<char>") {
    REQUIRE(ash::traits::is_iterable<const std::vector<char>>::value);
  }

  SECTION("true for const std::set<char>") {
    REQUIRE(ash::traits::is_iterable<const std::set<char>>::value);
  }

  SECTION("true for const std::map<int, char>") {
    REQUIRE(ash::traits::is_iterable<const std::map<int, char>>::value);
  }

  SECTION("true for std::map<int, const char>") {
    REQUIRE(ash::traits::is_iterable<std::map<int, const char>>::value);
  }

  SECTION("true for const std::string") {
    REQUIRE(ash::traits::is_iterable<const std::string>::value);
  }
}

TEST_CASE("is_const_iterable tests") {
  SECTION("false for int") {
    REQUIRE_FALSE(ash::traits::is_const_iterable<int>::value);
  }

  SECTION("false for double") {
    REQUIRE_FALSE(ash::traits::is_const_iterable<double>::value);
  }

  SECTION("false for const char pointer") {
    REQUIRE_FALSE(ash::traits::is_const_iterable<const char *>::value);
  }

  SECTION("true for char array reference") {
    REQUIRE(ash::traits::is_const_iterable<char(&)[10]>::value);
  }

  SECTION("true for char array value") {
    REQUIRE(ash::traits::is_const_iterable<char[10]>::value);
  }

  SECTION("true for std::array<char, 10>") {
    REQUIRE(ash::traits::is_const_iterable<std::array<char, 10>>::value);
  }

  SECTION("true for std::vector<char>") {
    REQUIRE(ash::traits::is_const_iterable<std::vector<char>>::value);
  }

  SECTION("true for std::set<char>") {
    REQUIRE(ash::traits::is_const_iterable<std::set<char>>::value);
  }

  SECTION("true for std::map<int, char>") {
    REQUIRE(ash::traits::is_const_iterable<std::map<int, char>>::value);
  }

  SECTION("true for std::string") {
    REQUIRE(ash::traits::is_const_iterable<std::string>::value);
  }

  SECTION("true for const char array reference") {
    REQUIRE(ash::traits::is_const_iterable<const char(&)[10]>::value);
  }

  SECTION("true for const char array value") {
    REQUIRE(ash::traits::is_const_iterable<const char[10]>::value);
  }

  SECTION("true for const std::array<char, 10>") {
    REQUIRE(ash::traits::is_const_iterable<const std::array<char, 10>>::value);
  }

  SECTION("true for std::array<const char, 10>") {
    REQUIRE(ash::traits::is_const_iterable<std::array<const char, 10>>::value);
  }

  SECTION("true for const std::vector<char>") {
    REQUIRE(ash::traits::is_const_iterable<const std::vector<char>>::value);
  }

  SECTION("true for const std::set<char>") {
    REQUIRE(ash::traits::is_const_iterable<const std::set<char>>::value);
  }

  SECTION("true for const std::map<int, char>") {
    REQUIRE(ash::traits::is_const_iterable<const std::map<int, char>>::value);
  }

  SECTION("true for std::map<int, const char>") {
    REQUIRE(ash::traits::is_const_iterable<std::map<int, const char>>::value);
  }

  SECTION("true for const std::string") {
    REQUIRE(ash::traits::is_const_iterable<const std::string>::value);
  }
}

TEST_CASE("is_contiguous_sequence tests") {
  SECTION("false for int") {
    REQUIRE_FALSE(ash::traits::is_contiguous_sequence<int>::value);
  }

  SECTION("false for double") {
    REQUIRE_FALSE(ash::traits::is_contiguous_sequence<double>::value);
  }

  SECTION("false for const char pointer") {
    REQUIRE_FALSE(ash::traits::is_contiguous_sequence<const char *>::value);
  }

  SECTION("true for char array") {
    REQUIRE(ash::traits::is_contiguous_sequence<char[10]>::value);
  }

  SECTION("true for std::array<char, 10>") {
    REQUIRE(ash::traits::is_contiguous_sequence<std::array<char, 10>>::value);
  }

  SECTION("true for std::vector<char>") {
    REQUIRE(ash::traits::is_contiguous_sequence<std::vector<char>>::value);
  }

  SECTION("false for std::set<char>") {
    REQUIRE_FALSE(ash::traits::is_contiguous_sequence<std::set<char>>::value);
  }

  SECTION("false for std::map<int, char>") {
    REQUIRE_FALSE(
        ash::traits::is_contiguous_sequence<std::map<int, char>>::value);
  }

  SECTION("true for std::string") {
    REQUIRE(ash::traits::is_contiguous_sequence<std::string>::value);
  }
}

TEST_CASE("can_reserve_capacity tests") {
  SECTION("false for int") {
    REQUIRE_FALSE(ash::traits::can_reserve_capacity<int>::value);
  }

  SECTION("false for double") {
    REQUIRE_FALSE(ash::traits::can_reserve_capacity<double>::value);
  }

  SECTION("false for const char pointer") {
    REQUIRE_FALSE(ash::traits::can_reserve_capacity<const char *>::value);
  }

  SECTION("false for char array") {
    REQUIRE_FALSE(ash::traits::can_reserve_capacity<char[10]>::value);
  }

  SECTION("false for std::array<char, 10>") {
    REQUIRE_FALSE(
        ash::traits::can_reserve_capacity<std::array<char, 10>>::value);
  }

  SECTION("true for std::vector<char>") {
    REQUIRE(ash::traits::can_reserve_capacity<std::vector<char>>::value);
  }

  SECTION("false for std::set<char>") {
    REQUIRE_FALSE(ash::traits::can_reserve_capacity<std::set<char>>::value);
  }

  SECTION("false for std::map<int, char>") {
    REQUIRE_FALSE(
        ash::traits::can_reserve_capacity<std::map<int, char>>::value);
  }

  SECTION("true for std::string") {
    REQUIRE(ash::traits::can_reserve_capacity<std::string>::value);
  }
}

TEST_CASE("can_be_resized tests") {
  SECTION("false for int") {
    REQUIRE_FALSE(ash::traits::can_be_resized<int>::value);
  }

  SECTION("false for double") {
    REQUIRE_FALSE(ash::traits::can_be_resized<double>::value);
  }

  SECTION("false for const char pointer") {
    REQUIRE_FALSE(ash::traits::can_be_resized<const char *>::value);
  }

  SECTION("false for char array") {
    REQUIRE_FALSE(ash::traits::can_be_resized<char[10]>::value);
  }

  SECTION("false for std::array<char, 10>") {
    REQUIRE_FALSE(ash::traits::can_be_resized<std::array<char, 10>>::value);
  }

  SECTION("true for std::vector<char>") {
    REQUIRE(ash::traits::can_be_resized<std::vector<char>>::value);
  }

  SECTION("false for std::set<char>") {
    REQUIRE_FALSE(ash::traits::can_be_resized<std::set<char>>::value);
  }

  SECTION("false for std::map<int, char>") {
    REQUIRE_FALSE(ash::traits::can_be_resized<std::map<int, char>>::value);
  }

  SECTION("true for std::string") {
    REQUIRE(ash::traits::can_be_resized<std::string>::value);
  }
}

TEST_CASE("has_static_size tests") {
  SECTION("false for int") {
    REQUIRE_FALSE(ash::traits::has_static_size<int>::value);
  }

  SECTION("false for double") {
    REQUIRE_FALSE(ash::traits::has_static_size<double>::value);
  }

  SECTION("false for const char pointer") {
    REQUIRE_FALSE(ash::traits::has_static_size<const char *>::value);
  }

  SECTION("true for char array") {
    REQUIRE(ash::traits::has_static_size<char[10]>::value);
  }

  SECTION("true for std::array<char, 10>") {
    REQUIRE(ash::traits::has_static_size<std::array<char, 10>>::value);
  }

  SECTION("false for std::vector<char>") {
    REQUIRE_FALSE(ash::traits::has_static_size<std::vector<char>>::value);
  }

  SECTION("false for std::set<char>") {
    REQUIRE_FALSE(ash::traits::has_static_size<std::set<char>>::value);
  }

  SECTION("false for std::map<int, char>") {
    REQUIRE_FALSE(ash::traits::has_static_size<std::map<int, char>>::value);
  }

  SECTION("false for std::string") {
    REQUIRE_FALSE(ash::traits::has_static_size<std::string>::value);
  }
}
