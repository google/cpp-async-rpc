/// \file
/// \brief Helper for serialized names.
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

#ifndef LASR_OBJECT_NAME_H_
#define LASR_OBJECT_NAME_H_

#include <string>
#include <utility>
#include "lasr/string_adapters.h"

namespace lasr {

template <typename Encoder, typename Arg>
std::string object_name(Arg&& arg) {
  std::string result;
  string_output_stream sos(result);
  Encoder encoder(sos);
  encoder(std::forward<Arg>(arg));
  return result;
}

}  // namespace lasr

#endif  // LASR_OBJECT_NAME_H_
