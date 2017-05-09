#ifndef ASH_REGISTRY_H_
#define ASH_REGISTRY_H_

#include <cassert>
#include <cstring>
#include <map>
#include <memory>
#include <set>

#include "ash/dynamic_base_class.h"
#include "ash/singleton.h"
#include "ash/status.h"
#include "ash/vector_assoc.h"

/*
 * unique, non-polymorphic
 *  save:
 *   1. save null?
 *   2. if !null
 *    2.1. static save
 *  load:
 *   1. load null?
 *   2. if !null
 *    2.1. static new
 *    2.2. set pointer
 *    2.3. static load
 *
 * shared, non-polymorphic
 *  save:
 *   1. get sequence/register, saved?
 *   2. save sequence
 *   3. if !saved
 *    3.1. static save
 *  load:
 *   1. read sequence, loaded?
 *   2. if !loaded
 *    2.1. static new
 *    2.2. set pointer
 *    2.3. register
 *    2.4. static load
 *   3. if loaded
 *    3.1. get registered
 *    3.2. set pointer
 *
 * unique, polymorphic
 *  save:
 *   1. save null?
 *   2. if !null
 *    2.1. save class_name
 *    2.2. dynamic save
 *  load:
 *   1. load null?
 *   2. if !null
 *    2.1. load class_name
 *    2.2. dynamic new (-> base)
 *    2.3. set pointer (-> base)
 *    2.4. dynamic load
 *
 * shared, polymorphic
 *  save:
 *   1. get sequence/register (-> all bases), saved?
 *   2. save sequence
 *   2. if !saved
 *    2.1. save class_name
 *    2.2. dynamic save
 *  load:
 *   1. read sequence, loaded?
 *   2. if !loaded
 *    2.1. load class_name
 *    2.2. dynamic new (-> base)
 *    2.3. set pointer (-> base)
 *    2.4. register (-> all bases)
 *    2.5. dynamic load
 *   3. if loaded
 *    3.1. get registered (-> base)
 *    3.2. set pointer (-> base)
 */
namespace ash {

namespace registry {

namespace detail {
struct const_char_ptr_compare {
	bool operator()(const char* a, const char* b) const {
		return std::strcmp(a, b) < 0;
	}
};
}  // namespace detail

template<typename T>
class dynamic_subclass_registry: public singleton<dynamic_subclass_registry<T>> {
public:
	void register_subclass(const char* class_name);
	bool is_subclass(const char* class_name) {
		return dynamic_subclass_set_.count(class_name) == 1;
	}
private:
	ash::vector_set<const char*, detail::const_char_ptr_compare> dynamic_subclass_set_;
};

namespace detail {
struct register_subclass {
	template<typename T>
	void operator()(mpt::wrap_type<T>, const char* class_name) {
		dynamic_subclass_registry<T>::get().register_subclass(class_name);
	}
};
}  // namespace detail

template<typename T>
void dynamic_subclass_registry<T>::register_subclass(const char* class_name) {
	dynamic_subclass_set_.insert(class_name);
	mpt::for_each(typename T::dynamic_base_classes { },
			detail::register_subclass { }, class_name);
}

template<typename S>
class dynamic_encoder_registry: public singleton<dynamic_encoder_registry<S>> {
public:
	template<typename T>
	void register_class(const char* class_name) {
		encoder_function_type f = [](S& s, const ::ash::dynamic_base_class& o) {
			return s(static_cast<const T&>(o));
		};

		assert(encoder_function_map_.emplace(class_name, (f)).second);
	}

	status encode(S& s, const ::ash::dynamic_base_class& o) {
		const auto it = encoder_function_map_.find(o.portable_class_name());
		if (it == encoder_function_map_.end())
			return status::NOT_FOUND;
		return it->second(s, o);
	}

private:
	using encoder_function_type = status (*)(S&, const ::ash::dynamic_base_class&);
	ash::vector_map<const char*, encoder_function_type,
			detail::const_char_ptr_compare> encoder_function_map_;
};

namespace detail {
template<typename T>
struct register_encoder {
	template<typename S>
	void operator()(mpt::wrap_type<S>, const char* class_name) {
		dynamic_encoder_registry<S>::get().template register_class<T>(
				class_name);
	}
};
}  // namespace detail

template<typename S>
class dynamic_decoder_registry: public singleton<dynamic_decoder_registry<S>> {
public:
	template<typename T>
	void register_class(const char* class_name) {
		decoder_function_type f = [](S& s, ::ash::dynamic_base_class& o) {
			return s(static_cast<T&>(o));
		};

		assert(decoder_function_map_.emplace(class_name, (f)).second);
	}

	status decode(S& s, ::ash::dynamic_base_class& o) {
		const auto it = decoder_function_map_.find(o.portable_class_name());
		if (it == decoder_function_map_.end())
			return status::NOT_FOUND;
		return it->second(s, o);
	}

private:
	using decoder_function_type = status (*)(S&, ::ash::dynamic_base_class&);
	ash::vector_map<const char*, decoder_function_type,
			detail::const_char_ptr_compare> decoder_function_map_;
};

namespace detail {
template<typename T>
struct register_decoder {
	template<typename S>
	void operator()(mpt::wrap_type<S>, const char* class_name) {
		dynamic_decoder_registry<S>::get().template register_class<T>(
				class_name);
	}
};
}  // namespace detail

class dynamic_object_factory: public singleton<dynamic_object_factory> {
public:
	template<typename T, typename Encoders, typename Decoders>
	const char* register_class(const char* class_name) {
		static_assert(is_dynamic<T>::value, "Only classes inheriting from ash::dynamic_base_class can be registered for polymorphism");

		// Register the class into this factory for object creation.
		factory_function_type f = []() {
			return std::unique_ptr<::ash::dynamic_base_class>(new T());
		};
		assert(factory_function_map_.emplace(class_name, (f)).second);

		// Register the class into the class hierarchy.
		dynamic_subclass_registry<T>::get().register_subclass(class_name);

		// Register the encoders.
		mpt::for_each(Encoders { }, detail::register_encoder<T> { },
				class_name);

		// Register the decoders.
		mpt::for_each(Decoders { }, detail::register_decoder<T> { },
				class_name);

		return class_name;
	}

	template<typename T>
	status_or<std::unique_ptr<T> > create(const char* class_name) const {
		if (!dynamic_subclass_registry<T>::get().is_subclass(class_name))
			return status::INVALID_ARGUMENT;
		const auto it = factory_function_map_.find(class_name);
		if (it == factory_function_map_.end())
			return status::NOT_FOUND;
		std::unique_ptr<::ash::dynamic_base_class> ptr(it->second());
		return std::unique_ptr < T > (static_cast<T*>(ptr.release()));
	}

	// Non-polymorphic save.
	template<typename S, typename T>
	typename std::enable_if<!is_dynamic<T>::value, status>::type save(S& s,
			const T& o) {
		return s(o);
	}

	// Polymorphic save.
	template<typename S, typename T>
	typename std::enable_if<is_dynamic<T>::value, status>::type save(S& s,
			const T& o) {
		s(o.portable_class_name());
		return dynamic_encoder_registry<S>::get().encode(s, o);
	}

	// Non-polymorphic load.
	template<typename S, typename T>
	typename std::enable_if<!is_dynamic<T>::value,
			status_or<std::unique_ptr<T> > >::type load(S& s) {
		std::unique_ptr < T > o(new T());
		ASH_RETURN_IF_ERROR(s(*o));
		return o;
	}

	// Polymorphic load.
	template<typename S, typename T>
	typename std::enable_if<is_dynamic<T>::value, status_or<std::unique_ptr<T> > >::type load(
			S& s) {
		std::string class_name;
		ASH_RETURN_IF_ERROR(s(class_name));
		std::unique_ptr<T> o;
		ASH_ASSIGN_OR_RETURN(o, create<T>(class_name.c_str()));
		ASH_RETURN_IF_ERROR(dynamic_decoder_registry<S>::get().decode(s, *o));
		return o;
	}

private:
	using factory_function_type = std::unique_ptr<::ash::dynamic_base_class> (*)();
	ash::vector_map<const char*, factory_function_type,
			detail::const_char_ptr_compare> factory_function_map_;
};

}  // namespace registry

}  // namespace ash

#endif /* ASH_REGISTRY_H_ */
