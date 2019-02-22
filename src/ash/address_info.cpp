/// \file
/// \brief Wrapper for name resolution results.
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

#include "ash/address_info.h"
#include "ash/errors.h"

namespace ash {

void swap(address_info::iterator& a, address_info::iterator& b) noexcept {
  a.swap(b);
}

void swap(address_info::const_iterator& a,
          address_info::const_iterator& b) noexcept {
  a.swap(b);
}

bool address_info::empty() const noexcept { return begin() == end(); }
address_info::size_type address_info::size() const noexcept {
  return std::distance(begin(), end());
}

address_info::iterator address_info::begin() noexcept {
  return iterator(result_.get());
}
address_info::const_iterator address_info::begin() const noexcept {
  return const_iterator(result_.get());
}
address_info::const_iterator address_info::cbegin() const noexcept {
  return const_iterator(result_.get());
}

address_info::iterator address_info::end() noexcept {
  return iterator(nullptr);
}
address_info::const_iterator address_info::end() const noexcept {
  return const_iterator(nullptr);
}
address_info::const_iterator address_info::cend() const noexcept {
  return const_iterator(nullptr);
}

address_info::address_info(struct addrinfo* result)
    : result_(result, &free_result) {}

void address_info::free_result(struct addrinfo* result) {
  freeaddrinfo(result);
}

std::string address_info::to_string(const struct addrinfo& addr) {
  char hostbuf[65];
  char portbuf[6];
  int res =
      getnameinfo(addr.ai_addr, addr.ai_addrlen, hostbuf, sizeof(hostbuf),
                  portbuf, sizeof(portbuf), NI_NUMERICHOST | NI_NUMERICSERV);
  if (res) throw errors::io_error("Can't print address as string");
  std::string host(hostbuf);
  if (host.find(':') != std::string::npos) {
    return "[" + std::string(hostbuf) + "]:" + std::string(portbuf);
  } else {
    return std::string(hostbuf) + ":" + std::string(portbuf);
  }
}

}  // namespace ash
