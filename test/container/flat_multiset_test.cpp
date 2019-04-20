/// \file
/// \brief Test for `flat_multiset` class.
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

#include <utility>
#include <vector>
#include "catch2/catch.hpp"
#include "lasr/container/flat_set.h"

TEST_CASE("flat_multiset tests") {
  using set = lasr::flat_multiset<int>;
  using vector = std::vector<int>;

  set s1;
  set s2{4, 3};
  set s3{3, 4, 3};

  SECTION("empty true for empty set") { REQUIRE(s1.empty()); }

  SECTION("empty false for non-empty set") { REQUIRE_FALSE(s2.empty()); }

  SECTION("size returns the number of elements") {
    REQUIRE(s1.size() == 0);
    REQUIRE(s2.size() == 2);
    REQUIRE(s3.size() == 3);
  }

  SECTION("items returned in the expected order") {
    REQUIRE(vector(s1.begin(), s1.end()) == (vector{}));
    REQUIRE(vector(s2.begin(), s2.end()) == (vector{3, 4}));
    REQUIRE(vector(s3.begin(), s3.end()) == (vector{3, 3, 4}));
  }

  SECTION("find missing item returns end()") {
    REQUIRE(s1.find(1) == s1.end());
    REQUIRE(s2.find(1) == s2.end());
    REQUIRE(s3.find(1) == s3.end());
  }

  SECTION("find first item returns begin()") {
    REQUIRE(s2.find(3) == s2.begin());
    REQUIRE(s3.find(3) == s3.begin());
  }

  SECTION("find existing item returns non-end and can be dereferenced") {
    REQUIRE(s2.find(4) != s2.end());
    REQUIRE(s3.find(4) != s3.end());

    REQUIRE(*(s2.find(4)) == 4);
    REQUIRE(*(s3.find(4)) == 4);
  }

  SECTION("inserting individual items works") {
    REQUIRE(s1.insert(3).second);
    REQUIRE(s2.insert(4).second);
    REQUIRE(s3.insert(5).second);

    REQUIRE(vector(s1.begin(), s1.end()) == (vector{3}));
    REQUIRE(vector(s2.begin(), s2.end()) == (vector{3, 4, 4}));
    REQUIRE(vector(s3.begin(), s3.end()) == (vector{3, 3, 4, 5}));
  }

  SECTION("inserting individual items with hints works") {
    s1.insert(s1.begin(), 3);
    s2.insert(s2.end(), 4);
    s3.insert(s3.begin(), 5);

    REQUIRE(vector(s1.begin(), s1.end()) == (vector{3}));
    REQUIRE(vector(s2.begin(), s2.end()) == (vector{3, 4, 4}));
    REQUIRE(vector(s3.begin(), s3.end()) == (vector{3, 3, 4, 5}));
  }

  SECTION("inserting a range works") {
    s1.insert(s2.begin(), s2.end());

    REQUIRE(vector(s1.begin(), s1.end()) == (vector{3, 4}));
  }

  SECTION("erasing individual values works") {
    REQUIRE(s1.erase(3) == 0);
    REQUIRE(s2.erase(4) == 1);
    REQUIRE(s3.erase(5) == 0);

    REQUIRE(vector(s1.begin(), s1.end()) == (vector{}));
    REQUIRE(vector(s2.begin(), s2.end()) == (vector{3}));
    REQUIRE(vector(s3.begin(), s3.end()) == (vector{3, 3, 4}));
  }

  SECTION("erasing individual items with iterators works") {
    s2.erase(s2.begin());
    s3.erase(s3.begin() + 1);

    REQUIRE(vector(s2.begin(), s2.end()) == (vector{4}));
    REQUIRE(vector(s3.begin(), s3.end()) == (vector{3, 4}));
  }

  SECTION("erasing a range works") {
    s2.erase(s2.begin(), s2.end());
    s3.erase(s3.begin(), s3.begin() + 1);

    REQUIRE(vector(s2.begin(), s2.end()) == (vector{}));
    REQUIRE(vector(s3.begin(), s3.end()) == (vector{3, 4}));
  }

  SECTION("lower_bound") {
    REQUIRE(s1.lower_bound(3) == s1.end());
    REQUIRE(s3.lower_bound(3) == s3.begin());
    REQUIRE(s3.lower_bound(4) == s3.begin() + 2);
    REQUIRE(s3.lower_bound(5) == s3.end());
  }

  SECTION("upper_bound") {
    REQUIRE(s1.upper_bound(3) == s1.end());
    REQUIRE(s3.upper_bound(3) == s3.begin() + 2);
    REQUIRE(s3.upper_bound(4) == s3.end());
  }

  SECTION("equal_range") {
    REQUIRE(s1.equal_range(3) == std::make_pair(s1.end(), s1.end()));
    REQUIRE(s3.equal_range(3) == std::make_pair(s3.begin(), s3.begin() + 2));
    REQUIRE(s3.equal_range(4) == std::make_pair(s3.begin() + 2, s3.end()));
  }

  SECTION("count") {
    REQUIRE(s1.count(3) == 0);
    REQUIRE(s2.count(3) == 1);
    REQUIRE(s3.count(3) == 2);
  }
}
