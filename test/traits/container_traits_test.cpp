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
#include <tuple>
#include <utility>
#include <vector>
#include "ash/preprocessor.h"
#include "catch2/catch.hpp"

TEST_CASE("is_iterable tests") {
#define IS_NOT_ITERABLE_TYPE_TEST(...)                           \
  SECTION("false for " #__VA_ARGS__) {                           \
    REQUIRE_FALSE(ash::traits::is_iterable<__VA_ARGS__>::value); \
  }

#define IS_NOT_ITERABLE_TYPE_TEST2(...) \
  ASH_NO_PARENS(IS_NOT_ITERABLE_TYPE_TEST, __VA_ARGS__)

  ASH_FOREACH(IS_NOT_ITERABLE_TYPE_TEST2, ASH_EMPTY, (int), (double),
              (const char *));

#define IS_ITERABLE_TYPE_TEST(...)                         \
  SECTION("true for " #__VA_ARGS__) {                      \
    REQUIRE(ash::traits::is_iterable<__VA_ARGS__>::value); \
  }

#define IS_ITERABLE_TYPE_TEST2(...) \
  ASH_NO_PARENS(IS_ITERABLE_TYPE_TEST, __VA_ARGS__)

  ASH_FOREACH(IS_ITERABLE_TYPE_TEST2, ASH_EMPTY, (char(&)[10]), (char[10]),
              (std::array<char, 10>), (std::vector<char>), (std::set<char>),
              (std::map<int, char>), (std::string), (const char(&)[10]),
              (const char[10]), (const std::array<char, 10>),
              (std::array<const char, 10>), (const std::set<char>),
              (const std::map<int, char>), (std::map<int, const char>),
              (const std::string));
}

TEST_CASE("is_const_iterable tests") {
#define IS_NOT_CONST_ITERABLE_TYPE_TEST(...)                           \
  SECTION("false for " #__VA_ARGS__) {                                 \
    REQUIRE_FALSE(ash::traits::is_const_iterable<__VA_ARGS__>::value); \
  }

#define IS_NOT_CONST_ITERABLE_TYPE_TEST2(...) \
  ASH_NO_PARENS(IS_NOT_CONST_ITERABLE_TYPE_TEST, __VA_ARGS__)

  ASH_FOREACH(IS_NOT_CONST_ITERABLE_TYPE_TEST2, ASH_EMPTY, (int), (double),
              (const char *));

#define IS_CONST_ITERABLE_TYPE_TEST(...)                         \
  SECTION("true for " #__VA_ARGS__) {                            \
    REQUIRE(ash::traits::is_const_iterable<__VA_ARGS__>::value); \
  }

#define IS_CONST_ITERABLE_TYPE_TEST2(...) \
  ASH_NO_PARENS(IS_CONST_ITERABLE_TYPE_TEST, __VA_ARGS__)

  ASH_FOREACH(IS_CONST_ITERABLE_TYPE_TEST2, ASH_EMPTY, (char(&)[10]),
              (char[10]), (std::array<char, 10>), (std::vector<char>),
              (std::set<char>), (std::map<int, char>), (std::string),
              (const char(&)[10]), (const char[10]),
              (const std::array<char, 10>), (std::array<const char, 10>),
              (const std::set<char>), (const std::map<int, char>),
              (std::map<int, const char>), (const std::string));
}

TEST_CASE("is_contiguous_sequence tests") {
#define IS_NOT_CONTIGUOUS_SEQUENCE_TYPE_TEST(...)                           \
  SECTION("false for " #__VA_ARGS__) {                                      \
    REQUIRE_FALSE(ash::traits::is_contiguous_sequence<__VA_ARGS__>::value); \
  }

#define IS_NOT_CONTIGUOUS_SEQUENCE_TYPE_TEST2(...) \
  ASH_NO_PARENS(IS_NOT_CONTIGUOUS_SEQUENCE_TYPE_TEST, __VA_ARGS__)

  ASH_FOREACH(IS_NOT_CONTIGUOUS_SEQUENCE_TYPE_TEST2, ASH_EMPTY, (int), (double),
              (const char *), (std::set<char>), (std::map<int, char>),
              (const std::set<char>), (const std::map<int, char>),
              (std::map<int, const char>));

#define IS_CONTIGUOUS_SEQUENCE_TYPE_TEST(...)                         \
  SECTION("true for " #__VA_ARGS__) {                                 \
    REQUIRE(ash::traits::is_contiguous_sequence<__VA_ARGS__>::value); \
  }

#define IS_CONTIGUOUS_SEQUENCE_TYPE_TEST2(...) \
  ASH_NO_PARENS(IS_CONTIGUOUS_SEQUENCE_TYPE_TEST, __VA_ARGS__)

  ASH_FOREACH(IS_CONTIGUOUS_SEQUENCE_TYPE_TEST2, ASH_EMPTY, (char(&)[10]),
              (char[10]), (std::array<char, 10>), (std::vector<char>),
              (std::string), (const char(&)[10]), (const char[10]),
              (const std::array<char, 10>), (std::array<const char, 10>),
              (const std::string));
}

TEST_CASE("can_reserve_capacity tests") {
#define IS_NOT_RESERVE_CAPABLE_TYPE_TEST(...)                             \
  SECTION("false for " #__VA_ARGS__) {                                    \
    REQUIRE_FALSE(ash::traits::can_reserve_capacity<__VA_ARGS__>::value); \
  }

#define IS_NOT_RESERVE_CAPABLE_TYPE_TEST2(...) \
  ASH_NO_PARENS(IS_NOT_RESERVE_CAPABLE_TYPE_TEST, __VA_ARGS__)

  ASH_FOREACH(IS_NOT_RESERVE_CAPABLE_TYPE_TEST2, ASH_EMPTY, (int), (double),
              (char *), (std::set<char>), (std::map<int, char>), (char(&)[10]),
              (char[10]), (std::array<char, 10>), (const std::string),
              (const std::vector<char>));

#define IS_RESERVE_CAPABLE_TYPE_TEST(...)                           \
  SECTION("true for " #__VA_ARGS__) {                               \
    REQUIRE(ash::traits::can_reserve_capacity<__VA_ARGS__>::value); \
  }

#define IS_RESERVE_CAPABLE_TYPE_TEST2(...) \
  ASH_NO_PARENS(IS_RESERVE_CAPABLE_TYPE_TEST, __VA_ARGS__)

  ASH_FOREACH(IS_RESERVE_CAPABLE_TYPE_TEST2, ASH_EMPTY, (std::vector<char>),
              (std::string));
}

TEST_CASE("can_be_resized tests") {
#define IS_NOT_RESIZE_CAPABLE_TYPE_TEST(...)                        \
  SECTION("false for " #__VA_ARGS__) {                              \
    REQUIRE_FALSE(ash::traits::can_be_resized<__VA_ARGS__>::value); \
  }

#define IS_NOT_RESIZE_CAPABLE_TYPE_TEST2(...) \
  ASH_NO_PARENS(IS_NOT_RESIZE_CAPABLE_TYPE_TEST, __VA_ARGS__)

  ASH_FOREACH(IS_NOT_RESIZE_CAPABLE_TYPE_TEST2, ASH_EMPTY, (int), (double),
              (char *), (std::set<char>), (std::map<int, char>), (char(&)[10]),
              (char[10]), (std::array<char, 10>), (const std::string),
              (const std::vector<char>));

#define IS_RESIZE_CAPABLE_TYPE_TEST(...)                      \
  SECTION("true for " #__VA_ARGS__) {                         \
    REQUIRE(ash::traits::can_be_resized<__VA_ARGS__>::value); \
  }

#define IS_RESIZE_CAPABLE_TYPE_TEST2(...) \
  ASH_NO_PARENS(IS_RESIZE_CAPABLE_TYPE_TEST, __VA_ARGS__)

  ASH_FOREACH(IS_RESIZE_CAPABLE_TYPE_TEST2, ASH_EMPTY, (std::vector<char>),
              (std::string));
}

TEST_CASE("has_static_size tests") {
#define IS_NOT_STATIC_SIZE_CAPABLE_TYPE_TEST(...)                    \
  SECTION("false for " #__VA_ARGS__) {                               \
    REQUIRE_FALSE(ash::traits::has_static_size<__VA_ARGS__>::value); \
  }

#define IS_NOT_STATIC_SIZE_CAPABLE_TYPE_TEST2(...) \
  ASH_NO_PARENS(IS_NOT_STATIC_SIZE_CAPABLE_TYPE_TEST, __VA_ARGS__)

  ASH_FOREACH(IS_NOT_STATIC_SIZE_CAPABLE_TYPE_TEST2, ASH_EMPTY, (int), (double),
              (char *), (std::set<char>), (std::map<int, char>),
              (std::vector<char>), (std::string));

#define IS_STATIC_SIZE_CAPABLE_TYPE_TEST(...)                  \
  SECTION("true for " #__VA_ARGS__) {                          \
    REQUIRE(ash::traits::has_static_size<__VA_ARGS__>::value); \
  }

#define IS_STATIC_SIZE_CAPABLE_TYPE_TEST2(...) \
  ASH_NO_PARENS(IS_STATIC_SIZE_CAPABLE_TYPE_TEST, __VA_ARGS__)

  ASH_FOREACH(IS_STATIC_SIZE_CAPABLE_TYPE_TEST2, ASH_EMPTY, (char(&)[10]),
              (char[10]), (std::array<char, 10>));
}

TEST_CASE("static_size tests") {
  SECTION("5 for int[5]") {
    REQUIRE(5 == ash::traits::static_size<int[5]>::value);
  }

  SECTION("5 for std::array<int, 5>") {
    REQUIRE(5 == ash::traits::static_size<std::array<int, 5>>::value);
  }

  SECTION("2 for std::pair<int, char>") {
    REQUIRE(2 == ash::traits::static_size<std::pair<int, char>>::value);
  }

  SECTION("3 for std::tuple<int, char, char*>") {
    REQUIRE(3 ==
            ash::traits::static_size<std::tuple<int, char, char *>>::value);
  }
}
