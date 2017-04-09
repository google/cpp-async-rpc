#ifndef ASH_BINARY_CODECS_H_
#define ASH_BINARY_CODECS_H_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <tuple>

#include "ash/traits.h"
#include "ash/mpt.h"
#include "ash/io_adapters.h"

namespace ash {

// Binary encoder.
template<typename Adapter, bool reverse_bytes>
class binary_encoder {
public:
	binary_encoder(Adapter out) :
			out_(out) {
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
			ASH_RETURN_IF_ERROR((*this)(*p));
		}
		return status::OK;
	}

private:
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

protected:
	Adapter out_;
};

// Binary decoder.
template<typename Adapter, bool reverse_bytes>
class binary_decoder {
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
			p.reset(new T());
			return (*this)(*p);
		} else {
			p.reset();
			return status::OK;
		}
	}

private:
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
	}

	// Just read out a whole block of memory if we don't need to reverse,
	// either because memory has the right endianness or the data type is byte-sized.
	template<typename T>
	typename std::enable_if<!reverse_bytes || sizeof(T) == 1, status>::type read_block(
			T* p, std::size_t l) {
		return in_.read_fully(reinterpret_cast<char*>(p), l * sizeof(T));
	}

protected:
	Adapter in_;
};

// Sizing OutputEncoder
class binary_sizer: public binary_encoder<output_sizer, false> {
public:
	binary_sizer() :
			binary_encoder<output_sizer, false>(output_sizer()) {
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

using native_binary_encoder = binary_encoder<output_adapter, false>;
using little_endian_binary_encoder = binary_encoder<output_adapter, !traits::target_is_little_endian>;
using big_endian_binary_encoder = binary_encoder<output_adapter, !traits::target_is_big_endian>;

using native_binary_decoder = binary_decoder<input_adapter, false>;
using little_endian_binary_decoder = binary_decoder<output_adapter, !traits::target_is_little_endian>;
using big_endian_binary_decoder = binary_decoder<output_adapter, !traits::target_is_big_endian>;

}  // namespace ash

#endif /* ASH_BINARY_CODECS_H_ */
