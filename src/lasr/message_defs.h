/// \file
/// \brief Definitions for RPC wire messages.
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

#ifndef LASR_MESSAGE_DEFS_H_
#define LASR_MESSAGE_DEFS_H_

#include <cstdint>

namespace lasr {
namespace rpc_defs {

using request_id_type = std::uint32_t;

enum class message_type : std::uint8_t {
  REQUEST,         // RPC request
  RESPONSE,        // RPC response
  CANCEL_REQUEST,  // RPC cancel request
};

}  // namespace rpc_defs
}  // namespace lasr

#endif  // LASR_MESSAGE_DEFS_H_
