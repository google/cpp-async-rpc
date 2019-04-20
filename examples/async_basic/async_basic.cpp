/// \file
/// \brief Basic test of same-process RPC server and client.
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

#include <chrono>
#include <exception>
#include <iostream>
#include <string>
#include "lasr/awaitable.h"
#include "lasr/context.h"
#include "lasr/errors.h"
#include "lasr/select.h"
#include "lasr/socket.h"

int main(int argc, char* argv[]) {
  try {
    lasr::context ctx;
    ctx.set_timeout(std::chrono::seconds(10));

    auto s =
        lasr::dial(lasr::endpoint().name("www.kernel.org").service("http"));

    std::string request = "GET / HTTP/1.0\r\nHost: www.kernel.org\r\n\r\n";
    char buf[256];

    while (true) {
      auto [sent, received] = lasr::select(
          request.size() ? s.async_write(request.data(), request.size())
                         : lasr::never().then([]() { return std::size_t{0}; }),
          s.async_read(buf, sizeof(buf)));

      if (sent) {
        std::cout << "S(" << *sent << ")" << std::endl;
        // Remove the bytes we already sent.
        request.erase(0, *sent);
      }

      if (received) {
        std::cout << "R(" << *received << ")" << std::endl
                  << std::string(buf, buf + *received) << std::endl;
      }
    }

    return 0;
  } catch (const lasr::errors::base_error& e) {
    std::cerr << "Exception of type " << e.portable_error_class_name()
              << " with message: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Some other exception." << std::endl;
    return 1;
  }
}
