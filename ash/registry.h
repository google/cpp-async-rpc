#ifndef ASH_REGISTRY_H_
#define ASH_REGISTRY_H_

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <typeinfo>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include <iostream>

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

/*
class ObjectRegistry {
public:
	using id_type = uint32_t;
	static constexpr id_type already_seen_marker = (static_cast<id_type>(1)) << 31;

private:
	class EntryBase {
	public:
		virtual ~EntryBase() {
		}
	};

	template<typename B>
	class Entry: public EntryBase {
	public:
		Entry(std::shared_ptr<B> ptr) :
				ptr_(ptr) {
		}

		std::shared_ptr<B> get() {
			return ptr_;
		}
	private:
		std::shared_ptr<B> ptr_;
	};

	std::unordered_map<std::pair<std::type_index, id_type>,
			std::unique_ptr<EntryBase>> objects_;

public:
	template<typename B>
	void registerObject(id_type id, std::shared_ptr<B> ptr) {
		objects_[std::make_pair(std::type_index(typeid(T)), id)].reset(
				new Entry<B>(ptr));
	}

	template<typename B>
	std::shared_ptr<B> getObject(id_type id, std::shared_ptr<B> ptr) const {
		auto it = objects_.find(std::make_pair(std::type_index(typeid(T)), id));
		if (it == objects_.end()) {
			throw std::runtime_error("Unknown object.");
		}
		return std::static_pointer_cast<const Entry<B>>(it->second)->get();
	}
};
*/

template<typename S>
class EncoderClassRegistry {
private:
	class EntryBase {
	public:
		// Get the portable name of the class.
		virtual const char* class_name() const = 0;

		// Save an object given a const void pointer to the most-derived class.
		virtual void save(S& s, const void* o) const = 0;
	};

	template<typename T>
	class Entry: public EntryBase {
	public:
		Entry(const char* name) :
				name_(name) {
			std::cerr << __PRETTY_FUNCTION__ << std::endl;
		}

		// Get the portable name of the class.
		const char* class_name() const override {
			return name_;
		}

		// Save an object given a reference to the base class.
		void save(S& s, const void* o) const override {
			const T* ptr = static_cast<const T*>(o);
			s(*ptr);
		}

	private:
		const char* name_;
	};

	std::unordered_map<std::type_index, std::unique_ptr<EntryBase>> classes_;

	const EntryBase& entry(const std::type_index& type) {
		auto it = classes_.find(type);
		if (it == classes_.end()) {
			throw std::runtime_error("Unknown type.");
		}
		return *(it->second);
	}

public:
	template<typename T>
	void registerClass(const char* name) {
		classes_[std::type_index(typeid(T))].reset(new Entry<T>(name));
	}

	template<typename T>
	const char* class_name(const T& o) const {
		return entry(std::type_index(typeid(o))).name();
	}

	template<typename T>
	void save(S& s, const T& o) const {
		return entry(std::type_index(typeid(o))).save(s,
				dynamic_cast<const void*>(&o));
	}
};

template<typename S>
class DecoderClassRegistry {
private:
	class EntryBase {
	public:
		//virtual void construct_register_and_load(S& s, ObjectRegistry& object_registry, const char* class_name, ObjectRegistry::id_type id) const = 0;
		virtual void construct_and_load(S& s, const char* class_name) const = 0;
	};

	template<typename T>
	class Entry: public EntryBase {
	public:
		//void construct_register_and_load(S& s, ObjectRegistry& object_registry, const char* class_name, ObjectRegistry::id_type id) const override {}
		void construct_and_load(S& s, const char* class_name) const override {}
	};

	std::unordered_map<std::type_index, std::unique_ptr<EntryBase>> classes_;

	const EntryBase& entry(const std::type_index& type) {
		auto it = classes_.find(type);
		if (it == classes_.end()) {
			throw std::runtime_error("Unknown type.");
		}
		return *(it->second);
	}

public:
	template<typename T>
	void registerClass(const char* name) {
		classes_[std::type_index(typeid(T))].reset(new Entry<T>(name));
	}

	template<typename T>
	const char* name(const T& o) const {
		return entry(std::type_index(typeid(o))).name();
	}

	template<typename T>
	void save(S& s, const T& o) const {
		return entry(std::type_index(typeid(o))).save(s,
				dynamic_cast<const void*>(&o));
	}
};

}  // namespace registry

}  // namespace ash

#endif /* ASH_REGISTRY_H_ */
