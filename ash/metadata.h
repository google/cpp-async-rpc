#ifndef ASH_METADATA_H_
#define ASH_METADATA_H_

#include "ash/mpt.h"
#include "ash/static_object.h"

#include <iostream>

namespace ash {

/// Serialization metadata helpers.
namespace metadata {


template <typename Y>
struct Private {
	using base_class = Y;
};
template <typename Y>
struct Protected {
	using base_class = Y;
};
template <typename Y>
struct Public {
	using base_class = Y;
};

struct FilterPublic {
	template<typename A, typename B>
	constexpr auto
	operator()(A previous, mpt::wrap_type<Public<B>> current)
	-> decltype(mpt::cat(previous, mpt::pack<B>{})) {
		return mpt::cat(previous, mpt::pack<B>{});
	}
	template<typename A>
	constexpr A
	operator()(A previous, ...) {
		return previous;
	}
};

struct FilterProtected {
	template<typename A, typename B>
	constexpr auto
	operator()(A previous, mpt::wrap_type<Protected<B>> current)
	-> decltype(mpt::cat(previous, mpt::pack<B>{})) {
		return mpt::cat(previous, mpt::pack<B>{});
	}
	template<typename A>
	constexpr A
	operator()(A previous, ...) {
		return previous;
	}
};

struct FilterPrivate {
	template<typename A, typename B>
	constexpr auto
	operator()(A previous, mpt::wrap_type<Private<B>> current)
	-> decltype(mpt::cat(previous, mpt::pack<B>{})) {
		return mpt::cat(previous, mpt::pack<B>{});
	}
	template<typename A>
	constexpr A
	operator()(A previous, ...) {
		return previous;
	}
};

template <typename ...Bases>
struct BaseClassHelper {
	using PublicBases = decltype(mpt::accumulate(mpt::pack<>{}, mpt::pack<Bases...>{}, FilterPublic{}));
	using ProtectedBases = decltype(mpt::accumulate(mpt::pack<>{}, mpt::pack<Bases...>{}, FilterProtected{}));
	using PrivateBases = decltype(mpt::accumulate(mpt::pack<>{}, mpt::pack<Bases...>{}, FilterPrivate{}));
};

template <typename T>
class Hello {
public:
	Hello() {
		std::cerr << "Hello!" << std::endl;
	}

	~Hello() {
		std::cerr << "Bye!" << std::endl;
	}
};

template<typename PublicBases, typename ProtectedBases, typename PrivateBases> class MetadataBase;
template<typename ...PublicBases, typename ...ProtectedBases, typename ...PrivateBases>
class MetadataBase<mpt::pack<PublicBases...>, mpt::pack<ProtectedBases...>, mpt::pack<PrivateBases...>>
	: public PublicBases..., protected ProtectedBases..., private PrivateBases... {
public:
	using public_base_classes = mpt::pack<PublicBases...>;
	using protected_base_classes = mpt::pack<ProtectedBases...>;
	using private_base_classes = mpt::pack<PrivateBases...>;
	using base_classes = decltype(mpt::cat(public_base_classes{}, protected_base_classes{}, private_base_classes{}));
};

template <typename ...Bases>
using MetadataBaseGen = MetadataBase<
		typename BaseClassHelper<Bases...>::PublicBases,
		typename BaseClassHelper<Bases...>::ProtectedBases,
		typename BaseClassHelper<Bases...>::PrivateBases>;

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
	using static_initialization = decltype(::ash::StaticObject<::ash::metadata::Hello<CLASS_TYPE>>::get()); \
	using own_type = CLASS_TYPE; \
	using field_descriptors = ash::mpt::pack<__VA_ARGS__>

}  // namespace metadata

}  // namespace ash

#endif /* ASH_METADATA_H_ */
