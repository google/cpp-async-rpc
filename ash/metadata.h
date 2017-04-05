#ifndef ASH_METADATA_H_
#define ASH_METADATA_H_

#include <type_traits>
#include <cassert>

#include "ash/dynamic_class.h"
#include "ash/mpt.h"
#include "ash/traits.h"

namespace ash {

/// Serialization metadata helpers.
namespace metadata {

struct IsDynamicClass {
	template<typename T>
	constexpr bool operator()(mpt::wrap_type<T>) {
		return std::is_base_of<::ash::DynamicClass, T>::value;
	}
};

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
	constexpr auto operator()(A previous,
			mpt::wrap_type<PublicVirtual<B>> current)
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
	constexpr auto operator()(A previous,
			mpt::wrap_type<ProtectedVirtual<B>> current)
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
	constexpr auto operator()(A previous,
			mpt::wrap_type<PrivateVirtual<B>> current)
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
	using public_bases = decltype(mpt::accumulate(mpt::pack<> {}, mpt::pack<Bases...> {}, FilterPublic {}));
	using public_virtual_bases = decltype(mpt::accumulate(mpt::pack<> {}, mpt::pack<Bases...> {}, FilterPublicVirtual {}));
	using protected_bases = decltype(mpt::accumulate(mpt::pack<> {}, mpt::pack<Bases...> {}, FilterProtected {}));
	using protected_virtual_bases = decltype(mpt::accumulate(mpt::pack<> {}, mpt::pack<Bases...> {}, FilterProtectedVirtual {}));
	using private_bases = decltype(mpt::accumulate(mpt::pack<> {}, mpt::pack<Bases...> {}, FilterPrivate {}));
	using private_virtual_bases = decltype(mpt::accumulate(mpt::pack<> {}, mpt::pack<Bases...> {}, FilterPrivateVirtual {}));
	using all_bases = decltype(mpt::cat(
					public_bases {}, public_virtual_bases {}, protected_bases {}, protected_virtual_bases {},
					private_bases {}, private_virtual_bases {}));
	constexpr static bool is_dynamic = (mpt::count_if(public_bases { },
			IsDynamicClass { }) > 0);
};

template<bool Dynamic, typename PublicBases, typename PublicVirtualBases,
		typename ProtectedBases, typename ProtectedVirtualBases,
		typename PrivateBases, typename PrivateVirtualBases> class MetadataBase;
template<typename ...PublicBases, typename ...PublicVirtualBases,
		typename ...ProtectedBases, typename ...ProtectedVirtualBases,
		typename ...PrivateBases, typename ...PrivateVirtualBases>
class MetadataBase<false, mpt::pack<PublicBases...>,
		mpt::pack<PublicVirtualBases...>, mpt::pack<ProtectedBases...>,
		mpt::pack<ProtectedVirtualBases...>, mpt::pack<PrivateBases...>,
		mpt::pack<PrivateVirtualBases...>> : public PublicBases...,
		public virtual PublicVirtualBases...,
		protected ProtectedBases...,
		protected virtual ProtectedVirtualBases...,
		private PrivateBases...,
		private virtual PrivateVirtualBases... {
public:
	using public_base_classes = mpt::pack<PublicBases...>;
	using public_virtual_base_classes = mpt::pack<PublicVirtualBases...>;
	using protected_base_classes = mpt::pack<ProtectedBases...>;
	using protected_virtual_base_classes = mpt::pack<ProtectedVirtualBases...>;
	using private_base_classes = mpt::pack<PrivateBases...>;
	using private_virtual_base_classes = mpt::pack<PrivateVirtualBases...>;
	using base_classes = decltype(mpt::cat(
					public_base_classes {}, public_virtual_base_classes {},
					protected_base_classes {}, protected_virtual_base_classes {},
					private_base_classes {}, private_virtual_base_classes {}));
	using dynamic_base_classes = decltype(mpt::filter_if(public_base_classes {}, IsDynamicClass {}));
	// Don't inherit load/save behavior from the parent, and provide an empty set of fields.
	template<typename S>
	void save(S& s) const = delete;
	template<typename S>
	void load(S& s) = delete;
	using field_descriptors = mpt::pack<>;
};
template<typename ...PublicBases, typename ...PublicVirtualBases,
		typename ...ProtectedBases, typename ...ProtectedVirtualBases,
		typename ...PrivateBases, typename ...PrivateVirtualBases>
class MetadataBase<true, mpt::pack<PublicBases...>,
		mpt::pack<PublicVirtualBases...>, mpt::pack<ProtectedBases...>,
		mpt::pack<ProtectedVirtualBases...>, mpt::pack<PrivateBases...>,
		mpt::pack<PrivateVirtualBases...>> : public MetadataBase<false,
		mpt::pack<PublicBases...>, mpt::pack<PublicVirtualBases...>,
		mpt::pack<ProtectedBases...>, mpt::pack<ProtectedVirtualBases...>,
		mpt::pack<PrivateBases...>, mpt::pack<PrivateVirtualBases...>> {
private:
	// Ensure the class becomes abstract if we don't include ASH_DYNAMIC_{CLASS,STRUCT} in it.
	const char *portableClassName() const override = 0;
};

template<typename ...Bases>
using MetadataBaseGen = MetadataBase<
BaseClassHelper<Bases...>::is_dynamic,
typename BaseClassHelper<Bases...>::public_bases,
typename BaseClassHelper<Bases...>::public_virtual_bases,
typename BaseClassHelper<Bases...>::protected_bases,
typename BaseClassHelper<Bases...>::protected_virtual_bases,
typename BaseClassHelper<Bases...>::private_bases,
typename BaseClassHelper<Bases...>::private_virtual_bases>;

#define ASH_CLASS(CLASS_NAME, ...) class CLASS_NAME : public ::ash::metadata::MetadataBaseGen<__VA_ARGS__ >
#define ASH_STRUCT(STRUCT_NAME, ...) struct STRUCT_NAME : public ::ash::metadata::MetadataBaseGen<__VA_ARGS__ >
#define ASH_PUBLIC(...) ::ash::metadata::Public( __VA_ARGS__ )
#define ASH_PROTECTED(...) ::ash::metadata::Protected( __VA_ARGS__ )
#define ASH_PRIVATE(...) ::ash::metadata::Private( __VA_ARGS__ )
#define ASH_DYNAMIC_IMPL \
private: \
	const char* portableClassName() const override { \
		using Descriptor = ash::metadata::detail::DynamicClassDescriptor<std::remove_cv<std::remove_reference<decltype(*this)>::type>::type>; \
		assert(Descriptor::class_name != nullptr); \
		return Descriptor::class_name; \
    }
#define ASH_DYNAMIC_STRUCT \
	ASH_DYNAMIC_IMPL \
	public:
#define ASH_DYNAMIC_CLASS \
	ASH_DYNAMIC_IMPL \
	private:

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

namespace detail {
template<typename> struct RemoveParens;
template<typename T> struct RemoveParens<void (T)> {
    typedef T type;
};
}  // namespace detail

#define ASH_REMOVE_PARENS(...) typename ::ash::metadata::detail::RemoveParens<void( __VA_ARGS__ )>::type;

/// Define a `FieldDescriptor` type for a member field named `NAME`.
#define ASH_FIELD(NAME) ::ash::metadata::FieldDescriptor<decltype(&own_type::NAME), &own_type::NAME>
/// Define the list of `FieldDescriptor` elements for the current class.
#define ASH_FIELDS(CLASS_TYPE, ...) \
	using own_type = ASH_REMOVE_PARENS(CLASS_TYPE); \
	using field_descriptors = ash::mpt::pack<__VA_ARGS__>

#define ASH_REGISTER(...) \
template <> \
const char* ::ash::metadata::detail::DynamicClassDescriptor< __VA_ARGS__ >::class_name = #__VA_ARGS__;

#define ASH_REGISTER_WITH_NAME(NAME, ...) \
template <> \
const char* ::ash::metadata::detail::DynamicClassDescriptor< __VA_ARGS__ >::class_name = NAME;

}  // namespace metadata

}  // namespace ash

#endif /* ASH_METADATA_H_ */
