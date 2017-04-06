#ifndef ASH_REGISTRY_H_
#define ASH_REGISTRY_H_

#include <ash/dynamic_base_class.h>
#include <cassert>
#include <cstring>
#include <map>
#include <set>

#include "ash/singleton.h"

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
	std::set<const char*, detail::const_char_ptr_compare> dynamic_subclass_set_;
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

class dynamic_object_factory: public singleton<dynamic_object_factory> {
public:
	template<typename T>
	const char* register_class(const char* class_name) {
		factory_function_type f =
				[]() {return static_cast<::ash::dynamic_base_class*>(new T());};
		assert(factory_function_map_.emplace(class_name, f).second);
		dynamic_subclass_registry<T>::get().register_subclass(class_name);
		return class_name;
	}

	// Returns nullptr in case of errors.
	template<typename T>
	T* create(const char* class_name) const {
		if (!dynamic_subclass_registry<T>::get().is_subclass(class_name))
			return nullptr;
		const auto it = factory_function_map_.find(class_name);
		if (it == factory_function_map_.end())
			return nullptr;
		return static_cast<T*>(it->second());
	}

private:
	using factory_function_type = ::ash::dynamic_base_class* (*)();
	std::map<const char*, factory_function_type, detail::const_char_ptr_compare> factory_function_map_;
};

}  // namespace registry

}  // namespace ash

#endif /* ASH_REGISTRY_H_ */
