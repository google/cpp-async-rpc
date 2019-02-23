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

#ifndef ASH_ADDRESS_RESOLVER_H_
#define ASH_ADDRESS_RESOLVER_H_

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string>
#include <utility>
#include "ash/address_info.h"
#include "ash/future.h"
#include "ash/queue.h"
#include "ash/singleton.h"
#include "ash/thread.h"

namespace ash {

class address_resolver : public singleton<address_resolver> {
 public:
  class request {
   public:
    request& name(const std::string& new_name);
    request& service(const std::string& new_service);
    request& port(int new_port);
    request& passive();
    request& active();
    request& stream();
    request& datagram();
    request& ip();
    request& ipv4();
    request& ipv6();

   private:
    friend class address_resolver;

    std::string name_;
    std::string service_;
    bool passive_ = false;
    int family_ = AF_UNSPEC;
    int sock_type_ = SOCK_STREAM;
  };

  ash::future<address_info> resolve(const request& req);

 private:
  friend class singleton<address_resolver>;
  using queue_type = queue<std::pair<request, promise<address_info>>>;

  static constexpr queue_type::size_type queue_size = 16;

  address_resolver();
  ~address_resolver();

  queue_type requests_;
  ash::thread resolver_thread_;
};

}  // namespace ash

#endif  // ASH_ADDRESS_RESOLVER_H_
