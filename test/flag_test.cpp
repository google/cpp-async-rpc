/// \file
/// \brief Test for the `arpc/flag.h` header.
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

#include "arpc/flag.h"
#include <chrono>
#include <mutex>
#include "arpc/awaitable.h"
#include "arpc/context.h"
#include "arpc/errors.h"
#include "arpc/select.h"
#include "arpc/thread.h"
#include "catch2/catch.hpp"

TEST_CASE("flag signaling") {
  arpc::flag fl;
  SECTION("with a non-signaled state") {
    SECTION("is_set returns false") { REQUIRE(!fl.is_set()); }
    SECTION("bool conversion returns false") { REQUIRE(!fl); }
    SECTION("set succeeds") {
      REQUIRE_NOTHROW(fl.set());
      SECTION("then is_set returns true") { REQUIRE(fl.is_set()); }
      SECTION("then we can reset") {
        REQUIRE_NOTHROW(fl.reset());
        SECTION("and is_set returns false again") { REQUIRE(!fl.is_set()); }
      }
    }
    SECTION("and a timeout") {
      arpc::context ctx;
      ctx.set_timeout(std::chrono::milliseconds(10));
      SECTION("async_wait times out") {
        REQUIRE_THROWS_AS(arpc::select(fl.async_wait()),
                          arpc::errors::deadline_exceeded);
      }
    }
    SECTION("setting from a different thread lets us progress") {
      arpc::thread th([&fl]() {
        try {
          auto [res] =
              arpc::select(arpc::timeout(std::chrono::milliseconds(100)));
        } catch (const arpc::errors::deadline_exceeded&) {
        }
        fl.set();
      });
      fl.wait();
      th.join();
    }
  }
  SECTION("with a signaled state") {
    REQUIRE_NOTHROW(fl.set());
    SECTION("is_set returns true") { REQUIRE(fl.is_set()); }
    SECTION("bool conversion returns true") { REQUIRE(fl); }
    SECTION("wait succeeds") { REQUIRE_NOTHROW(fl.wait()); }
    SECTION("reset succeeds") {
      REQUIRE_NOTHROW(fl.reset());
      SECTION("then is_set returns false") { REQUIRE(!fl.is_set()); }
      SECTION("then we can set") {
        REQUIRE_NOTHROW(fl.set());
        SECTION("and is_set returns trueagain") { REQUIRE(fl.is_set()); }
      }
    }
    SECTION("async_wait triggers") {
      auto [res] = arpc::select(fl.async_wait());
      REQUIRE(res);
    }
  }
}
