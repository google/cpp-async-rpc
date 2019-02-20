/// \file
/// \brief Asynchronous address resolver.
///
/// \copyright
///   Copyright 2018 by Google Inc. All Rights Reserved.
///
/// \copyright
///   Licensed under the Apache License, Version 2.0 (the "License"); you may
///   not use this channel except in compliance with the License. You may obtain
///   a copy of the License at
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

#include "ash/address_resolver.h"
#include <exception>
#include "ash/errors.h"
#include "ash/select.h"

namespace ash {

address_resolver::address_resolver()
    : requests_(queue_size), resolver_thread_([this]() {
        while (true) {
          auto [req] = select(requests_.async_get());
          if (req) {
            struct addrinfo hints = {
                AI_ADDRCONFIG | (req->passive ? AI_PASSIVE : 0), AF_UNSPEC,
                req->datagram ? SOCK_DGRAM : SOCK_STREAM, 0};
            struct addrinfo* result;

            int res = getaddrinfo(
                req->host.empty() ? nullptr : req->host.c_str(),
                req->service.empty() ? nullptr : req->service.c_str(), &hints,
                &result);
            if (res) {
              req->promise.set_exception(std::make_exception_ptr(
                  errors::io_error("Can't resolve name")));

            } else {
              req->promise.set_value(address_info(result));
            }
          }
        }
      }) {}

address_resolver::~address_resolver() {
  resolver_thread_.get_context().cancel();
  resolver_thread_.join();
}

ash::future<address_info> address_resolver::resolve(const std::string& host,
                                                    int port, bool passive,
                                                    bool datagram) {
  request req{host, std::to_string(port), passive, datagram};
  auto fut = req.promise.get_future();
  requests_.put(std::move(req));
  return fut;
}
ash::future<address_info> address_resolver::resolve(const std::string& host,
                                                    const std::string& service,
                                                    bool passive,
                                                    bool datagram) {
  request req{host, service, passive, datagram};
  auto fut = req.promise.get_future();
  requests_.put(std::move(req));
  return fut;
}

}  // namespace ash
