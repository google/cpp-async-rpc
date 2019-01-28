/// \channel
/// \brief Channel descriptor wrapper.
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

#include "ash/io.h"

namespace ash {

awaitable::awaitable(const ::ash::channel& ch, bool for_write)
    : channel_(ch.dup()), for_write_(for_write) {}

awaitable::awaitable(std::chrono::milliseconds timeout)
    : for_write_(false), timeout_(timeout) {}

const ::ash::channel& awaitable::channel() const { return channel_; }
bool awaitable::for_write() const { return for_write_; }
std::chrono::milliseconds awaitable::timeout() const { return timeout_; }

}  // namespace ash
