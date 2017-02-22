#ifndef ASH_METADATA_H_
#define ASH_METADATA_H_

#include "ash/mpt.h"

namespace ash {

/// Serialization metadata helpers.
namespace metadata {

/// A FieldDescriptor type specifies how to load/save one data member.
template<typename MPtr, MPtr m_ptr>
struct FieldDescriptor;
/// A FieldDescriptor type specifies how to load/save one data member.
template<typename C, typename T, T C::*m_ptr>
struct FieldDescriptor<T C::*, m_ptr> {
	using class_type = C;
	using member_type = T;

	template<typename S>
	static void load(S&s, C& o) {
		s(o.*m_ptr);
	}

	template<typename S>
	static void save(S&s, const C& o) {
		s(o.*m_ptr);
	}
};
/// A `FieldElementLoader` knows how to load fields.
struct FieldElementLoader {
	template <typename D, typename S, typename C>
	void operator()(mpt::wrap_type<D>, S& s, C& o) {
		D::load(s, o);
	}
};
/// A `FieldElementLoader` knows how to save fields.
struct FieldElementSaver {
	template <typename D, typename S, typename C>
	void operator()(mpt::wrap_type<D>, S& s, const C& o) {
		D::save(s, o);
	}
};

/// \brief Descriptor type for a serializable class.
/// A `ClassDescriptor` type specifies how to load/save one class in terms of its
/// components, that can be themselves ClassDescriptor types for base classes
/// or individual `FieldDescriptor` types for data members.
template<typename C, typename ...D>
struct ClassDescriptor {
	using class_type = C;

	template<typename S>
	static void load(S&s, C& o) {
		mpt::for_each(mpt::pack<D...>{}, FieldElementLoader(), s, o);
	}

	template<typename S>
	static void save(S&s, const C& o) {
		mpt::for_each(mpt::pack<D...>{}, FieldElementSaver(), s, o);
	}
};

/// Define a `FieldDescriptor` type for a member field named `NAME`.
#define ASH_FIELD(NAME) ::ash::metadata::FieldDescriptor<decltype(&self_class::NAME), &self_class::NAME>
/// Declare `BASE` as one of the base classes for the current class.
#define ASH_BASE(BASE) BASE::class_descriptor
/// Define a `ClassDescriptor` for the current class.
#define ASH_CLASS(CLS, ...) \
	using self_class = CLS; \
	using class_descriptor = ::ash::metadata::ClassDescriptor<CLS, __VA_ARGS__>

}  // namespace metadata

}  // namespace ash

#endif /* ASH_METADATA_H_ */
