/// \file
/// \brief Test for the `ash/serialization_traits.h` header.
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

#include "ash/traits/serialization_traits.h"
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include "ash/serializable.h"
#include "ash/testing/static_checks.h"
#include "catch2/catch.hpp"

struct A : ash::serializable<A> {
  int x;
  ASH_FIELDS(x);
};

struct B : ash::serializable<B, A> {
  int y;
  ASH_FIELDS(y);
};

struct C : ash::serializable<C> {
  ASH_CUSTOM_SERIALIZATION_VERSION(33);
};

struct D {};

template <typename T, bool v>
using check_can_be_serialized =
    ash::testing::check_value<bool, ash::traits::can_be_serialized<T>::value,
                              v>;

TEST_CASE("can_be_serialized") {
  check_can_be_serialized<A, true>();
  check_can_be_serialized<B, true>();
  check_can_be_serialized<C, true>();
  check_can_be_serialized<D, false>();
}

template <typename T1, typename T2>
using check_get_base_classes =
    ash::testing::check_type<typename ash::traits::get_base_classes<T1>::type,
                             T2>;

TEST_CASE("get_base_classes") {
  check_get_base_classes<A, ash::mpt::pack<>>();
  check_get_base_classes<B, ash::mpt::pack<A>>();
  check_get_base_classes<C, ash::mpt::pack<>>();
  check_get_base_classes<D, ash::mpt::pack<>>();
}

template <typename T, ash::traits::custom_serialization_version_type v>
using check_get_custom_serialization_version = ash::testing::check_value<
    ash::traits::custom_serialization_version_type,
    ash::traits::get_custom_serialization_version<T>::value, v>;

TEST_CASE("get_custom_serialization_version") {
  check_get_custom_serialization_version<A, 0>();
  check_get_custom_serialization_version<B, 0>();
  check_get_custom_serialization_version<C, 33>();
  check_get_custom_serialization_version<D, 0>();
}

template <typename T1, typename T2>
using check_get_field_pointers =
    ash::testing::check_type<typename ash::traits::get_field_pointers<T1>::type,
                             T2>;

TEST_CASE("get_field_pointers") {
  check_get_field_pointers<A, ash::mpt::value_pack<&A::x>>();
  check_get_field_pointers<B, ash::mpt::value_pack<&B::y>>();
  check_get_field_pointers<C, ash::mpt::value_pack<>>();
  check_get_field_pointers<D, ash::mpt::value_pack<>>();
}

template <typename T, bool v>
using check_has_base_classes =
    ash::testing::check_value<bool, ash::traits::has_base_classes<T>::value, v>;

TEST_CASE("has_base_classes") {
  check_has_base_classes<A, true>();
  check_has_base_classes<B, true>();
  check_has_base_classes<C, true>();
  check_has_base_classes<D, false>();
}

template <typename T, bool v>
using check_has_custom_serialization = ash::testing::check_value<
    bool, ash::traits::has_custom_serialization<T>::value, v>;

TEST_CASE("has_custom_serialization") {
  check_has_custom_serialization<A, false>();
  check_has_custom_serialization<B, false>();
  check_has_custom_serialization<C, true>();
  check_has_custom_serialization<D, false>();
}

template <typename T, bool v>
using check_has_field_pointers =
    ash::testing::check_value<bool, ash::traits::has_field_pointers<T>::value,
                              v>;

TEST_CASE("has_field_pointers") {
  check_has_field_pointers<A, true>();
  check_has_field_pointers<B, true>();
  check_has_field_pointers<C, true>();
  check_has_field_pointers<D, false>();
}
