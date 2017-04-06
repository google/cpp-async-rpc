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
	typename std::enable_if<traits::is_serializable_scalar<T>::value, void>::type operator()(
			const T& v) {
		write_block(&v, 1);
	}

	// Contiguous sequences.
	template<typename T>
	typename std::enable_if<
			traits::is_iterable<T>::value
					&& traits::is_contiguous_sequence<T>::value, void>::type operator()(
			const T& sequence) {
		maybe_write_size(sequence);
		write_sequence(&(*sequence.cbegin()), sequence.size());
	}

	// Non-contiguous iterables.
	template<typename T>
	typename std::enable_if<
			traits::is_iterable<T>::value
					&& !traits::is_contiguous_sequence<T>::value, void>::type operator()(
			const T& sequence) {
		maybe_write_size(sequence);
		for (const auto& i : sequence) {
			(*this)(i);
		}
	}

	// Pairs.
	template<typename U, typename V>
	void operator()(const std::pair<U, V>& p) {
		(*this)(p.first);
		(*this)(p.second);
	}

	// Tuples.
	template<typename ... T>
	void operator()(const std::tuple<T ...>& t) {
		mpt::for_each(t, tuple_element_saver(), *this);
	}

	// Plain old arrays.
	template<typename T, std::size_t N>
	void operator()(const T (&a)[N]) {
		write_sequence(&a[0], N);
	}

	// Saveable objects.
	template<typename T>
	typename std::enable_if<traits::can_be_saved<T, binary_encoder>::value, void>::type operator()(
			const T& o) {
		save_base_classes(o);
		save_fields(o);
		invoke_save(o);
	}

	// Unique pointers.
	template<typename T, typename Deleter>
	void operator()(const std::unique_ptr<T, Deleter>& p) {
		bool present = (p != nullptr);
		(*this)(present);
		if (present) {
			(*this)(*p);
		}
	}

private:
	struct tuple_element_saver {
		template<typename T, typename S>
		void operator()(const T& v, S& s) {
			s(v);
		}
	};

	// Save base classes, if they exist.
	struct base_class_saver {
		template<typename T, typename B, typename S>
		void operator()(mpt::wrap_type<B>, const T& o, S& s) {
			const B& base = dynamic_cast<const B&>(o);
			s(base);
		}
	};

	template<typename T>
	typename std::enable_if<traits::has_base_classes<T>::value, void>::type save_base_classes(
			const T& o) {
		mpt::for_each(typename T::base_classes { }, base_class_saver { }, o,
				*this);
	}

	template<typename T>
	typename std::enable_if<!traits::has_base_classes<T>::value, void>::type save_base_classes(
			const T& o) {
	}

	// Save fields from field_descriptors, if present.
	struct field_saver {
		template<typename T, typename FD, typename S>
		void operator()(mpt::wrap_type<FD>, const T& o, S& s) {
			s(o.*(FD::member_pointer));
		}
	};

	template<typename T>
	typename std::enable_if<traits::has_field_descriptors<T>::value, void>::type save_fields(
			const T& o) {
		mpt::for_each(typename T::field_descriptors { }, field_saver { }, o,
				*this);
	}

	template<typename T>
	typename std::enable_if<!traits::has_field_descriptors<T>::value, void>::type save_fields(
			const T& o) {
	}

	// Invoke the save method, if present.
	template<typename T>
	typename std::enable_if<traits::has_save<T, void(binary_encoder&)>::value,
			void>::type invoke_save(const T& o) {
		o.save(*this);
	}

	template<typename T>
	typename std::enable_if<!traits::has_save<T, void(binary_encoder&)>::value,
			void>::type invoke_save(const T& o) {
	}

	// Write some container's size... or not (if it's known at compile time).
	template<typename T>
	typename std::enable_if<traits::has_static_size<T>::value, void>::type maybe_write_size(
			const T& t) {
	}

	template<typename T>
	typename std::enable_if<!traits::has_static_size<T>::value, void>::type maybe_write_size(
			const T& t) {
		write_variant(t.size());
	}

	// Save potentially short integers in a compact form.
	//
	// Use the highest bit in each byte to determine whether more bytes come
	// after the current one. The final byte will have its high order bit
	// clear.
	//
	// For convenience, this is always represented in little endian format.
	void write_variant(std::size_t l) {
		do {
			uint8_t o = static_cast<uint8_t>(l & 0x7f);
			l >>= 7;
			if (l) {
				o |= 0x80;
			}
			(*this)(o);
		} while (l);
	}

	// For contiguous sequences of scalars, just treat them as a block of contiguous
	// memory.
	template<typename T>
	typename std::enable_if<traits::is_serializable_scalar<T>::value, void>::type write_sequence(
			const T*p, std::size_t l) {
		write_block(p, l);
	}

	// For non-trivial objects, call the whole serialization machinery.
	template<typename T>
	typename std::enable_if<!traits::is_serializable_scalar<T>::value, void>::type write_sequence(
			const T*p, std::size_t l) {
		while (l-- > 0) {
			(*this)(*p++);
		}
	}

	// If we need to reverse the data, write it out byte by byte.
	template<typename T>
	typename std::enable_if<reverse_bytes && sizeof(T) != 1, void>::type write_block(
			const T* p_, std::size_t l) {
		const char *p = reinterpret_cast<const char*>(p_);
		constexpr std::size_t data_size = sizeof(T);
		for (std::size_t i = 0; i < l; i++, p += data_size) {
			for (std::size_t j = 1; j <= data_size; j++) {
				out_.putc(p[data_size - j]);
			}
		}
	}

	// Just write out a whole block of memory if we don't need to reverse,
	// either because memory has the right endianness or the data type is byte-sized.
	template<typename T>
	typename std::enable_if<!reverse_bytes || sizeof(T) == 1, void>::type write_block(
			const T* p, std::size_t l) {
		out_.write(reinterpret_cast<const char*>(p), l * sizeof(T));
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
	typename std::enable_if<traits::is_serializable_scalar<T>::value, void>::type operator()(
			T& v) {
		read_block(&v, 1);
	}

	// Contiguous sequences with fixed size (that is, std::array).
	template<typename T>
	typename std::enable_if<
			traits::is_iterable<T>::value
					&& traits::is_contiguous_sequence<T>::value
					&& traits::has_static_size<T>::value, void>::type operator()(
			T& sequence) {
		read_sequence(&(*sequence.begin()), sequence.size());
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
					&& !traits::is_associative<T>::value, void>::type operator()(
			T& sequence) {
		const auto l = read_size();
		// No need to reserve as we are setting the size directly.
		sequence.resize(l);
		read_sequence(&(*sequence.begin()), sequence.size());
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
					&& !traits::is_associative<T>::value, void>::type operator()(
			T& sequence) {
		auto l = read_size();
		sequence.clear();
		maybe_reserve(sequence, l);
		while (l-- > 0) {
			typename traits::deserializable_value_type<typename T::value_type>::type v;
			(*this)(v);
			sequence.push_back(std::move(v));
		}
	}

	// Associative containers.
	template<typename T>
	typename std::enable_if<
			traits::is_iterable<T>::value && !traits::has_static_size<T>::value
					&& traits::is_associative<T>::value, void>::type operator()(
			T& sequence) {
		auto l = read_size();
		sequence.clear();
		maybe_reserve(sequence, l);
		while (l-- > 0) {
			typename traits::deserializable_value_type<typename T::value_type>::type v;
			(*this)(v);
			sequence.insert(std::move(v));
		}
	}

	// Pairs.
	template<typename U, typename V>
	void operator()(std::pair<U, V>& p) {
		(*this)(p.first);
		(*this)(p.second);
	}

	// Tuples.
	template<typename ... T>
	void operator()(std::tuple<T ...>& t) {
		mpt::for_each(t, tuple_element_loader(), *this);
	}

	// Plain old arrays.
	template<typename T, std::size_t N>
	void operator()(T (&a)[N]) {
		read_sequence(&a[0], N);
	}

	// Loadable objects.
	template<typename T>
	typename std::enable_if<traits::can_be_loaded<T, binary_decoder>::value,
			void>::type operator()(T& o) {
		load_base_classes(o);
		load_fields(o);
		invoke_load(o);
	}

	// Unique pointers.
	template<typename T, typename Deleter>
	void operator()(std::unique_ptr<T, Deleter>& p) {
		bool present;
		(*this)(present);
		if (present) {
			p.reset(new T());
			(*this)(*p);
		} else {
			p.reset();
		}
	}

private:
	struct tuple_element_loader {
		template<typename T, typename S>
		void operator()(T& v, S& s) {
			s(v);
		}
	};

	// Load base classes, if they exist.
	struct base_class_loader {
		template<typename T, typename B, typename S>
		void operator()(mpt::wrap_type<B>, T& o, S& s) {
			B& base = dynamic_cast<B&>(o);
			s(base);
		}
	};

	template<typename T>
	typename std::enable_if<traits::has_base_classes<T>::value, void>::type load_base_classes(
			T& o) {
		mpt::for_each(typename T::base_classes { }, base_class_loader { }, o,
				*this);
	}

	template<typename T>
	typename std::enable_if<!traits::has_base_classes<T>::value, void>::type load_base_classes(
			T& o) {
	}

	// Load fields from field_descriptors, if present.
	struct field_loader {
		template<typename T, typename FD, typename S>
		void operator()(mpt::wrap_type<FD>, T& o, S& s) {
			s(o.*(FD::member_pointer));
		}
	};

	template<typename T>
	typename std::enable_if<traits::has_field_descriptors<T>::value, void>::type load_fields(
			T& o) {
		mpt::for_each(typename T::field_descriptors { }, field_loader { }, o,
				*this);
	}

	template<typename T>
	typename std::enable_if<!traits::has_field_descriptors<T>::value, void>::type load_fields(
			T& o) {
	}

	// Invoke the load method, if present.
	template<typename T>
	typename std::enable_if<traits::has_load<T, void(binary_decoder&)>::value,
			void>::type invoke_load(T& o) {
		o.load(*this);
	}

	template<typename T>
	typename std::enable_if<!traits::has_load<T, void(binary_decoder&)>::value,
			void>::type invoke_load(T& o) {
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

	std::size_t read_size() {
		return read_variant();
	}

	// Read potentially short integers from a compact form.
	//
	// Use the highest bit in each byte to determine whether more bytes come
	// after the current one. The final byte will have its high order bit
	// clear.
	//
	// For convenience, this is always represented in little endian format.
	std::size_t read_variant() {
		std::size_t l = 0;
		uint8_t o;
		do {
			(*this)(o);
			l <<= 7;
			l |= (static_cast<std::size_t>(o) & 0x7f);
		} while ((o & 0x80) != 0);
		return l;
	}

	// For contiguous sequences of scalars, just treat them as a block of contiguous
	// memory.
	template<typename T>
	typename std::enable_if<traits::is_serializable_scalar<T>::value, void>::type read_sequence(
			T* p, std::size_t l) {
		read_block(p, l);
	}

	// For non-trivial objects, call the whole serialization machinery.
	template<typename T>
	typename std::enable_if<!traits::is_serializable_scalar<T>::value, void>::type read_sequence(
			T* p, std::size_t l) {
		while (l-- > 0) {
			(*this)(*p++);
		}
	}

	// If we need to reverse the data, read it out byte by byte.
	template<typename T>
	typename std::enable_if<reverse_bytes && sizeof(T) != 1, void>::type read_block(
			T* p_, std::size_t l) {
		char *p = reinterpret_cast<char*>(p_);
		constexpr std::size_t data_size = sizeof(T);
		for (std::size_t i = 0; i < l; i++, p += data_size) {
			for (std::size_t j = 1; j <= data_size; j++) {
				p[data_size - j] = in_.getc();
			}
		}
	}

	// Just read out a whole block of memory if we don't need to reverse,
	// either because memory has the right endianness or the data type is byte-sized.
	template<typename T>
	typename std::enable_if<!reverse_bytes || sizeof(T) == 1, void>::type read_block(
			T* p, std::size_t l) {
		in_.read(reinterpret_cast<char*>(p), l * sizeof(T));
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
