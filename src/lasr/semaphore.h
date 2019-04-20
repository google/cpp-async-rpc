/// \file
/// \brief select-friendly semaphore objects.
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

#ifndef LASR_SEMAPHORE_H_
#define LASR_SEMAPHORE_H_

#include "lasr/queue.h"

namespace lasr {

/// A semaphore is just a queue<void>.
using semaphore = queue<void>;

}  // namespace lasr

#endif  // LASR_SEMAPHORE_H_
