/// \file
/// \brief Test for the `arpc/type_hash.h` header.
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

#include "arpc/type_hash.h"
#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include "arpc/serializable.h"
#include "arpc/testing/static_checks.h"
#include "catch2/catch.hpp"

struct A : arpc::serializable<A> {
  int x;
  ARPC_FIELDS(x);
};

struct B : arpc::serializable<B> {
  int z;
  ARPC_FIELDS(z);
};

struct C : arpc::serializable<C, A> {
  char y;
  ARPC_FIELDS(y);
};

struct D : arpc::serializable<D, B> {
  char q;
  ARPC_FIELDS(q);
};

struct E : arpc::serializable<E> {
  ARPC_CUSTOM_SERIALIZATION_VERSION(33);
};

struct F : arpc::serializable<F> {
  std::unique_ptr<F> x;
  ARPC_FIELDS(x);
};

struct G : arpc::serializable<G> {
  std::shared_ptr<G> x;
  ARPC_FIELDS(x);
};

struct H : arpc::serializable<H> {
  std::weak_ptr<H> x;
  ARPC_FIELDS(x);
};

struct J;

struct I : arpc::serializable<I> {
  std::shared_ptr<J> x;
  ARPC_FIELDS(x);
};

struct J : arpc::serializable<J> {
  std::shared_ptr<I> x;
  ARPC_FIELDS(x);
};

template <typename T, arpc::traits::type_hash_t v>
using check_type_hash =
    arpc::testing::check_value<arpc::traits::type_hash_t,
                               arpc::traits::type_hash_v<T>, v>;

TEST_CASE("type variant hashing") {
  check_type_hash<char, 258>();
  check_type_hash<const char, 258>();
  check_type_hash<char&, 258>();
  check_type_hash<const char&, 258>();
  check_type_hash<char&&, 258>();
  check_type_hash<const char&&, 258>();
}

TEST_CASE("basic type hashing") {
  check_type_hash<void, 0>();
  check_type_hash<bool, 257>();
  check_type_hash<char, 258>();
  check_type_hash<unsigned char, 259>();
  check_type_hash<signed char, 387>();
  check_type_hash<float, 1156>();
  check_type_hash<double, 2180>();
  check_type_hash<std::uint8_t, 259>();
  check_type_hash<std::int8_t, 387>();
  check_type_hash<std::uint16_t, 515>();
  check_type_hash<std::int16_t, 643>();
  check_type_hash<std::uint32_t, 1027>();
  check_type_hash<std::int32_t, 1155>();
  check_type_hash<std::uint64_t, 2051>();
  check_type_hash<std::int64_t, 2179>();
}

TEST_CASE("serializable type hashing") {
  check_type_hash<A, 2395276125>();
  check_type_hash<B, 2395276125>();
  check_type_hash<C, 4130864777>();
  check_type_hash<D, 4130864777>();
  check_type_hash<E, 339237524>();
}

TEST_CASE("array type hashing") {
  check_type_hash<char[2], 100871792>();
  check_type_hash<std::array<char, 2>, 100871792>();
}

TEST_CASE("sequence type hashing") {
  check_type_hash<std::string, 134221210>();
  check_type_hash<std::vector<char>, 134221210>();
}

TEST_CASE("associative type hashing") {
  check_type_hash<std::set<std::uint32_t>, 150997544>();
  check_type_hash<std::multiset<std::uint32_t>, 150997544>();
  check_type_hash<std::unordered_set<std::uint32_t>, 150997544>();
  check_type_hash<std::set<std::string>, 16778161>();
  check_type_hash<std::unordered_set<std::string>, 16778161>();
  check_type_hash<std::set<std::pair<std::string, std::uint32_t>>,
                  1209558117>();
  check_type_hash<std::map<std::string, std::uint32_t>, 1259890160>();
  check_type_hash<std::multimap<std::string, std::uint32_t>, 1259890160>();
  check_type_hash<std::unordered_map<std::string, std::uint32_t>, 1259890160>();
  check_type_hash<std::map<std::string, bool>, 1259888882>();
  check_type_hash<std::unordered_map<std::string, bool>, 1259888882>();
}

TEST_CASE("tuple type hashing") {
  check_type_hash<std::pair<double, bool>, 2333312786>();
  check_type_hash<std::tuple<double, bool>, 2333312786>();
  check_type_hash<std::tuple<double, bool, char>, 3899508564>();
}

TEST_CASE("optional type hashing") {
  check_type_hash<std::optional<double>, 184555989>();
  check_type_hash<std::optional<bool>, 184553552>();
}

TEST_CASE("chrono type hashing") {
  check_type_hash<std::chrono::milliseconds, 255884>();
  check_type_hash<std::chrono::hours, 921356>();
  check_type_hash<std::chrono::time_point<std::chrono::system_clock,
                                          std::chrono::milliseconds>,
                  255885>();
}

TEST_CASE("function type hashing") {
  check_type_hash<void(char), 2250908155>();
  check_type_hash<void(const char&), 2250908155>();
  check_type_hash<void(char) const, 2271695995>();
  check_type_hash<void(const char&) const, 2271695995>();
  check_type_hash<bool(char), 2234233700>();
  check_type_hash<bool(), 285219778>();
  check_type_hash<bool(float, double), 2245061954>();
}

TEST_CASE("pointer type hashing") {
  check_type_hash<std::unique_ptr<void>, 234886666>();
  check_type_hash<std::shared_ptr<void>, 251664285>();
  check_type_hash<std::weak_ptr<void>, 268441904>();
  check_type_hash<std::unique_ptr<bool>, 234886923>();
  check_type_hash<std::shared_ptr<bool>, 251664028>();
  check_type_hash<std::weak_ptr<bool>, 268441649>();
}

TEST_CASE("type cycle hashing") {
  check_type_hash<F, 4088272144>();
  check_type_hash<G, 263225253>();
  check_type_hash<H, 1671033618>();
  check_type_hash<I, 794330892>();
  check_type_hash<J, 794330892>();
}
