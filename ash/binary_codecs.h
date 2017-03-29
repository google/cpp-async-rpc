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
class BinaryEncoder {
public:
	BinaryEncoder(Adapter out) :
			out_(out) {
	}

	// Serializable scalar.
	template<typename T>
	typename std::enable_if<traits::is_serializable_scalar<T>::value, void>::type operator()(
			const T& v) {
		writeBlock(&v, 1);
	}

	// Contiguous sequences.
	template<typename T>
	typename std::enable_if<
			traits::is_iterable<T>::value
					&& traits::is_contiguous_sequence<T>::value, void>::type operator()(
			const T& sequence) {
		maybeWriteSize(sequence);
		writeSequence(&(*sequence.cbegin()), sequence.size());
	}

	// Non-contiguous iterables.
	template<typename T>
	typename std::enable_if<
			traits::is_iterable<T>::value
					&& !traits::is_contiguous_sequence<T>::value, void>::type operator()(
			const T& sequence) {
		maybeWriteSize(sequence);
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
		mpt::for_each(t, TupleElementSaver(), *this);
	}

	// Plain old arrays.
	template<typename T, std::size_t N>
	void operator()(const T (&a)[N]) {
		writeSequence(&a[0], N);
	}

	// Saveable objects.
	template<typename T>
	typename std::enable_if<traits::can_be_saved<T, BinaryEncoder>::value, void>::type operator()(
			const T& o) {
		saveBaseClasses(o);
		saveFields(o);
		invokeSave(o);
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
	struct TupleElementSaver {
		template<typename T, typename S>
		void operator()(const T& v, S& s) {
			s(v);
		}
	};

	// Save base classes, if they exist.
	struct BaseClassSaver {
		template<typename T, typename B, typename S>
		void operator()(mpt::wrap_type<B>, const T& o, S& s) {
			const B& base = dynamic_cast<const B&>(o);
			s(base);
		}
	};

	template<typename T>
	typename std::enable_if<traits::has_base_classes<T>::value, void>::type saveBaseClasses(
			const T& o) {
		mpt::for_each(typename T::base_classes { }, BaseClassSaver { }, o,
				*this);
	}

	template<typename T>
	typename std::enable_if<!traits::has_base_classes<T>::value, void>::type saveBaseClasses(
			const T& o) {
	}

	// Save fields from field_descriptors, if present.
	struct FieldSaver {
		template<typename T, typename FD, typename S>
		void operator()(mpt::wrap_type<FD>, const T& o, S& s) {
			s(o.*(FD::member_pointer));
		}
	};

	template<typename T>
	typename std::enable_if<traits::has_field_descriptors<T>::value, void>::type saveFields(
			const T& o) {
		mpt::for_each(typename T::field_descriptors { }, FieldSaver { }, o,
				*this);
	}

	template<typename T>
	typename std::enable_if<!traits::has_field_descriptors<T>::value, void>::type saveFields(
			const T& o) {
	}

	// Invoke the save method, if present.
	template<typename T>
	typename std::enable_if<traits::has_save<T, void(BinaryEncoder&)>::value,
			void>::type invokeSave(const T& o) {
		o.save(*this);
	}

	template<typename T>
	typename std::enable_if<!traits::has_save<T, void(BinaryEncoder&)>::value,
			void>::type invokeSave(const T& o) {
	}

	// Write some container's size... or not (if it's known at compile time).
	template<typename T>
	typename std::enable_if<traits::has_static_size<T>::value, void>::type maybeWriteSize(
			const T& t) {
	}

	template<typename T>
	typename std::enable_if<!traits::has_static_size<T>::value, void>::type maybeWriteSize(
			const T& t) {
		writeVariant(t.size());
	}

	// Save potentially short integers in a compact form.
	//
	// Use the highest bit in each byte to determine whether more bytes come
	// after the current one. The final byte will have its high order bit
	// clear.
	//
	// For convenience, this is always represented in little endian format.
	void writeVariant(std::size_t l) {
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
	typename std::enable_if<traits::is_serializable_scalar<T>::value, void>::type writeSequence(
			const T*p, std::size_t l) {
		writeBlock(p, l);
	}

	// For non-trivial objects, call the whole serialization machinery.
	template<typename T>
	typename std::enable_if<!traits::is_serializable_scalar<T>::value, void>::type writeSequence(
			const T*p, std::size_t l) {
		while (l-- > 0) {
			(*this)(*p++);
		}
	}

	// If we need to reverse the data, write it out byte by byte.
	template<typename T>
	typename std::enable_if<reverse_bytes && sizeof(T) != 1, void>::type writeBlock(
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
	typename std::enable_if<!reverse_bytes || sizeof(T) == 1, void>::type writeBlock(
			const T* p, std::size_t l) {
		out_.write(reinterpret_cast<const char*>(p), l * sizeof(T));
	}

protected:
	Adapter out_;
};

// Binary decoder.
template<typename Adapter, bool reverse_bytes>
class BinaryDecoder {
public:
	BinaryDecoder(Adapter in) :
			in_(in) {
	}

	// Serializable scalar.
	template<typename T>
	typename std::enable_if<traits::is_serializable_scalar<T>::value, void>::type operator()(
			T& v) {
		readBlock(&v, 1);
	}

	// Contiguous sequences with fixed size (that is, std::array).
	template<typename T>
	typename std::enable_if<
			traits::is_iterable<T>::value
					&& traits::is_contiguous_sequence<T>::value
					&& traits::has_static_size<T>::value, void>::type operator()(
			T& sequence) {
		readSequence(&(*sequence.begin()), sequence.size());
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
		const auto l = readSize();
		// No need to reserve as we are setting the size directly.
		sequence.resize(l);
		readSequence(&(*sequence.begin()), sequence.size());
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
		auto l = readSize();
		sequence.clear();
		maybeReserve(sequence, l);
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
		auto l = readSize();
		sequence.clear();
		maybeReserve(sequence, l);
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
		mpt::for_each(t, TupleElementLoader(), *this);
	}

	// Plain old arrays.
	template<typename T, std::size_t N>
	void operator()(T (&a)[N]) {
		readSequence(&a[0], N);
	}

	// Loadable objects.
	template<typename T>
	typename std::enable_if<traits::can_be_loaded<T, BinaryDecoder>::value, void>::type operator()(
			T& o) {
		loadBaseClasses(o);
		loadFields(o);
		invokeLoad(o);
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
	struct TupleElementLoader {
		template<typename T, typename S>
		void operator()(T& v, S& s) {
			s(v);
		}
	};

	// Load base classes, if they exist.
	struct BaseClassLoader {
		template<typename T, typename B, typename S>
		void operator()(mpt::wrap_type<B>, T& o, S& s) {
			B& base = dynamic_cast<B&>(o);
			s(base);
		}
	};

	template<typename T>
	typename std::enable_if<traits::has_base_classes<T>::value, void>::type loadBaseClasses(
			T& o) {
		mpt::for_each(typename T::base_classes { }, BaseClassLoader { }, o,
				*this);
	}

	template<typename T>
	typename std::enable_if<!traits::has_base_classes<T>::value, void>::type loadBaseClasses(
			T& o) {
	}

	// Load fields from field_descriptors, if present.
	struct FieldLoader {
		template<typename T, typename FD, typename S>
		void operator()(mpt::wrap_type<FD>, T& o, S& s) {
			s(o.*(FD::member_pointer));
		}
	};

	template<typename T>
	typename std::enable_if<traits::has_field_descriptors<T>::value, void>::type loadFields(
			T& o) {
		mpt::for_each(typename T::field_descriptors { }, FieldLoader { }, o,
				*this);
	}

	template<typename T>
	typename std::enable_if<!traits::has_field_descriptors<T>::value, void>::type loadFields(
			T& o) {
	}

	// Invoke the load method, if present.
	template<typename T>
	typename std::enable_if<traits::has_load<T, void(BinaryDecoder&)>::value,
			void>::type invokeLoad(T& o) {
		o.load(*this);
	}

	template<typename T>
	typename std::enable_if<!traits::has_load<T, void(BinaryDecoder&)>::value,
			void>::type invokeLoad(T& o) {
	}

	// Reserve space in a container that supports a reserve method.
	template<typename T>
	typename std::enable_if<traits::has_storage_reservation<T>::value, void>::type maybeReserve(
			T& t, typename T::size_type l) {
		t.reserve(l);
	}

	template<typename T>
	typename std::enable_if<!traits::has_storage_reservation<T>::value, void>::type maybeReserve(
			T& t, typename T::size_type l) {
	}

	std::size_t readSize() {
		return readVariant();
	}

	// Read potentially short integers from a compact form.
	//
	// Use the highest bit in each byte to determine whether more bytes come
	// after the current one. The final byte will have its high order bit
	// clear.
	//
	// For convenience, this is always represented in little endian format.
	std::size_t readVariant() {
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
	typename std::enable_if<traits::is_serializable_scalar<T>::value, void>::type readSequence(
			T* p, std::size_t l) {
		readBlock(p, l);
	}

	// For non-trivial objects, call the whole serialization machinery.
	template<typename T>
	typename std::enable_if<!traits::is_serializable_scalar<T>::value, void>::type readSequence(
			T* p, std::size_t l) {
		while (l-- > 0) {
			(*this)(*p++);
		}
	}

	// If we need to reverse the data, read it out byte by byte.
	template<typename T>
	typename std::enable_if<reverse_bytes && sizeof(T) != 1, void>::type readBlock(
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
	typename std::enable_if<!reverse_bytes || sizeof(T) == 1, void>::type readBlock(
			T* p, std::size_t l) {
		in_.read(reinterpret_cast<char*>(p), l * sizeof(T));
	}

protected:
	Adapter in_;
};

// Sizing OutputEncoder
class BinarySizer: public BinaryEncoder<OutputSizerAdapter, false> {
public:
	BinarySizer() :
			BinaryEncoder<OutputSizerAdapter, false>(OutputSizerAdapter()) {
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

using NativeBinaryEncoder = BinaryEncoder<DelegatingOutputAdapter, false>;
using LittleEndianBinaryEncoder = BinaryEncoder<DelegatingOutputAdapter, !traits::target_is_little_endian>;
using BigEndianBinaryEncoder = BinaryEncoder<DelegatingOutputAdapter, !traits::target_is_big_endian>;

using NativeBinaryDecoder = BinaryDecoder<DelegatingInputAdapter, false>;
using LittleEndianBinaryDecoder = BinaryDecoder<DelegatingOutputAdapter, !traits::target_is_little_endian>;
using BigEndianBinaryDecoder = BinaryDecoder<DelegatingOutputAdapter, !traits::target_is_big_endian>;

}  // namespace ash

#endif /* ASH_BINARY_CODECS_H_ */
