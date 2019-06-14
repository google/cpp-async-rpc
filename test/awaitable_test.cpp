/// \file
/// \brief Test for the `arpc/awaitable.h` header.
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

#include "arpc/awaitable.h"
#include <chrono>
#include "arpc/errors.h"
#include "catch2/catch.hpp"

TEST_CASE("read file construction") {
  arpc::awaitable<void> a(1);
  REQUIRE(a.get_fd() == 1);
  REQUIRE(!a.for_write());
  REQUIRE(a.timeout() < std::chrono::milliseconds::zero());
  REQUIRE(!a.for_polling());
}

TEST_CASE("write file construction") {
  arpc::awaitable<void> a(2, true);
  REQUIRE(a.get_fd() == 2);
  REQUIRE(a.for_write());
  REQUIRE(a.timeout() < std::chrono::milliseconds::zero());
  REQUIRE(!a.for_polling());
}

TEST_CASE("timeout construction") {
  arpc::awaitable<void> a(std::chrono::seconds(1));
  REQUIRE(a.get_fd() == -1);
  REQUIRE(!a.for_write());
  REQUIRE(a.timeout() == std::chrono::seconds(1));
  REQUIRE(!a.for_polling());
}

TEST_CASE("polling construction") {
  arpc::awaitable<void> a(std::chrono::seconds(1), true);
  REQUIRE(a.get_fd() == -1);
  REQUIRE(!a.for_write());
  REQUIRE(a.timeout() == std::chrono::seconds(1));
  REQUIRE(a.for_polling());
}

TEST_CASE("never factory method") {
  auto a = arpc::never();
  REQUIRE(a.get_fd() == -1);
  REQUIRE(!a.for_write());
  REQUIRE(a.timeout() < std::chrono::milliseconds::zero());
  REQUIRE(!a.for_polling());
}

TEST_CASE("always factory method") {
  auto a = arpc::always();
  REQUIRE(a.get_fd() == -1);
  REQUIRE(!a.for_write());
  REQUIRE(a.timeout() == std::chrono::milliseconds::zero());
  REQUIRE(!a.for_polling());
}

TEST_CASE("timeout factory method") {
  auto a = arpc::timeout(std::chrono::seconds(1));
  REQUIRE(a.get_fd() == -1);
  REQUIRE(!a.for_write());
  REQUIRE(a.timeout() == std::chrono::seconds(1));
  REQUIRE(!a.for_polling());
}

TEST_CASE("deadline factory method") {
  auto a = arpc::deadline(std::chrono::system_clock::now() +
                          std::chrono::milliseconds(1000));
  REQUIRE(a.get_fd() == -1);
  REQUIRE(!a.for_write());
  REQUIRE(a.timeout() <= std::chrono::milliseconds(1000));
  REQUIRE(a.timeout() > std::chrono::milliseconds(0));
  REQUIRE(!a.for_polling());
}

TEST_CASE("polling factory method") {
  auto a = arpc::polling(std::chrono::seconds(1));
  REQUIRE(a.get_fd() == -1);
  REQUIRE(!a.for_write());
  REQUIRE(a.timeout() == std::chrono::seconds(1));
  REQUIRE(a.for_polling());
}

TEST_CASE("awaitable react functions") {
  SECTION("default function works") {
    REQUIRE_NOTHROW(arpc::always().get_react_fn()());
  }
  SECTION("can return a value") {
    auto a = arpc::always().then([]() { return 13; });
    REQUIRE(a.get_react_fn()() == 13);
  }
  SECTION("can pipe a value") {
    auto a = arpc::always().then([]() { return 13; }).then([](int i) {
      return (static_cast<double>(i) + 0.5);
    });
    REQUIRE(a.get_react_fn()() == 13.5);
  }
  SECTION("can wrap behaviour") {
    auto a = arpc::always().then([]() { return 13; }).decorate([](auto& f) {
      return (static_cast<double>(f()) + 0.5);
    });
    REQUIRE(a.get_react_fn()() == 13.5);
  }
  SECTION("can throw") {
    auto a = arpc::always().then(
        []() { throw arpc::errors::data_mismatch("error"); });
    REQUIRE_THROWS_AS(a.get_react_fn()(), arpc::errors::data_mismatch);
  }
  SECTION("can catch") {
    auto a =
        arpc::always()
            .then([]() -> int { throw arpc::errors::data_mismatch("error"); })
            .except<arpc::errors::data_mismatch>([](...) { return 22; });
    REQUIRE(a.get_react_fn()() == 22);
  }
  SECTION("can catch and keep processing") {
    auto a =
        arpc::always()
            .then([]() -> int { throw arpc::errors::data_mismatch("error"); })
            .except<arpc::errors::data_mismatch>([](...) { return 22; })
            .then([](int i) { return (static_cast<double>(i) + 0.5); });
    REQUIRE(a.get_react_fn()() == 22.5);
  }
  SECTION("can catch and rethrow") {
    auto a =
        arpc::always()
            .then([]() -> int { throw arpc::errors::data_mismatch("error"); })
            .except<arpc::errors::data_mismatch>([](...) -> int { throw; });
    REQUIRE_THROWS_AS(a.get_react_fn()(), arpc::errors::data_mismatch);
  }
  SECTION("can catch and rethrow a different exception") {
    auto a =
        arpc::always()
            .then([]() -> int { throw arpc::errors::data_mismatch("error"); })
            .except<arpc::errors::data_mismatch>(
                [](...) -> int { throw arpc::errors::cancelled("error"); });
    REQUIRE_THROWS_AS(a.get_react_fn()(), arpc::errors::cancelled);
  }
}
