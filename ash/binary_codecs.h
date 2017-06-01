#ifndef ASH_BINARY_CODECS_H_
#define ASH_BINARY_CODECS_H_

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "ash/traits.h"
#include "ash/mpt.h"
#include "ash/io_adapters.h"
#include "ash/registry.h"
#include "ash/status_or.h"
#include "ash/vector_assoc.h"

namespace ash {

// Binary encoder.
template<typename MyClass, typename Adapter, bool reverse_bytes>
class binary_encoder {
public:
	binary_encoder(Adapter out) :
			out_(out) {
	}

	// Special case for const char*, compatible with std::string de-serialization.
	status operator()(const char* s) {
		std::size_t l = std::strlen(s);
		ASH_RETURN_IF_ERROR(write_variant(l));
		return write_block(s, l);
	}

	// Serializable scalar.
	template<typename T>
	typename std::enable_if<traits::is_serializable_scalar<T>::value, status>::type operator()(
			const T& v) {
		return write_block(&v, 1);
	}

	// Contiguous sequences.
	template<typename T>
	typename std::enable_if<
			traits::is_iterable<T>::value
					&& traits::is_contiguous_sequence<T>::value, status>::type operator()(
			const T& sequence) {
		ASH_RETURN_IF_ERROR(maybe_write_size(sequence));
		return write_sequence(&(*sequence.cbegin()), sequence.size());
	}

	// Non-contiguous iterables.
	template<typename T>
	typename std::enable_if<
			traits::is_iterable<T>::value
					&& !traits::is_contiguous_sequence<T>::value, status>::type operator()(
			const T& sequence) {
		ASH_RETURN_IF_ERROR(maybe_write_size(sequence));
		for (const auto& i : sequence) {
			ASH_RETURN_IF_ERROR((*this)(i));
		}
		return status::OK;
	}

	// Pairs.
	template<typename U, typename V>
	status operator()(const std::pair<U, V>& p) {
		ASH_RETURN_IF_ERROR((*this)(p.first));
		return (*this)(p.second);
	}

	// Tuples.
	template<typename ... T>
	status operator()(const std::tuple<T ...>& t) {
		return apply_until_first_error(t, tuple_element_saver(), *this);
	}

	// Plain old arrays.
	template<typename T, std::size_t N>
	status operator()(const T (&a)[N]) {
		return write_sequence(&a[0], N);
	}

	// Saveable objects.
	template<typename T>
	typename std::enable_if<traits::can_be_saved<T, binary_encoder>::value,
			status>::type operator()(const T& o) {
		ASH_RETURN_IF_ERROR(save_base_classes(o));
		ASH_RETURN_IF_ERROR(save_fields(o));
		return invoke_save(o);
	}

	// Unique pointers.
	template<typename T, typename Deleter>
	status operator()(const std::unique_ptr<T, Deleter>& p) {
		bool present = (p != nullptr);
		ASH_RETURN_IF_ERROR((*this)(present));
		if (present) {
			ASH_RETURN_IF_ERROR(save_object_reference(*p));
		}
		return status::OK;
	}

	// Shared pointers.
	template<typename T>
	status operator()(const std::shared_ptr<T>& p) {
		// Short-circuit nullptr.
		if (!p) {
			return write_variant(0);
		}

		auto it = shared_object_map_.find(p.get());
		bool first_time = false;
		if (it == shared_object_map_.end()) {
			std::size_t next_object_id = shared_object_map_.size();
			it = shared_object_map_.emplace(p.get(), next_object_id).first;
			first_time = true;
		}
		ASH_RETURN_IF_ERROR(write_variant(it->second));
		if (first_time) {
			ASH_RETURN_IF_ERROR(save_object_reference(*p));
		}
		return status::OK;
	}

	// Weak pointers.
	template<typename T>
	status operator()(const std::weak_ptr<T>& p) {
		std::shared_ptr<T> shared = p.lock();
		return (*this)(shared);
	}

private:
	struct class_info {
		std::size_t class_id;
		typename registry::dynamic_encoder_registry<MyClass>::encoder_function_type encoder_function;
	};

	// Save potentially short integers in a compact form.
	//
	// Use the highest bit in each byte to determine whether more bytes come
	// after the current one. The final byte will have its high order bit
	// clear.
	//
	// For convenience, this is always represented in little endian format.
	status write_variant(std::size_t l) {
		do {
			uint8_t o = static_cast<uint8_t>(l & 0x7f);
			l >>= 7;
			if (l) {
				o |= 0x80;
			}
			ASH_RETURN_IF_ERROR((*this)(o));
		} while (l);
		return status::OK;
	}

	status save_dynamic_object_reference(const ::ash::dynamic_base_class& o) {
		const char* class_name = o.portable_class_name();
		auto it = class_info_map_.find(class_name);
		bool first_time = false;
		if (it == class_info_map_.end()) {
			// Not cached yet. Need to interrogate the dynamic_encoder_registry for this class.
			typename registry::dynamic_encoder_registry<MyClass>::encoder_function_type encoder_function;
			ASH_ASSIGN_OR_RETURN(encoder_function,
					registry::dynamic_encoder_registry<MyClass>::get()[class_name]);
			std::size_t next_class_id = class_info_map_.size();
			it = class_info_map_.emplace(class_name, class_info { next_class_id,
					encoder_function }).first;
			first_time = true;
		}
		ASH_RETURN_IF_ERROR(write_variant(it->second.class_id));
		if (first_time) {
			ASH_RETURN_IF_ERROR((*this)(class_name));
		}
		return it->second.encoder_function(static_cast<MyClass&>(*this), o);
	}

	template<typename T>
	typename std::enable_if<!is_dynamic<T>::value, status>::type save_object_reference(
			const T& o) {
		return (*this)(o);
	}

	template<typename T>
	typename std::enable_if<is_dynamic<T>::value, status>::type save_object_reference(
			const T& o) {
		return save_dynamic_object_reference(o);
	}

	struct tuple_element_saver {
		template<typename T, typename S>
		status operator()(const T& v, S& s) {
			return s(v);
		}
	};

	// Save base classes, if they exist.
	struct base_class_saver {
		template<typename T, typename B, typename S>
		status operator()(mpt::wrap_type<B>, const T& o, S& s) {
			const B& base = static_cast<const B&>(o);
			return s(base);
		}
	};

	template<typename T>
	typename std::enable_if<traits::has_base_classes<T>::value, status>::type save_base_classes(
			const T& o) {
		return apply_until_first_error(typename T::base_classes { },
				base_class_saver { }, o, *this);
	}

	template<typename T>
	typename std::enable_if<!traits::has_base_classes<T>::value, status>::type save_base_classes(
			const T& o) {
		return status::OK;
	}

	// Save fields from field_descriptors, if present.
	struct field_saver {
		template<typename T, typename FD, typename S>
		status operator()(mpt::wrap_type<FD>, const T& o, S& s) {
			return s(o.*(FD::member_pointer));
		}
	};

	template<typename T>
	typename std::enable_if<traits::has_field_descriptors<T>::value, status>::type save_fields(
			const T& o) {
		return apply_until_first_error(typename T::field_descriptors { },
				field_saver { }, o, *this);
	}

	template<typename T>
	typename std::enable_if<!traits::has_field_descriptors<T>::value, status>::type save_fields(
			const T& o) {
		return status::OK;
	}

	// Invoke the save method, if present.
	template<typename T>
	typename std::enable_if<traits::has_save<T, status(binary_encoder&)>::value,
			status>::type invoke_save(const T& o) {
		return o.save(*this);
	}

	template<typename T>
	typename std::enable_if<
			!traits::has_save<T, status(binary_encoder&)>::value, status>::type invoke_save(
			const T& o) {
		return status::OK;
	}

	// Write some container's size... or not (if it's known at compile time).
	template<typename T>
	typename std::enable_if<traits::has_static_size<T>::value, status>::type maybe_write_size(
			const T& t) {
		return status::OK;
	}

	template<typename T>
	typename std::enable_if<!traits::has_static_size<T>::value, status>::type maybe_write_size(
			const T& t) {
		return write_variant(t.size());
	}

	// For contiguous sequences of scalars, just treat them as a block of contiguous
	// memory.
	template<typename T>
	typename std::enable_if<traits::is_serializable_scalar<T>::value, status>::type write_sequence(
			const T*p, std::size_t l) {
		return write_block(p, l);
	}

	// For non-trivial objects, call the whole serialization machinery.
	template<typename T>
	typename std::enable_if<!traits::is_serializable_scalar<T>::value, status>::type write_sequence(
			const T*p, std::size_t l) {
		while (l-- > 0) {
			ASH_RETURN_IF_ERROR((*this)(*p++));
		}
		return status::OK;
	}

	// If we need to reverse the data, write it out byte by byte.
	template<typename T>
	typename std::enable_if<reverse_bytes && sizeof(T) != 1, status>::type write_block(
			const T* p_, std::size_t l) {
		const char *p = reinterpret_cast<const char*>(p_);
		constexpr std::size_t data_size = sizeof(T);
		for (std::size_t i = 0; i < l; i++, p += data_size) {
			for (std::size_t j = 1; j <= data_size; j++) {
				ASH_RETURN_IF_ERROR(out_.putc(p[data_size - j]));
			}
		}
		return status::OK;
	}

	// Just write out a whole block of memory if we don't need to reverse,
	// either because memory has the right endianness or the data type is byte-sized.
	template<typename T>
	typename std::enable_if<!reverse_bytes || sizeof(T) == 1, status>::type write_block(
			const T* p, std::size_t l) {
		return out_.write(reinterpret_cast<const char*>(p), l * sizeof(T));
	}

	// Here we depend on portable_class_name() returning identical pointers for speed.
	ash::vector_map<const char*, class_info> class_info_map_;
	ash::vector_map<void*, std::size_t> shared_object_map_ = { { nullptr, 0 } };

protected:
	Adapter out_;
};

// Binary decoder.
template<typename MyClass, typename Adapter, bool reverse_bytes>
class binary_decoder {
private:
	class update_pointer {
	public:
		update_pointer(std::shared_ptr<void>& ptr) :
				ptr_(ptr) {
		}
		template<typename T> void operator()(const std::shared_ptr<T>& ptr) {
			ptr_ = ptr;
		}
	private:
		std::shared_ptr<void>& ptr_;
	};

	class no_pointer_update {
	public:
		template<typename T> void operator()(const std::unique_ptr<T>& ptr) {
		}
	};

	struct shared_object_info {
		std::shared_ptr<void> ptr;
		registry::type_id type_id;
	};

public:
	binary_decoder(Adapter in) :
			in_(in) {
	}

	// Serializable scalar.
	template<typename T>
	typename std::enable_if<traits::is_serializable_scalar<T>::value, status>::type operator()(
			T& v) {
		return read_block(&v, 1);
	}

	// Contiguous sequences with fixed size (that is, std::array).
	template<typename T>
	typename std::enable_if<
			traits::is_iterable<T>::value
					&& traits::is_contiguous_sequence<T>::value
					&& traits::has_static_size<T>::value, status>::type operator()(
			T& sequence) {
		return read_sequence(&(*sequence.begin()), sequence.size());
	}

	// Contiguous sequences that can be resized, and contain just scalars.
	// These we don't mind resizing and default-initializing, so that we can
	// overwrite the values in a single block read. This should match strings
	// and vectors of scalars.
	template<typename T>
	typename std::enable_if<
			traits::is_iterable<T>::value && !traits::has_static_size<T>::value
					&& traits::is_contiguous_sequence<T>::value
					&& traits::has_storage_resizing<T>::value
					&& traits::is_serializable_scalar<typename T::value_type>::value
					&& !traits::is_associative<T>::value, status>::type operator()(
			T& sequence) {
		std::size_t l;
		ASH_ASSIGN_OR_RETURN(l, read_size());
		// No need to reserve as we are setting the size directly.
		sequence.resize(l);
		return read_sequence(&(*sequence.begin()), sequence.size());
	}

	// Containers where we need to read elements one by one and push_back them.
	// This should apply to deques, lists and vectors of non-scalars.
	template<typename T>
	typename std::enable_if<
			traits::is_iterable<T>::value && !traits::has_static_size<T>::value
					&& (!traits::is_contiguous_sequence<T>::value
							|| !traits::has_storage_resizing<T>::value
							|| !traits::is_serializable_scalar<
									typename T::value_type>::value)
					&& !traits::is_associative<T>::value, status>::type operator()(
			T& sequence) {
		std::size_t l;
		ASH_ASSIGN_OR_RETURN(l, read_size());
		sequence.clear();
		maybe_reserve(sequence, l);
		while (l-- > 0) {
			typename traits::deserializable_value_type<typename T::value_type>::type v;
			ASH_RETURN_IF_ERROR((*this)(v));
			sequence.push_back(std::move(v));
		}
		return status::OK;
	}

	// Associative containers.
	template<typename T>
	typename std::enable_if<
			traits::is_iterable<T>::value && !traits::has_static_size<T>::value
					&& traits::is_associative<T>::value, status>::type operator()(
			T& sequence) {
		std::size_t l;
		ASH_ASSIGN_OR_RETURN(l, read_size());
		sequence.clear();
		maybe_reserve(sequence, l);
		while (l-- > 0) {
			typename traits::deserializable_value_type<typename T::value_type>::type v;
			ASH_RETURN_IF_ERROR((*this)(v));
			sequence.insert(std::move(v));
		}
		return status::OK;
	}

	// Pairs.
	template<typename U, typename V>
	status operator()(std::pair<U, V>& p) {
		ASH_RETURN_IF_ERROR((*this)(p.first));
		return (*this)(p.second);
	}

	// Tuples.
	template<typename ... T>
	status operator()(std::tuple<T ...>& t) {
		return apply_until_first_error(t, tuple_element_loader(), *this);
	}

	// Plain old arrays.
	template<typename T, std::size_t N>
	status operator()(T (&a)[N]) {
		return read_sequence(&a[0], N);
	}

	// Loadable objects.
	template<typename T>
	typename std::enable_if<traits::can_be_loaded<T, binary_decoder>::value,
			status>::type operator()(T& o) {
		ASH_RETURN_IF_ERROR(load_base_classes(o));
		ASH_RETURN_IF_ERROR(load_fields(o));
		return invoke_load(o);
	}

	// Unique pointers.
	template<typename T, typename Deleter>
	status operator()(std::unique_ptr<T, Deleter>& p) {
		bool present;
		ASH_RETURN_IF_ERROR((*this)(present));
		if (present) {
			return load_object_reference(p, no_pointer_update());
		} else {
			p.reset();
			return status::OK;
		}
	}

	// Shared pointers.
	template<typename T>
	status operator()(std::shared_ptr<T>& p) {
		// Read the object numeric ID for this stream.
		std::size_t object_id;
		ASH_ASSIGN_OR_RETURN(object_id, read_variant());

		// Short-circuit nullptr.
		if (object_id == 0) {
			p.reset();
			return status::OK;
		}

		// Are we reading a new object?
		bool first_time = false;

		// Check for garbage (unsorted object IDs) in the input.
		if (object_id > shared_object_vector_.size()) {
			// We didn't get either a known ID or the next one to assign.
			return status::OUT_OF_RANGE;
		}

		// Enter a new object pointer if needed.
		if (object_id == shared_object_vector_.size()) {
			shared_object_vector_.push_back(shared_object_info { nullptr,
					registry::get_type_id<T>() });
			first_time = true;
		}

		// Get the shared object info.
		auto& object_info = shared_object_vector_[object_id];

		if (first_time) {
			// Deserialize the object the first time.
			return load_object_reference(p, update_pointer(object_info.ptr));
		} else {
			// Check for type compatibility.
			if (object_info.type_id != registry::get_type_id<T>()
					|| !registry::check_dynamic_compatibility<T>(
							object_info.ptr.get())) {
				// We recorded an incompatible type when we saw this object before,
				// or the object's dynamic type can't be stored in this pointer.
				return status::INVALID_ARGUMENT;
			}
			// Cast the object reference we already had.
			p = std::static_pointer_cast < T > (object_info.ptr);
			return status::OK;
		}
	}

	// Weak pointers.
	template<typename T>
	status operator()(std::weak_ptr<T>& p) {
		std::shared_ptr<T> shared;
		ASH_RETURN_IF_ERROR((*this)(shared));
		p = shared;
		return status::OK;
	}

private:
	struct class_info {
		std::string class_name;
		registry::dynamic_object_factory::factory_function_type factory_function;
		typename registry::dynamic_decoder_registry<MyClass>::decoder_function_type decoder_function;
	};

	// Read potentially short integers from a compact form.
	//
	// Use the highest bit in each byte to determine whether more bytes come
	// after the current one. The final byte will have its high order bit
	// clear.
	//
	// For convenience, this is always represented in little endian format.
	status_or<std::size_t> read_variant() {
		std::size_t l = 0;
		uint8_t o;
		do {
			ASH_RETURN_IF_ERROR((*this)(o));
			l <<= 7;
			l |= (static_cast<std::size_t>(o) & 0x7f);
		} while ((o & 0x80) != 0);
		return l;
	}

	template<typename P, typename O>
	status load_dynamic_object_reference(P& ptr, O o) {
		using T = typename P::element_type;

		// Read the class numeric ID for this stream.
		std::size_t class_id;
		ASH_ASSIGN_OR_RETURN(class_id, read_variant());

		// Check for garbage (unsorted class IDs) in the input.
		if (class_id > class_info_vector_.size()) {
			// We didn't get either a known ID or the next one to assign.
			return status::OUT_OF_RANGE;
		}

		// Construct the required info structure if needed.
		if (class_id == class_info_vector_.size()) {
			// Not seen yet. Need to read a class name and interrogate the dynamic_object_factory and dynamic_decoder_registry for this class.
			std::string class_name;
			ASH_RETURN_IF_ERROR((*this)(class_name));
			registry::dynamic_object_factory::factory_function_type factory_function;
			typename registry::dynamic_decoder_registry<MyClass>::decoder_function_type decoder_function;
			ASH_ASSIGN_OR_RETURN(factory_function,
					registry::dynamic_object_factory::get()[class_name.c_str()]);
			ASH_ASSIGN_OR_RETURN(decoder_function,
					registry::dynamic_decoder_registry<MyClass>::get()[class_name.c_str()]);
			class_info_vector_.push_back(class_info { class_name,
					factory_function, decoder_function });
		}

		// Info structure for the concrete class.
		const auto& info = class_info_vector_[class_id];

		// Check that the subclass is compatible (slow).
		if (!registry::dynamic_subclass_registry<T>::get().is_subclass(
				info.class_name.c_str()))
			return status::INVALID_ARGUMENT;

		// Instantiate the object.
		ptr.reset(static_cast<T*>(info.factory_function()));
		o(ptr);

		// Perform the decoding.
		return info.decoder_function(static_cast<MyClass&>(*this), *ptr);
	}

	template<typename P, typename O>
	typename std::enable_if<!is_dynamic<typename P::element_type>::value, status>::type load_object_reference(
			P& ptr, O o) {
		ptr.reset(new typename P::element_type());
		o(ptr);
		return (*this)(*ptr);
	}

	template<typename P, typename O>
	typename std::enable_if<is_dynamic<typename P::element_type>::value, status>::type load_object_reference(
			P& ptr, O o) {
		return load_dynamic_object_reference(ptr, o);
	}

	struct tuple_element_loader {
		template<typename T, typename S>
		status operator()(T& v, S& s) {
			return s(v);
		}
	};

	// Load base classes, if they exist.
	struct base_class_loader {
		template<typename T, typename B, typename S>
		status operator()(mpt::wrap_type<B>, T& o, S& s) {
			B& base = static_cast<B&>(o);
			return s(base);
		}
	};

	template<typename T>
	typename std::enable_if<traits::has_base_classes<T>::value, status>::type load_base_classes(
			T& o) {
		return apply_until_first_error(typename T::base_classes { },
				base_class_loader { }, o, *this);
	}

	template<typename T>
	typename std::enable_if<!traits::has_base_classes<T>::value, status>::type load_base_classes(
			T& o) {
		return status::OK;
	}

	// Load fields from field_descriptors, if present.
	struct field_loader {
		template<typename T, typename FD, typename S>
		status operator()(mpt::wrap_type<FD>, T& o, S& s) {
			return s(o.*(FD::member_pointer));
		}
	};

	template<typename T>
	typename std::enable_if<traits::has_field_descriptors<T>::value, status>::type load_fields(
			T& o) {
		return apply_until_first_error(typename T::field_descriptors { },
				field_loader { }, o, *this);
	}

	template<typename T>
	typename std::enable_if<!traits::has_field_descriptors<T>::value, status>::type load_fields(
			T& o) {
		return status::OK;
	}

	// Invoke the load method, if present.
	template<typename T>
	typename std::enable_if<traits::has_load<T, status(binary_decoder&)>::value,
			status>::type invoke_load(T& o) {
		return o.load(*this);
	}

	template<typename T>
	typename std::enable_if<
			!traits::has_load<T, status(binary_decoder&)>::value, status>::type invoke_load(
			T& o) {
		return status::OK;
	}

	// Reserve space in a container that supports a reserve method.
	template<typename T>
	typename std::enable_if<traits::has_storage_reservation<T>::value, void>::type maybe_reserve(
			T& t, typename T::size_type l) {
		t.reserve(l);
	}

	template<typename T>
	typename std::enable_if<!traits::has_storage_reservation<T>::value, void>::type maybe_reserve(
			T& t, typename T::size_type l) {
	}

	status_or<std::size_t> read_size() {
		return read_variant();
	}

	// For contiguous sequences of scalars, just treat them as a block of contiguous
	// memory.
	template<typename T>
	typename std::enable_if<traits::is_serializable_scalar<T>::value, status>::type read_sequence(
			T* p, std::size_t l) {
		return read_block(p, l);
	}

	// For non-trivial objects, call the whole serialization machinery.
	template<typename T>
	typename std::enable_if<!traits::is_serializable_scalar<T>::value, status>::type read_sequence(
			T* p, std::size_t l) {
		while (l-- > 0) {
			ASH_RETURN_IF_ERROR((*this)(*p++));
		}
		return status::OK;
	}

	// If we need to reverse the data, read it out byte by byte.
	template<typename T>
	typename std::enable_if<reverse_bytes && sizeof(T) != 1, status>::type read_block(
			T* p_, std::size_t l) {
		char *p = reinterpret_cast<char*>(p_);
		constexpr std::size_t data_size = sizeof(T);
		for (std::size_t i = 0; i < l; i++, p += data_size) {
			for (std::size_t j = 1; j <= data_size; j++) {
				ASH_ASSIGN_OR_RETURN(p[data_size - j], in_.getc());
			}
		}
		return status::OK;
	}

	// Just read out a whole block of memory if we don't need to reverse,
	// either because memory has the right endianness or the data type is byte-sized.
	template<typename T>
	typename std::enable_if<!reverse_bytes || sizeof(T) == 1, status>::type read_block(
			T* p, std::size_t l) {
		return in_.read_fully(reinterpret_cast<char*>(p), l * sizeof(T));
	}

	std::vector<class_info> class_info_vector_;
	std::vector<shared_object_info> shared_object_vector_ = {
			shared_object_info { nullptr, nullptr } };

protected:
	Adapter in_;
};

// Sizing OutputEncoder
class binary_sizer: public binary_encoder<binary_sizer, output_sizer, false> {
public:
	binary_sizer() :
			binary_encoder<binary_sizer, output_sizer, false>(output_sizer()) {
	}

	// Get the total number of bytes written so far.
	std::size_t size() {
		return out_.size();
	}

	// Reset the byte count so that we can reuse the object.
	void reset() {
		out_.reset();
	}
};

class native_binary_encoder: public binary_encoder<native_binary_encoder,
		output_adapter, false> {
	using binary_encoder<native_binary_encoder, output_adapter, false>::binary_encoder;
};
class reversing_binary_encoder: public binary_encoder<reversing_binary_encoder,
		output_adapter, true> {
	using binary_encoder<reversing_binary_encoder, output_adapter, true>::binary_encoder;
};
using little_endian_binary_encoder = mpt::conditional_t<traits::target_is_little_endian, native_binary_encoder, reversing_binary_encoder>;
using big_endian_binary_encoder = mpt::conditional_t<traits::target_is_big_endian, native_binary_encoder, reversing_binary_encoder>;

class native_binary_decoder: public binary_decoder<native_binary_decoder,
		input_adapter, false> {
	using binary_decoder<native_binary_decoder, input_adapter, false>::binary_decoder;
};
class reversing_binary_decoder: public binary_decoder<reversing_binary_decoder,
		input_adapter, true> {
	using binary_decoder<reversing_binary_decoder, input_adapter, true>::binary_decoder;
};
using little_endian_binary_decoder = mpt::conditional_t<traits::target_is_little_endian, native_binary_decoder, reversing_binary_decoder>;
using big_endian_binary_decoder = mpt::conditional_t<traits::target_is_big_endian, native_binary_decoder, reversing_binary_decoder>;

}  // namespace ash

#endif /* ASH_BINARY_CODECS_H_ */
