/// \file
/// \brief Asynchronous address resolver.
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

#include <exception>
#include "lasr/address_resolver.h"
#include "lasr/errors.h"
#include "lasr/select.h"

namespace lasr {

address_resolver::address_resolver()
    : requests_(queue_size), resolver_thread_([this]() {
        while (true) {
          auto [req_pair] = select(requests_.async_get());
          if (req_pair) {
            auto& [req, promise] = *req_pair;

            struct addrinfo hints = {
                AI_ADDRCONFIG | AI_V4MAPPED | (req.passive_ ? AI_PASSIVE : 0),
                req.family_, req.sock_type_, 0};
            struct addrinfo* result;

            int res = getaddrinfo(
                req.name_.empty() ? nullptr : req.name_.c_str(),
                req.service_.empty() ? nullptr : req.service_.c_str(), &hints,
                &result);
            if (res) {
              promise.set_exception(std::make_exception_ptr(
                  errors::io_error("Can't resolve name")));

            } else {
              promise.set_value(address_list(result));
            }
          }
        }
      }) {}

address_resolver::~address_resolver() {
  resolver_thread_.get_context().cancel();
  resolver_thread_.join();
}

lasr::future<address_list> address_resolver::async_resolve(const endpoint& req) {
  auto req_pair = std::pair(req, promise<address_list>{});
  auto fut = req_pair.second.get_future();
  requests_.put(std::move(req_pair));
  return fut;
}

address_list address_resolver::resolve(const endpoint& req) {
  return async_resolve(req).get();
}

lasr::future<address_list> address_resolver::async_resolve(endpoint&& req) {
  auto req_pair = std::pair(std::move(req), promise<address_list>{});
  auto fut = req_pair.second.get_future();
  requests_.put(std::move(req_pair));
  return fut;
}

address_list address_resolver::resolve(endpoint&& req) {
  return async_resolve(std::move(req)).get();
}
}  // namespace lasr
