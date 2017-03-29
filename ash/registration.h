#ifndef ASH_REGISTRATION_H_
#define ASH_REGISTRATION_H_

#include <memory>
#include <stdexcept>
#include <typeinfo>
#include <typeindex>
#include <typeindex>
#include <unordered_map>

#include "ash/registry.h"
#include "ash/metadata.h"
#include "ash/static_object.h"
#include "ash/config.h"

namespace ash {

namespace registry {
/*
template <typename T>
class EncoderRegisterer {
private:
	template <typename S>
	struct BaseRegisterer {
		template <typename B>
		void operator()(mpt::wrap_type<B>, const char* name) {
			auto& registry = StaticObject<EncoderRegistry<S, B>>::get();
			registry.template registerClass<T>(name);
		}
	};

	struct Registerer {
		template <typename S>
		void operator()(mpt::wrap_type<S>, const char* name) {
			mpt::for_each(metadata::DynamicBaseClasses<T>{}, BaseRegisterer<S>{}, name);
		}
	};

public:
	EncoderRegisterer(const char* name) {
		mpt::for_each(config::AllEncoders{}, Registerer{}, name);
	}
	virtual ~EncoderRegisterer() {
		// Nothing.
	}
};
*/

#define ASH_CONCAT(x, y) ASH__DO__CONCAT(x, y)
#define ASH__DO__CONCAT(x, y) x ## y

#define ASH_REGISTER_CLASS(NAME, ...) \
namespace { \
/*::ash::registry::EncoderRegisterer<__VA_ARGS__> ASH_CONCAT(ash_encoder_registerer, __COUNTER__)(NAME);*/ \
}

}  // namespace registry

}  // namespace ash

#endif /* ASH_REGISTRATION_H_ */
