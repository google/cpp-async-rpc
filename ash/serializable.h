#ifndef ASH_SERIALIZABLE_H_
#define ASH_SERIALIZABLE_H_

#include <cassert>

#include "ash/config.h"
#include "ash/dynamic_base_class.h"
#include "ash/preprocessor.h"
#include "ash/mpt.h"
#include "ash/registry.h"

namespace ash {

namespace detail {
struct dynamic_class_filter {
	template<typename T>
	constexpr bool operator()(mpt::wrap_type<T>) {
		return is_dynamic<T>::value;
	}
};

template<bool dynamic, typename OwnType, typename ... Bases> class serializable_mixin;

template<typename OwnType, typename ... Bases>
struct serializable_mixin<false, OwnType, Bases...> : Bases... {
	using own_type = OwnType;
	using base_classes = mpt::pack<Bases...>;
	using dynamic_base_classes = decltype(mpt::filter_if(base_classes {}, dynamic_class_filter {}));

	// Don't inherit load/save behavior from the parent, and provide an empty set of fields.
	template<typename S>
	void save(S& s) const = delete;
	template<typename S>
	void load(S& s) = delete;
	using field_descriptors = mpt::pack<>;
};

template<typename OwnType, typename ...Bases>
struct serializable_mixin<true, OwnType, Bases...> : serializable_mixin<false,
		OwnType, Bases...> {
private:
	// Implement the virtual function that returns the name of our type.
	const char *portable_class_name_internal() const override {
		using Descriptor = ash::detail::dynamic_class_descriptor<OwnType>;
		assert(Descriptor::class_name != nullptr);
		return Descriptor::class_name;
	}
};

/// A field_descriptor type specifies how to access one data member.
template<typename MPtr, MPtr m_ptr>
struct field_descriptor;

/// A field_descriptor type specifies how to access one data member.
template<typename C, typename T, T C::*m_ptr>
struct field_descriptor<T C::*, m_ptr> {
	using class_type = C;
	using member_type = T;
	static constexpr auto member_pointer = m_ptr;
};
}  // namespace detail

/// Inherit publicly from this in serializable classes, specifying own type and public bases.
template<typename OwnType, typename ...Bases>
using serializable = detail::serializable_mixin<
(mpt::count_if(mpt::pack<Bases...> {}, detail::dynamic_class_filter {}) > 0),
OwnType, Bases...>;

/// Inherit publicly from this in dynamic classes, specifying own type and public bases.
template<typename OwnType, typename ...Bases>
using dynamic = mpt::conditional_t<(mpt::count_if(mpt::pack<Bases...> {}, detail::dynamic_class_filter {}) > 0), serializable<OwnType, Bases...>, serializable<OwnType, ::ash::dynamic_base_class, Bases...>>;

/// Define a `field_descriptor` type for a member field named `NAME`.
#define ASH_FIELD(NAME) ::ash::detail::field_descriptor<decltype(&own_type::NAME), &own_type::NAME>
#define ASH_FIELD_SEP() ,

/// Needed to find our own type in template classes, as the base class is dependent.
#define ASH_OWN_TYPE(...) using own_type = __VA_ARGS__

/// Define the list of `field_descriptor` elements for the current class.
#define ASH_FIELDS(...) \
	using field_descriptors = ::ash::mpt::pack<ASH_FOREACH(ASH_FIELD, ASH_FIELD_SEP, __VA_ARGS__)>

/// Register a dynamic class into the class factories.
#define ASH_REGISTER(...) \
template <> const char* ::ash::detail::dynamic_class_descriptor< __VA_ARGS__ >::class_name = ::ash::registry::dynamic_object_factory::get().register_class< __VA_ARGS__ , ::ash::config::all_encoders, ::ash::config::all_decoders>(#__VA_ARGS__)

/// Register a dynamic class into the class factories under a custom name.
#define ASH_REGISTER_WITH_NAME(NAME, ...) \
template <> const char* ::ash::detail::dynamic_class_descriptor< __VA_ARGS__ >::class_name = ::ash::registry::dynamic_object_factory::get().register_class< __VA_ARGS__ , ::ash::config::all_encoders, ::ash::config::all_decoders>(NAME)

}  // namespace ash

#endif /* ASH_SERIALIZABLE_H_ */
