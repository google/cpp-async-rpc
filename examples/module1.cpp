/// \file
/// \brief Test compilation unit.
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

#include "module1.h"
#include <chrono>
#include <iostream>
#include "arpc/channel.h"
#include "arpc/select.h"

void run_module1() {
  arpc::channel in(0);
  auto [read, timeout] =
      arpc::select(in.can_read(), arpc::timeout(std::chrono::milliseconds(3000)));
  std::cerr << !!read << !!timeout << std::endl;
  in.release();
}
