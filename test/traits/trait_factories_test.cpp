/// \file
/// \brief Test for the `ash/trait_factories.h` header.
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

#include "ash/traits/trait_factories.h"
#include "ash/testing/static_checks.h"
#include "catch2/catch.hpp"

struct A {
  static constexpr int x = 1;
  static void s_v();
  int i();
  int i_c() const;
  using t = char;
};

struct B {};

struct C {
  static void s_v(double d);
  int i(double d);
  int i_c(double d) const;
};

struct D {
  static int s_v();
  double i();
  double i_c() const;
};

ASH_MAKE_METHOD_CHECKER(has_s_v, s_v);
template <typename T, bool v>
using check_has_s_v =
    ash::testing::check_value<bool, has_s_v<T, void()>::value, v>;

ASH_MAKE_METHOD_CHECKER(has_i, i);
template <typename T, bool v>
using check_has_i = ash::testing::check_value<bool, has_i<T, int()>::value, v>;

TEST_CASE("method_checker") {
  check_has_s_v<A, true>();
  check_has_s_v<B, false>();
  check_has_s_v<C, false>();
  check_has_s_v<D, false>();

  check_has_i<A, true>();
  check_has_i<B, false>();
  check_has_i<C, false>();
  check_has_i<D, false>();
}

ASH_MAKE_METHOD_CHECKER(has_s_v_c, s_v);
template <typename T, bool v>
using check_has_s_v_c =
    ash::testing::check_value<bool, has_s_v_c<T, void()>::value, v>;

ASH_MAKE_CONST_METHOD_CHECKER(has_i_c, i_c);
template <typename T, bool v>
using check_has_i_c =
    ash::testing::check_value<bool, has_i_c<T, int()>::value, v>;

TEST_CASE("const_method_checker") {
  check_has_s_v_c<A, true>();
  check_has_s_v_c<B, false>();
  check_has_s_v_c<C, false>();
  check_has_s_v_c<D, false>();

  check_has_i_c<A, true>();
  check_has_i_c<B, false>();
  check_has_i_c<C, false>();
  check_has_i_c<D, false>();
}

ASH_MAKE_NESTED_CONSTANT_CHECKER(has_x, x);
template <typename T, bool v>
using check_has_x = ash::testing::check_value<bool, has_x<T, int>::value, v>;

TEST_CASE("nested_constant_checker") {
  check_has_x<A, true>();
  check_has_x<B, false>();
  check_has_x<C, false>();
  check_has_x<D, false>();
}

ASH_MAKE_NESTED_TYPE_CHECKER(has_t, t);
template <typename T, bool v>
using check_has_t = ash::testing::check_value<bool, has_t<T>::value, v>;

TEST_CASE("nested_type_checker") {
  check_has_t<A, true>();
  check_has_t<B, false>();
  check_has_t<C, false>();
  check_has_t<D, false>();
}
