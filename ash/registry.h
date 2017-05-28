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
	using encoder_function_type = status (*)(S&, const ::ash::dynamic_base_class&);

	template<typename T>
	void register_class(const char* class_name) {
		encoder_function_type f = [](S& s, const ::ash::dynamic_base_class& o) {
			return s(static_cast<const T&>(o));
		};

		assert(encoder_function_map_.emplace(class_name, (f)).second);
	}

	status_or<encoder_function_type> operator[](const char* class_name) const {
		const auto it = encoder_function_map_.find(class_name);
		if (it == encoder_function_map_.end())
			return status::NOT_FOUND;
		return it->second;
	}

private:
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
	using decoder_function_type = status (*)(S&, ::ash::dynamic_base_class&);

	template<typename T>
	void register_class(const char* class_name) {
		decoder_function_type f = [](S& s, ::ash::dynamic_base_class& o) {
			return s(static_cast<T&>(o));
		};

		assert(decoder_function_map_.emplace(class_name, (f)).second);
	}

	status_or<decoder_function_type> operator[](const char* class_name) const {
		const auto it = decoder_function_map_.find(class_name);
		if (it == decoder_function_map_.end())
			return status::NOT_FOUND;
		return it->second;
	}

private:
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
	using factory_function_type = std::unique_ptr<::ash::dynamic_base_class> (*)();

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

	status_or<factory_function_type> operator[](const char* class_name) const {
		const auto it = factory_function_map_.find(class_name);
		if (it == factory_function_map_.end())
			return status::NOT_FOUND;
		return it->second;
	}

private:
	ash::vector_map<const char*, factory_function_type,
			detail::const_char_ptr_compare> factory_function_map_;
};

template<typename S>
class encoder_class_dictionary {
	struct info {
		std::size_t class_id;
		typename dynamic_encoder_registry<S>::encoder_function_type encoder_function;
	};

public:
	status encode(S& s, const ::ash::dynamic_base_class& o) {
		const char* class_name = o.portable_class_name();
		auto it = info_map_.find(class_name);
		bool needs_class_name = false;
		if (it == info_map_.end()) {
			// Not cached yet. Need to interrogate the dynamic_encoder_registry for this class.
			typename dynamic_encoder_registry<S>::encoder_function_type encoder_function;
			ASH_ASSIGN_OR_RETURN(encoder_function,
					dynamic_encoder_registry<S>::get()[class_name]);
			it = info_map_.emplace(class_name, info { next_id_++,
					encoder_function }).first;
			needs_class_name = true;
		}
		ASH_RETURN_IF_ERROR(s.write_variant(it->second.class_id));
		if (needs_class_name) {
			ASH_RETURN_IF_ERROR(s(class_name));
		}
		return it->second.encoder_function(s, o);
	}

private:
	std::size_t next_id_ = 0;
	ash::vector_map<const char*, info> info_map_;
};

template<typename S>
class decoder_class_dictionary {
	struct info {
		std::string class_name;
		dynamic_object_factory::factory_function_type factory_function;
		typename dynamic_decoder_registry<S>::decoder_function_type decoder_function;
	};

public:
	template<typename T>
	status_or<std::unique_ptr<T> > create_and_decode(S& s) {
		// Read the class numeric ID for this stream.
		std::size_t class_id;
		ASH_ASSIGN_OR_RETURN(class_id, s.read_variant());

		// Check for garbage (unsorted class IDs) in the input.
		if (class_id > info_vector_.size()) {
			// We didn't get either a known ID or the next one to assign.
			return status::INVALID_ARGUMENT;
		}

		// Construct the required info structure if needed.
		if (class_id == info_vector_.size()) {
			// Not seen yet. Need to read a class name and interrogate the dynamic_object_factory and dynamic_decoder_registry for this class.
			std::string class_name;
			ASH_RETURN_IF_ERROR(s(class_name));
			dynamic_object_factory::factory_function_type factory_function;
			typename dynamic_decoder_registry<S>::decoder_function_type decoder_function;
			ASH_ASSIGN_OR_RETURN(factory_function,
					dynamic_object_factory::get()[class_name.c_str()]);
			ASH_ASSIGN_OR_RETURN(decoder_function,
					dynamic_decoder_registry<S>::get()[class_name.c_str()]);
			info_vector_.push_back(info { class_name, factory_function,
					decoder_function });
		}

		// Info structure for the concrete class.
		const auto& info = info_vector_[class_id];

		// Check that the subclass is compatible (slow).
		if (!dynamic_subclass_registry<T>::get().is_subclass(
				info.class_name.c_str()))
			return status::INVALID_ARGUMENT;

		// Instantiate the object.
		std::unique_ptr<::ash::dynamic_base_class> ptr(info.factory_function());

		// Perform the decoding.
		ASH_RETURN_IF_ERROR(info.decoder_function(s, *ptr));

		// Return the result.
		return std::unique_ptr < T > (static_cast<T*>(ptr.release()));
	}

private:
	std::size_t next_id_ = 0;
	std::vector<info> info_vector_;
};

}  // namespace registry

}  // namespace ash

#endif /* ASH_REGISTRY_H_ */
