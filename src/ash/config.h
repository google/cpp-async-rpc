/// \file
/// \brief Header providing static configuration for Ash components.
///
/// \copyright
///   Copyright 2018 by Google Inc. All Rights Reserved.
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

#ifndef ASH_CONFIG_H_
#define ASH_CONFIG_H_

#include "ash/binary_codecs.h"
#include "ash/mpt.h"

namespace ash {
namespace config {

using all_encoders =
    mpt::pack<ash::binary_sizer, ash::big_endian_binary_encoder,
              ash::little_endian_binary_encoder>;

using all_decoders = mpt::pack<ash::big_endian_binary_decoder,
                               ash::little_endian_binary_decoder>;

}  // namespace config
}  // namespace ash

#endif  // ASH_CONFIG_H_
