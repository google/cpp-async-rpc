#ifndef ASH_CONFIG_H_
#define ASH_CONFIG_H_

#include "ash/mpt.h"
#include "ash/binary_codecs.h"

namespace ash {

namespace config {

using all_encoders = mpt::pack<
ash::binary_encoder<ash::output_sizer, false>,
ash::big_endian_binary_encoder,
ash::little_endian_binary_encoder
>;

using all_decoders = mpt::pack<
ash::big_endian_binary_decoder,
ash::little_endian_binary_decoder
>;

}  // namespace config

}  // namespace ash

#endif /* ASH_CONFIG_H_ */
