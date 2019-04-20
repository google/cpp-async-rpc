/// \file
/// \brief Test for the `lasr/serialization_traits.h` header.
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

#include "lasr/traits/serialization_traits.h"
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include "catch2/catch.hpp"
#include "lasr/serializable.h"
#include "lasr/testing/static_checks.h"

struct A : lasr::serializable<A> {
  int x;
  LASR_FIELDS(x);
};

struct B : lasr::serializable<B, A> {
  int y;
  LASR_FIELDS(y);
};

struct C : lasr::serializable<C> {
  LASR_CUSTOM_SERIALIZATION_VERSION(33);
};

struct D {};

template <typename T, bool v>
using check_can_be_serialized =
    lasr::testing::check_value<bool, lasr::traits::can_be_serialized<T>::value,
                               v>;

TEST_CASE("can_be_serialized") {
  check_can_be_serialized<A, true>();
  check_can_be_serialized<B, true>();
  check_can_be_serialized<C, true>();
  check_can_be_serialized<D, false>();
}

template <typename T1, typename T2>
using check_get_base_classes =
    lasr::testing::check_type<typename lasr::traits::get_base_classes<T1>::type,
                              T2>;

TEST_CASE("get_base_classes") {
  check_get_base_classes<A, lasr::mpt::pack<>>();
  check_get_base_classes<B, lasr::mpt::pack<A>>();
  check_get_base_classes<C, lasr::mpt::pack<>>();
  check_get_base_classes<D, lasr::mpt::pack<>>();
}

template <typename T, lasr::traits::custom_serialization_version_type v>
using check_get_custom_serialization_version = lasr::testing::check_value<
    lasr::traits::custom_serialization_version_type,
    lasr::traits::get_custom_serialization_version<T>::value, v>;

TEST_CASE("get_custom_serialization_version") {
  check_get_custom_serialization_version<A, 0>();
  check_get_custom_serialization_version<B, 0>();
  check_get_custom_serialization_version<C, 33>();
  check_get_custom_serialization_version<D, 0>();
}

template <typename T1, typename T2>
using check_get_field_descriptors = lasr::testing::check_type<
    typename lasr::traits::get_field_descriptors<T1>::type, T2>;

TEST_CASE("get_field_descriptors") {
  check_get_field_descriptors<A,
                              lasr::mpt::pack<lasr::field_descriptor<&A::x>>>();
  check_get_field_descriptors<B,
                              lasr::mpt::pack<lasr::field_descriptor<&B::y>>>();
  check_get_field_descriptors<C, lasr::mpt::pack<>>();
  check_get_field_descriptors<D, lasr::mpt::pack<>>();
}

template <typename T, bool v>
using check_has_base_classes =
    lasr::testing::check_value<bool, lasr::traits::has_base_classes<T>::value,
                               v>;

TEST_CASE("has_base_classes") {
  check_has_base_classes<A, true>();
  check_has_base_classes<B, true>();
  check_has_base_classes<C, true>();
  check_has_base_classes<D, false>();
}

template <typename T, bool v>
using check_has_custom_serialization = lasr::testing::check_value<
    bool, lasr::traits::has_custom_serialization<T>::value, v>;

TEST_CASE("has_custom_serialization") {
  check_has_custom_serialization<A, false>();
  check_has_custom_serialization<B, false>();
  check_has_custom_serialization<C, true>();
  check_has_custom_serialization<D, false>();
}

template <typename T, bool v>
using check_has_field_descriptors = lasr::testing::check_value<
    bool, lasr::traits::has_field_descriptors<T>::value, v>;

TEST_CASE("has_field_descriptors") {
  check_has_field_descriptors<A, true>();
  check_has_field_descriptors<B, true>();
  check_has_field_descriptors<C, true>();
  check_has_field_descriptors<D, false>();
}
