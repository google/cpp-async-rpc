/// \file
/// \brief Socket channel factory.
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

#include "lasr/socket.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <utility>
#include "lasr/address_resolver.h"
#include "lasr/context.h"
#include "lasr/errors.h"
#include "lasr/select.h"

namespace lasr {
channel socket(int family, int type, int protocol) {
  channel res(::socket(family, type, protocol));
  if (!res) throw_io_error("Error creating socket");
  return res;
}

channel socket(const address& addr) {
  channel res(::socket(addr.family(), addr.socket_type(), addr.protocol()));
  if (!res) throw_io_error("Error creating socket");
  return res;
}

channel dial(endpoint name, bool non_blocking) {
  auto addr_list = address_resolver::get().resolve(std::move(name.active()));

  if (addr_list.empty())
    throw errors::invalid_argument("Can't connect with empty address_list");

  auto deadline_left = context::current().deadline_left();

  auto it = addr_list.begin();
  do {
    context ctx;
    if (deadline_left) ctx.set_timeout(*deadline_left / addr_list.size());

    try {
      auto s = socket(*it);
      s.make_non_blocking(non_blocking);
      s.connect(*it);
      return s;
    } catch (const errors::base_error&) {
      if (++it == addr_list.end()) throw;
    }
  } while (true);
}

listener::listener(endpoint name, bool reuse_addr, bool non_blocking,
                   int backlog)
    : non_blocking_(non_blocking) {
  auto addr_list = address_resolver::get().resolve(std::move(name.passive()));
  for (const auto& addr : addr_list) {
    auto s = socket(addr);
    s.make_non_blocking(non_blocking_)
        .reuse_addr(reuse_addr)
        .bind(addr)
        .listen(backlog);

    listening_.push_back(std::move(s));
  }
  // Do this later so that the captured this pointers don't get invalidated by
  // vector reallocations.
  for (auto& s : listening_) {
    acceptors_.push_back(s.async_accept());
  }
}

channel listener::accept() {
  do {
    if (pending_.size()) {
      auto res = std::move(pending_.back());
      pending_.pop_back();
      return res;
    }
    auto [res] = select(acceptors_);
    for (auto& r : res) {
      if (r) {
        auto s = std::move(r).value();
        s.make_non_blocking(non_blocking_);
        pending_.push_back(std::move(s));
      }
    }
  } while (true);
}

}  // namespace ash
