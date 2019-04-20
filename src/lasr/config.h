/// \file
/// \brief Header providing static configuration for Ash components.
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

#ifndef LASR_CONFIG_H_
#define LASR_CONFIG_H_

#include "lasr/binary_codecs.h"
#include "lasr/mpt.h"

namespace lasr {
namespace config {

using all_encoders =
    mpt::pack<lasr::binary_sizer, lasr::big_endian_binary_encoder,
              lasr::little_endian_binary_encoder>;

using all_decoders = mpt::pack<lasr::big_endian_binary_decoder,
                               lasr::little_endian_binary_decoder>;

}  // namespace config
}  // namespace lasr

#endif  // LASR_CONFIG_H_
