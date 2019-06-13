/// \file
/// \brief Test for the `arpc/mutex.h` header.
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

#include "arpc/mutex.h"
#include <chrono>
#include <mutex>
#include "arpc/awaitable.h"
#include "arpc/context.h"
#include "arpc/errors.h"
#include "arpc/select.h"
#include "arpc/thread.h"
#include "arpc/wait.h"
#include "catch2/catch.hpp"

TEST_CASE("mutex locking") {
  arpc::mutex mu;
  SECTION("with an unlocked mutex") {
    SECTION("try_lock succeeds") { REQUIRE(mu.try_lock()); }
    SECTION("maybe_lock succeeds") { REQUIRE_NOTHROW(mu.maybe_lock()); }
    SECTION("lock succeeds") {
      REQUIRE_NOTHROW(mu.lock());
      SECTION("then we can unlock") {
        REQUIRE_NOTHROW(mu.unlock());
        SECTION("and lock succeeds again") { REQUIRE_NOTHROW(mu.lock()); }
      }
    }
    SECTION("can_lock triggers") {
      auto [can_lock] = arpc::select(mu.can_lock());
      REQUIRE(can_lock);
    }
    SECTION("async_lock triggers") {
      auto [async_lock] = arpc::select(mu.async_lock());
      REQUIRE(async_lock);
    }
  }
  SECTION("with a locked mutex") {
    REQUIRE_NOTHROW(mu.lock());
    SECTION("try_lock fails") { REQUIRE(!mu.try_lock()); }
    SECTION("maybe_lock throws") {
      REQUIRE_THROWS_AS(mu.maybe_lock(), arpc::errors::try_again);
    }
    SECTION("and a timeout") {
      arpc::context ctx;
      ctx.set_timeout(std::chrono::milliseconds(10));
      SECTION("lock times out") {
        REQUIRE_THROWS_AS(mu.lock(), arpc::errors::deadline_exceeded);
      }
      SECTION("can_lock times out") {
        REQUIRE_THROWS_AS(arpc::select(mu.can_lock()),
                          arpc::errors::deadline_exceeded);
      }
      SECTION("async_lock times out") {
        REQUIRE_THROWS_AS(arpc::select(mu.async_lock()),
                          arpc::errors::deadline_exceeded);
      }
    }
    SECTION("unlocking from a different thread lets us progress") {
      arpc::thread th([&mu]() {
        arpc::wait(arpc::timeout(std::chrono::milliseconds(100)));
        mu.unlock();
      });
      REQUIRE_NOTHROW(mu.lock());
      th.join();
    }
  }
}
