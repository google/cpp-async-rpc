#ifndef ASH_INTERFACES_H_
#define ASH_INTERFACES_H_

namespace ash {

// Namespace defining metadata objects.
namespace interfaces {

// A MethodDescriptor type specifies one remotely callable interface method.
template<typename MPtr, MPtr m_ptr>
struct MethodDescriptor;

template<typename C, typename Result, typename ... Args, Result (C::*m_ptr)(
		const Args& ...)>
struct MethodDescriptor<Result (C::*)(const Args& ...), m_ptr> {
	using class_type = C;
	using result_type = Result;
	using method_type_ptr = Result (C::*)(const Args& ...);
	using method_type = Result(const Args&& ...);
	static constexpr Result (C::*method)(const Args& ...) = m_ptr;
};

// A ClassDescriptor type specifies one interface class in terms of its
// components, that can be themselves InterfaceDescriptor types for extended
// base interfaces or individual MethodDescriptor types for methods.
template<typename C, typename ... D>
struct InterfaceDescriptor;

template<typename C, typename D1, typename ... DN>
struct InterfaceDescriptor<C, D1, DN ...> {
	using interface_type = C;
};

template<typename C>
struct InterfaceDescriptor<C> {
	using interface_type = C;
};

#define ASH_EXPORT(NAME) ::ash::interfaces::MethodDescriptor<decltype(&self_interface::NAME), &self_interface::NAME>
#define ASH_EXTEND(BASE) BASE::interface_descriptor
#define ASH_INTERFACE(CLS, ...) \
	using self_interface = CLS; \
	using interface_descriptor = ::ash::interfaces::InterfaceDescriptor<CLS, __VA_ARGS__>

}  // namespace interfaces

// Empty request/response type.
struct Empty {
	using self_interface = Empty;
	using interface_descriptor = ::ash::interfaces::InterfaceDescriptor<Empty>;
};

}  // namespace ash

#endif /* ASH_INTERFACES_H_ */
