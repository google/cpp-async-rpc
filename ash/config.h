#ifndef ASH_CONFIG_H_
#define ASH_CONFIG_H_

#include "ash/mpt.h"
#include "ash/binary_codecs.h"

namespace ash {

namespace config {

using AllEncoders = mpt::pack<
		ash::BigEndianBinaryEncoder,
		ash::LittleEndianBinaryEncoder>;

using AllDecoders = mpt::pack<
		ash::BigEndianBinaryDecoder,
		ash::LittleEndianBinaryDecoder>;

}  // namespace config

}  // namespace ash


#endif /* ASH_CONFIG_H_ */
