/// \file
/// \brief Binary format codecs for serialization.
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

#include "lasr/binary_codecs.h"

namespace lasr {

binary_sizer::binary_sizer()
    : binary_encoder<binary_sizer, output_sizer, false>(output_sizer()) {}

std::size_t binary_sizer::size() { return out_.size(); }

void binary_sizer::reset() { out_.reset(); }

}  // namespace ash
