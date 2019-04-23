/// \file
/// \brief Channel descriptor wrapper.
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

#include "lasr/pipe.h"
#include "lasr/errors.h"
#ifndef ESP_PLATFORM
#include <unistd.h>
#else  // ESP_PLATFORM
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "lasr/socket.h"
#endif  // ESP_PLATFORM

namespace lasr {

#ifdef ESP_PLATFORM
namespace {
#if LWIP_IPV4
const struct sockaddr_in loopback_ip_address = {
    sizeof(struct sockaddr_in), AF_INET, htons(0), htonl(INADDR_LOOPBACK), {0}};
constexpr auto loopback_family = AF_INET;
#elif LWIP_IPV6
const struct sockaddr_in6 loopback_ip_address = {sizeof(struct sockaddr_in6),
                                                 AF_INET6, 0, in6addr_loopback};
constexpr auto loopback_family = AF_INET6;
#else  // LWIP_IPV4 || LWIP_IPV6
#error "Neither LWIP_IPV4 or LWIP_IPV6 defined!"
#endif  // LWIP_IPV4 || LWIP_IPV6
}  // namespace
#endif  // ESP_PLATFORM

void pipe(channel fds[2]) {
#ifndef ESP_PLATFORM
  int fd[2] = {-1, -1};
  if (::pipe(fd)) throw_io_error("Error creating pipe pair");
  fds[0].reset(fd[0]);
  fds[1].reset(fd[1]);
#else   // ESP_PLATFORM
  channel fd[2];
  fd[0] = socket(loopback_family, SOCK_DGRAM, 0);
  fd[1] = socket(loopback_family, SOCK_DGRAM, 0);
  // Everything else is done using the native socket API to prevent dependency
  // cycles (async calls depend on the context's cancellation flag in select)
  // and the synchronous calls are implemented in terms of the asynchronous
  // ones.
  if (::bind(fd[0].get(),
             reinterpret_cast<const sockaddr*>(&loopback_ip_address),
             sizeof(loopback_ip_address)))
    throw_io_error("Failed to bind pipe socket");
  if (::bind(fd[1].get(),
             reinterpret_cast<const sockaddr*>(&loopback_ip_address),
             sizeof(loopback_ip_address)))
    throw_io_error("Failed to bind pipe socket");
  struct sockaddr_storage own_addr;
  socklen_t own_addr_len = sizeof(own_addr);
  if (::getsockname(fd[0].get(), reinterpret_cast<sockaddr*>(&own_addr),
                    &own_addr_len))
    throw_io_error("Can't get pipe socket port");
  if (::connect(fd[1].get(), reinterpret_cast<const sockaddr*>(&own_addr),
                own_addr_len))
    throw_io_error("Can't connect pipe socket back");

  fds[0].swap(fd[0]);
  fds[1].swap(fd[1]);
#endif  // ESP_PLATFORM
}

}  // namespace lasr
