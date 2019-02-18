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

#ifndef ASH_ADDRESS_INFO_H_
#define ASH_ADDRESS_INFO_H_

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <memory>
#include <string>
#include <vector>

namespace ash {

class address_info : public std::vector<const struct addrinfo*> {
 public:
  address_info(address_info&&) = default;
  address_info& operator=(address_info&&) = default;

  ~address_info() = default;

  static std::string to_string(const struct addrinfo* addr);

 private:
  friend class address_resolver;
  explicit address_info(struct addrinfo* result);
  static void free_result(struct addrinfo* result);
  std::unique_ptr<struct addrinfo, void (*)(struct addrinfo*)> result_;
};

}  // namespace ash

#endif  // ASH_ADDRESS_INFO_H_
