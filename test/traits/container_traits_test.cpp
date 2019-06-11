/// \file
/// \brief Test for the `arpc/traits/container_traits.h` header.
///
/// \copyright
///   Copyright 2019 by Google LLC.
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

#include "arpc/traits/container_traits.h"
#include <array>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include "arpc/container/flat_map.h"
#include "arpc/container/flat_set.h"
#include "arpc/testing/static_checks.h"
#include "catch2/catch.hpp"

template <typename T, bool v>
using check_is_iterable =
    arpc::testing::check_value<bool, arpc::traits::is_iterable_v<T>, v>;

TEST_CASE("is_iterable tests") {
  check_is_iterable<int, false>();
  check_is_iterable<double, false>();
  check_is_iterable<const char *, false>();

  check_is_iterable<char(&)[10], true>();
  check_is_iterable<char[10], true>();
  check_is_iterable<std::array<char, 10>, true>();
  check_is_iterable<std::vector<char>, true>();
  check_is_iterable<std::set<char>, true>();
  check_is_iterable<std::map<int, char>, true>();
  check_is_iterable<std::string, true>();
  check_is_iterable<std::string_view, true>();
  check_is_iterable<const char(&)[10], true>();
  check_is_iterable<const char[10], true>();
  check_is_iterable<const std::array<char, 10>, true>();
  check_is_iterable<std::array<const char, 10>, true>();
  check_is_iterable<const std::set<char>, true>();
  check_is_iterable<const std::map<int, char>, true>();
  check_is_iterable<std::map<int, const char>, true>();
  check_is_iterable<const std::string, true>();
  check_is_iterable<const std::string_view, true>();
}

template <typename T, bool v>
using check_is_const_iterable =
    arpc::testing::check_value<bool, arpc::traits::is_const_iterable_v<T>, v>;

TEST_CASE("is_const_iterable tests") {
  check_is_const_iterable<int, false>();
  check_is_const_iterable<double, false>();
  check_is_const_iterable<const char *, false>();

  check_is_const_iterable<char(&)[10], true>();
  check_is_const_iterable<char[10], true>();
  check_is_const_iterable<std::array<char, 10>, true>();
  check_is_const_iterable<std::vector<char>, true>();
  check_is_const_iterable<std::set<char>, true>();
  check_is_const_iterable<std::map<int, char>, true>();
  check_is_const_iterable<std::string, true>();
  check_is_const_iterable<std::string_view, true>();
  check_is_const_iterable<const char(&)[10], true>();
  check_is_const_iterable<const char[10], true>();
  check_is_const_iterable<const std::array<char, 10>, true>();
  check_is_const_iterable<std::array<const char, 10>, true>();
  check_is_const_iterable<const std::set<char>, true>();
  check_is_const_iterable<const std::map<int, char>, true>();
  check_is_const_iterable<std::map<int, const char>, true>();
  check_is_const_iterable<const std::string, true>();
  check_is_const_iterable<const std::string_view, true>();
}

template <typename T, bool v>
using check_is_contiguous_sequence =
    arpc::testing::check_value<bool, arpc::traits::is_contiguous_sequence_v<T>,
                               v>;

TEST_CASE("is_contiguous_sequence tests") {
  check_is_contiguous_sequence<int, false>();
  check_is_contiguous_sequence<double, false>();
  check_is_contiguous_sequence<const char *, false>();
  check_is_contiguous_sequence<std::set<char>, false>();
  check_is_contiguous_sequence<std::map<int, char>, false>();
  check_is_contiguous_sequence<const std::set<char>, false>();
  check_is_contiguous_sequence<const std::map<int, char>, false>();
  check_is_contiguous_sequence<std::map<int, const char>, false>();

  check_is_contiguous_sequence<char[10], true>();
  check_is_contiguous_sequence<std::array<char, 10>, true>();
  check_is_contiguous_sequence<std::vector<char>, true>();
  check_is_contiguous_sequence<std::string, true>();
  check_is_contiguous_sequence<std::string_view, true>();
  check_is_contiguous_sequence<const char(&)[10], true>();
  check_is_contiguous_sequence<const char[10], true>();
  check_is_contiguous_sequence<const std::array<char, 10>, true>();
  check_is_contiguous_sequence<std::array<const char, 10>, true>();
  check_is_contiguous_sequence<const std::string, true>();
  check_is_contiguous_sequence<const std::string_view, true>();
}

template <typename T, bool v>
using check_can_reserve_capacity =
    arpc::testing::check_value<bool, arpc::traits::can_reserve_capacity_v<T>,
                               v>;

TEST_CASE("can_reserve_capacity tests") {
  check_can_reserve_capacity<int, false>();
  check_can_reserve_capacity<double, false>();
  check_can_reserve_capacity<char *, false>();
  check_can_reserve_capacity<std::set<char>, false>();
  check_can_reserve_capacity<std::map<int, char>, false>();
  check_can_reserve_capacity<char(&)[10], false>();
  check_can_reserve_capacity<char[10], false>();
  check_can_reserve_capacity<std::array<char, 10>, false>();
  check_can_reserve_capacity<const std::string, false>();
  check_can_reserve_capacity<const std::vector<char>, false>();

  check_can_reserve_capacity<std::vector<char>, true>();
  check_can_reserve_capacity<std::string, true>();
  check_can_reserve_capacity<std::string_view, false>();
}

template <typename T, bool v>
using check_can_be_resized =
    arpc::testing::check_value<bool, arpc::traits::can_be_resized_v<T>, v>;

TEST_CASE("can_be_resized tests") {
  check_can_be_resized<int, false>();
  check_can_be_resized<double, false>();
  check_can_be_resized<char *, false>();
  check_can_be_resized<std::set<char>, false>();
  check_can_be_resized<std::map<int, char>, false>();
  check_can_be_resized<char(&)[10], false>();
  check_can_be_resized<char[10], false>();
  check_can_be_resized<std::array<char, 10>, false>();
  check_can_be_resized<const std::string, false>();
  check_can_be_resized<const std::vector<char>, false>();
  check_can_be_resized<std::string_view, false>();

  check_can_be_resized<std::vector<char>, true>();
  check_can_be_resized<std::string, true>();
}

template <typename T, bool v>
using check_has_static_size =
    arpc::testing::check_value<bool, arpc::traits::has_static_size_v<T>, v>;

TEST_CASE("has_static_size tests") {
  check_has_static_size<int, false>();
  check_has_static_size<double, false>();
  check_has_static_size<char *, false>();
  check_has_static_size<std::set<char>, false>();
  check_has_static_size<std::map<int, char>, false>();
  check_has_static_size<std::vector<char>, false>();
  check_has_static_size<std::string, false>();
  check_has_static_size<std::string_view, false>();

  check_has_static_size<char(&)[10], true>();
  check_has_static_size<char[10], true>();
  check_has_static_size<std::array<char, 10>, true>();
  check_has_static_size<const std::array<char, 10>, true>();
}

template <typename T, std::size_t v>
using check_static_size =
    arpc::testing::check_value<std::size_t, arpc::traits::static_size_v<T>, v>;

TEST_CASE("static_size tests") {
  check_static_size<int[5], 5>();
  check_static_size<std::array<int, 5>, 5>();
  check_static_size<std::pair<int, char>, 2>();
  check_static_size<std::tuple<int, char, char *>, 3>();
  check_static_size<const std::array<int, 5>, 5>();
  check_static_size<const std::pair<int, char>, 2>();
  check_static_size<const std::tuple<int, char, char *>, 3>();
}

template <typename T, bool v>
using check_is_associative =
    arpc::testing::check_value<bool, arpc::traits::is_associative_v<T>, v>;

TEST_CASE("is_associative tests") {
  check_is_associative<int, false>();
  check_is_associative<double, false>();
  check_is_associative<char *, false>();
  check_is_associative<std::vector<char>, false>();
  check_is_associative<std::string, false>();
  check_is_associative<std::string_view, false>();
  check_is_associative<char(&)[10], false>();
  check_is_associative<char[10], false>();
  check_is_associative<std::array<char, 10>, false>();
  check_is_associative<const std::array<char, 10>, false>();
  check_is_associative<const std::string_view, false>();

  check_is_associative<std::set<char>, true>();
  check_is_associative<std::map<int, char>, true>();
  check_is_associative<std::multiset<char>, true>();
  check_is_associative<std::multimap<int, char>, true>();
  check_is_associative<std::unordered_set<char>, true>();
  check_is_associative<std::unordered_map<int, char>, true>();
}

template <typename T, bool v>
using check_is_map =
    arpc::testing::check_value<bool, arpc::traits::is_map_v<T>, v>;

TEST_CASE("is_map tests") {
  check_is_map<int, false>();
  check_is_map<double, false>();
  check_is_map<char *, false>();
  check_is_map<std::vector<char>, false>();
  check_is_map<std::string, false>();
  check_is_map<std::string_view, false>();
  check_is_map<char(&)[10], false>();
  check_is_map<char[10], false>();
  check_is_map<std::array<char, 10>, false>();
  check_is_map<const std::array<char, 10>, false>();
  check_is_map<const std::string_view, false>();
  check_is_map<std::set<char>, false>();
  check_is_map<arpc::flat_set<char>, false>();
  check_is_map<std::multiset<char>, false>();
  check_is_map<arpc::flat_multiset<char>, false>();
  check_is_map<std::unordered_set<char>, false>();

  check_is_map<std::map<int, char>, true>();
  check_is_map<arpc::flat_map<int, char>, true>();
  check_is_map<std::multimap<int, char>, true>();
  check_is_map<arpc::flat_multimap<int, char>, true>();
  check_is_map<std::unordered_map<int, char>, true>();
}

template <typename T, bool v>
using check_is_set =
    arpc::testing::check_value<bool, arpc::traits::is_set_v<T>, v>;

TEST_CASE("is_set tests") {
  check_is_set<int, false>();
  check_is_set<double, false>();
  check_is_set<char *, false>();
  check_is_set<std::vector<char>, false>();
  check_is_set<std::string, false>();
  check_is_set<std::string_view, false>();
  check_is_set<char(&)[10], false>();
  check_is_set<char[10], false>();
  check_is_set<std::array<char, 10>, false>();
  check_is_set<const std::array<char, 10>, false>();
  check_is_set<const std::string_view, false>();
  check_is_set<std::map<int, char>, false>();
  check_is_set<arpc::flat_map<int, char>, false>();
  check_is_set<std::multimap<int, char>, false>();
  check_is_set<arpc::flat_multimap<int, char>, false>();
  check_is_set<std::unordered_map<int, char>, false>();

  check_is_set<std::set<char>, true>();
  check_is_set<arpc::flat_set<char>, true>();
  check_is_set<std::multiset<char>, true>();
  check_is_set<arpc::flat_multiset<char>, true>();
  check_is_set<std::unordered_set<char>, true>();
}

template <typename T, bool v>
using check_is_multi_key_associative =
    arpc::testing::check_value<bool,
                               arpc::traits::is_multi_key_associative_v<T>, v>;

TEST_CASE("is_multi_key_associative tests") {
  check_is_multi_key_associative<int, false>();
  check_is_multi_key_associative<double, false>();
  check_is_multi_key_associative<char *, false>();
  check_is_multi_key_associative<std::vector<char>, false>();
  check_is_multi_key_associative<std::string, false>();
  check_is_multi_key_associative<std::string_view, false>();
  check_is_multi_key_associative<char(&)[10], false>();
  check_is_multi_key_associative<char[10], false>();
  check_is_multi_key_associative<std::array<char, 10>, false>();
  check_is_multi_key_associative<const std::array<char, 10>, false>();
  check_is_multi_key_associative<const std::string_view, false>();
  check_is_multi_key_associative<std::set<char>, false>();
  check_is_multi_key_associative<std::unordered_set<char>, false>();
  check_is_multi_key_associative<arpc::flat_set<char>, false>();
  check_is_multi_key_associative<std::map<int, char>, false>();
  check_is_multi_key_associative<std::unordered_map<int, char>, false>();
  check_is_multi_key_associative<arpc::flat_map<int, char>, false>();

  check_is_multi_key_associative<std::multiset<char>, true>();
  check_is_multi_key_associative<arpc::flat_multiset<char>, true>();
  check_is_multi_key_associative<std::multimap<int, char>, true>();
  check_is_multi_key_associative<arpc::flat_multimap<int, char>, true>();
}
