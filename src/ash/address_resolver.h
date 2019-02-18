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

#include <string>
#include <tuple>
#include "ash/address_info.h"
#include "ash/future.h"
#include "ash/queue.h"
#include "ash/singleton.h"
#include "ash/thread.h"

namespace ash {

class address_resolver : public singleton<address_resolver> {
 public:
  ash::future<address_info> resolve(const std::string& host, int port,
                                    bool passive = false,
                                    bool datagram = false);
  ash::future<address_info> resolve(const std::string& host,
                                    const std::string& service,
                                    bool passive = false,
                                    bool datagram = false);

 private:
  friend class singleton<address_resolver>;

  struct request {
    std::string host;
    std::string service;
    bool passive;
    bool datagram;
    ash::promise<address_info> promise;
  };

  static constexpr queue<request>::size_type queue_size = 16;

  address_resolver();
  ~address_resolver();

  queue<request> requests_;
  ash::thread resolver_thread_;
};

}  // namespace ash

#endif  // ASH_ADDRESS_RESOLVER_H_
