#ifndef ASH_SERIALIZABLE_H_
#define ASH_SERIALIZABLE_H_

#include "ash/config.h"
#include "ash/dynamic_base_class.h"
#include "ash/registry.h"
#include "ash/serializable_base.h"

namespace ash {

/// Register a dynamic class into the class factories.
#define ASH_REGISTER(...) \
template <> const char* ::ash::detail::dynamic_class_descriptor< __VA_ARGS__ >::class_name = ::ash::registry::dynamic_object_factory::get().register_class< __VA_ARGS__ , ::ash::config::all_encoders, ::ash::config::all_decoders>(#__VA_ARGS__)

/// Register a dynamic class into the class factories under a custom name.
#define ASH_REGISTER_WITH_NAME(NAME, ...) \
template <> const char* ::ash::detail::dynamic_class_descriptor< __VA_ARGS__ >::class_name = ::ash::registry::dynamic_object_factory::get().register_class< __VA_ARGS__ , ::ash::config::all_encoders, ::ash::config::all_decoders>(NAME)

}  // namespace ash

#endif /* ASH_SERIALIZABLE_H_ */
