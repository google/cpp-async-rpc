#ifndef ASH_METADATA_H_
#define ASH_METADATA_H_

#include "ash/mpt.h"
#include "ash/traits.h"

namespace ash {

/// Serialization metadata helpers.
namespace metadata {

template<typename Y>
struct Private {
	using base_class = Y;
};
template<typename Y>
struct Protected {
	using base_class = Y;
};
template<typename Y>
struct Public {
	using base_class = Y;
};
template<typename Y>
struct PrivateVirtual {
	using base_class = Y;
};
template<typename Y>
struct ProtectedVirtual {
	using base_class = Y;
};
template<typename Y>
struct PublicVirtual {
	using base_class = Y;
};

struct FilterPublic {
	template<typename A, typename B>
	constexpr auto operator()(A previous, mpt::wrap_type<Public<B>> current)
	-> decltype(mpt::cat(previous, mpt::pack<B> {})) {
		return mpt::cat(previous, mpt::pack<B> { });
	}
	template<typename A>
	constexpr A operator()(A previous, ...) {
		return previous;
	}
};

struct FilterProtected {
	template<typename A, typename B>
	constexpr auto operator()(A previous, mpt::wrap_type<Protected<B>> current)
	-> decltype(mpt::cat(previous, mpt::pack<B> {})) {
		return mpt::cat(previous, mpt::pack<B> { });
	}
	template<typename A>
	constexpr A operator()(A previous, ...) {
		return previous;
	}
};

struct FilterPrivate {
	template<typename A, typename B>
	constexpr auto operator()(A previous, mpt::wrap_type<Private<B>> current)
	-> decltype(mpt::cat(previous, mpt::pack<B> {})) {
		return mpt::cat(previous, mpt::pack<B> { });
	}
	template<typename A>
	constexpr A operator()(A previous, ...) {
		return previous;
	}
};

struct FilterPublicVirtual {
	template<typename A, typename B>
	constexpr auto operator()(A previous, mpt::wrap_type<PublicVirtual<B>> current)
	-> decltype(mpt::cat(previous, mpt::pack<B> {})) {
		return mpt::cat(previous, mpt::pack<B> { });
	}
	template<typename A>
	constexpr A operator()(A previous, ...) {
		return previous;
	}
};

struct FilterProtectedVirtual {
	template<typename A, typename B>
	constexpr auto operator()(A previous, mpt::wrap_type<ProtectedVirtual<B>> current)
	-> decltype(mpt::cat(previous, mpt::pack<B> {})) {
		return mpt::cat(previous, mpt::pack<B> { });
	}
	template<typename A>
	constexpr A operator()(A previous, ...) {
		return previous;
	}
};

struct FilterPrivateVirtual {
	template<typename A, typename B>
	constexpr auto operator()(A previous, mpt::wrap_type<PrivateVirtual<B>> current)
	-> decltype(mpt::cat(previous, mpt::pack<B> {})) {
		return mpt::cat(previous, mpt::pack<B> { });
	}
	template<typename A>
	constexpr A operator()(A previous, ...) {
		return previous;
	}
};

template<typename ...Bases>
struct BaseClassHelper {
	using PublicBases = decltype(mpt::accumulate(mpt::pack<> {}, mpt::pack<Bases...> {}, FilterPublic {}));
	using PublicVirtualBases = decltype(mpt::accumulate(mpt::pack<> {}, mpt::pack<Bases...> {}, FilterPublicVirtual {}));
	using ProtectedBases = decltype(mpt::accumulate(mpt::pack<> {}, mpt::pack<Bases...> {}, FilterProtected {}));
	using ProtectedVirtualBases = decltype(mpt::accumulate(mpt::pack<> {}, mpt::pack<Bases...> {}, FilterProtectedVirtual {}));
	using PrivateBases = decltype(mpt::accumulate(mpt::pack<> {}, mpt::pack<Bases...> {}, FilterPrivate {}));
	using PrivateVirtualBases = decltype(mpt::accumulate(mpt::pack<> {}, mpt::pack<Bases...> {}, FilterPrivateVirtual {}));
};

template<typename PublicBases, typename PublicVirtualBases,
		 typename ProtectedBases, typename ProtectedVirtualBases,
		 typename PrivateBases, typename PrivateVirtualBases> class MetadataBase;
template<typename ...PublicBases, typename ...PublicVirtualBases,
         typename ...ProtectedBases, typename ...ProtectedVirtualBases,
		 typename ...PrivateBases, typename ...PrivateVirtualBases>
class MetadataBase<mpt::pack<PublicBases...>, mpt::pack<PublicVirtualBases...>,
                   mpt::pack<ProtectedBases...>, mpt::pack<ProtectedVirtualBases...>,
				   mpt::pack<PrivateBases...>, mpt::pack<PrivateVirtualBases...>> :
		public PublicBases..., public virtual PublicVirtualBases...,
		protected ProtectedBases..., protected virtual ProtectedVirtualBases...,
		private PrivateBases..., private virtual PrivateVirtualBases... {
public:
	using public_base_classes = mpt::pack<PublicBases...>;
	using public_virtual_base_classes = mpt::pack<PublicVirtualBases...>;
	using protected_base_classes = mpt::pack<ProtectedBases...>;
	using protected_virtual_base_classes = mpt::pack<ProtectedVirtualBases...>;
	using private_base_classes = mpt::pack<PrivateBases...>;
	using private_virtual_base_classes = mpt::pack<PrivateVirtualBases...>;
	using base_classes = decltype(mpt::cat(
			public_base_classes {}, public_virtual_base_classes{},
			protected_base_classes {}, protected_virtual_base_classes{},
			private_base_classes {}, private_virtual_base_classes{}));
	// Don't inherit load/save behavior from the parent, and provide an empty set of fields.
	template<typename S>
	void save(S& s) const {
	}
	template<typename S>
	void load(S& s) {
	}
	using field_descriptors = mpt::pack<>;
};

template<typename ...Bases>
using MetadataBaseGen = MetadataBase<
typename BaseClassHelper<Bases...>::PublicBases,
typename BaseClassHelper<Bases...>::PublicVirtualBases,
typename BaseClassHelper<Bases...>::ProtectedBases,
typename BaseClassHelper<Bases...>::ProtectedVirtualBases,
typename BaseClassHelper<Bases...>::PrivateBases,
typename BaseClassHelper<Bases...>::PrivateVirtualBases>;

#define ASH_CLASS(CLASS_NAME, ...) class CLASS_NAME : public ::ash::metadata::MetadataBaseGen<__VA_ARGS__ >
#define ASH_STRUCT(STRUCT_NAME, ...) struct STRUCT_NAME : public ::ash::metadata::MetadataBaseGen<__VA_ARGS__ >
#define ASH_PUBLIC(...) ::ash::metadata::Public( __VA_ARGS__ )
#define ASH_PROTECTED(...) ::ash::metadata::Protected( __VA_ARGS__ )
#define ASH_PRIVATE(...) ::ash::metadata::Private( __VA_ARGS__ )

/// A FieldDescriptor type specifies how to access one data member.
template<typename MPtr, MPtr m_ptr>
struct FieldDescriptor;
/// A FieldDescriptor type specifies how to access one data member.
template<typename C, typename T, T C::*m_ptr>
struct FieldDescriptor<T C::*, m_ptr> {
	using class_type = C;
	using member_type = T;
	static constexpr auto member_pointer = m_ptr;
};

/// Define a `FieldDescriptor` type for a member field named `NAME`.
#define ASH_FIELD(NAME) ::ash::metadata::FieldDescriptor<decltype(&own_type::NAME), &own_type::NAME>
/// Define the list of `FieldDescriptor` elements for the current class.
#define ASH_FIELDS(CLASS_TYPE, ...) \
	using own_type = CLASS_TYPE; \
	using field_descriptors = ash::mpt::pack<__VA_ARGS__>

namespace detail {
template<typename T, typename Enable = void>
struct dynamic_base_classes_helper;

template<typename T>
struct concat_dynamic_base_classes;
template<typename ...B>
struct concat_dynamic_base_classes<mpt::pack<B...>> {
	using types = decltype(mpt::cat(typename dynamic_base_classes_helper<B>::types{}...));
};

template<typename T, typename Enable>
struct dynamic_base_classes_helper {
	using types = mpt::conditional_t<std::is_polymorphic<T>::value, mpt::pack<T>, mpt::pack<>>;
};

struct IsDynamic {
	template<typename T>
	constexpr bool operator()(mpt::wrap_type<T>) {
		return std::is_polymorphic<T>::value;
	}
};

template<typename T>
struct dynamic_base_classes_helper<T,
		typename std::enable_if<
			std::is_polymorphic<T>::value
			&&
			(mpt::count_if(typename T::base_classes{}, IsDynamic{}) > 0)
		>::type> {
	using types = typename concat_dynamic_base_classes<typename T::base_classes>::types;
};
}  // namespace detail

template<typename T>
using DynamicBaseClasses = typename detail::dynamic_base_classes_helper<T>::types;

}  // namespace metadata

}  // namespace ash

#endif /* ASH_METADATA_H_ */
