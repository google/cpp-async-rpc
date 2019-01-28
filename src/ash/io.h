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

#ifndef ASH_IO_H_
#define ASH_IO_H_

#include <algorithm>
#include "ash/common/io.h"
#include "ash/posix/io.h"

namespace ash {
using namespace ::ash::posix;  // NOLINT(build/namespaces)

class awaitable {
 public:
  explicit awaitable(const ::ash::channel& ch, bool for_write = false);
  explicit awaitable(std::chrono::milliseconds timeout);

  const ::ash::channel& channel() const;
  bool for_write() const;
  std::chrono::milliseconds timeout() const;

 private:
  const ::ash::channel channel_;
  const bool for_write_;
  const std::chrono::milliseconds timeout_ = std::chrono::milliseconds(-1);
};

template <typename Duration>
awaitable timeout(Duration duration) {
  return awaitable(
      std::chrono::duration_cast<std::chrono::milliseconds>(duration));
}

template <typename Timepoint>
awaitable deadline(Timepoint when) {
  std::chrono::milliseconds delta =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          when - std::chrono::system_clock::now());
  return awaitable(std::max(std::chrono::milliseconds::zero(), delta));
}

}  // namespace ash

#endif  // ASH_IO_H_
