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

address_info::address_info(struct addrinfo* result)
    : result_(result, &free_result) {
  struct addrinfo* p = result_.get();
  while (p) {
    push_back(p);
    p = p->ai_next;
  }
}

void address_info::free_result(struct addrinfo* result) {
  freeaddrinfo(result);
}

std::string address_info::to_string(const struct addrinfo* addr) {
  char hostbuf[65];
  char portbuf[6];
  int res =
      getnameinfo(addr->ai_addr, addr->ai_addrlen, hostbuf, sizeof(hostbuf),
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
