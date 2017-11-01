#ifndef INCLUDE_ASH_CONFIG_H_
#define INCLUDE_ASH_CONFIG_H_

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

#endif  // INCLUDE_ASH_CONFIG_H_
