/// \file
/// \brief Binary format codecs for serialization.
///
/// \copyright
///   Copyright 2018 by Google Inc. All Rights Reserved.
///
/// \copyright
///   Licensed under the Apache License, Version 2.0 (the "License"); you may
///   not use this file except in compliance with the License. You may obtain a
///   copy of the License at
///
/// \copyright
///   http://www.apache.org/licenses/LICENSE-2.0
///
/// \copyright
///   Unless required by applicable law or agreed to in writing, software
///   distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
///   WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
///   License for the specific language governing permissions and limitations
///   under the License.

#ifndef INCLUDE_ASH_BINARY_CODECS_H_
#define INCLUDE_ASH_BINARY_CODECS_H_

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include "ash/container/flat_map.h"
#include "ash/errors.h"
#include "ash/io_adapters.h"
#include "ash/mpt.h"
#include "ash/registry.h"
#include "ash/traits/container_traits.h"
#include "ash/traits/serialization_traits.h"
#include "ash/traits/type_traits.h"
#include "ash/type_hash.h"

namespace ash {

// Tag class to enable type compatibility hashes in serialized data.
struct verify_structure {};

// Binary encoder.
template <typename MyClass, typename Adapter, bool reverse_bytes>
class binary_encoder {
 public:
  explicit binary_encoder(Adapter out) : out_(out) {}

  // Tag processing variant that'll write a structure hash to the output.
  template <typename T, typename... Tags>
  void operator()(const T& o, verify_structure, Tags... tags) {
    const std::uint32_t type_hash = traits::type_hash_v<T>;
    (*this)(type_hash);
    return (*this)(o, tags...);
  }

  // Special case for const char*, compatible with std::string de-serialization.
  void operator()(const char* s) {
    std::size_t l = std::strlen(s);
    write_variant(l);
    write_block(s, l);
  }

  // Serializable scalar.
  template <typename T>
  std::enable_if_t<traits::is_bit_transferrable_scalar_v<T>, void> operator()(
      const T& v) {
    write_block(&v, 1);
  }

  // Contiguous sequences.
  template <typename T>
  std::enable_if_t<traits::is_const_iterable_v<T> &&
                       traits::is_contiguous_sequence_v<T>,
                   void>
  operator()(const T& sequence) {
    maybe_write_size(sequence);
    write_sequence(&*std::begin(sequence),
                   std::end(sequence) - std::begin(sequence));
  }

  // Non-contiguous iterables.
  template <typename T>
  std::enable_if_t<traits::is_const_iterable_v<T> &&
                       !traits::is_contiguous_sequence_v<T>,
                   void>
  operator()(const T& sequence) {
    maybe_write_size(sequence);
    for (const auto& i : sequence) {
      (*this)(i);
    }
  }

  // Pairs.
  template <typename U, typename V>
  void operator()(const std::pair<U, V>& p) {
    (*this)(p.first);
    (*this)(p.second);
  }

  // Tuples.
  template <typename... T>
  void operator()(const std::tuple<T...>& t) {
    mpt::for_each(t, tuple_element_saver(), *this);
  }

  // Saveable objects.
  template <typename T>
  std::enable_if_t<traits::can_be_serialized_v<T>, void> operator()(
      const T& o) {
    save_base_classes(o);
    save_fields(o);
    invoke_save(o);
  }

  // Unique pointers.
  template <typename T, typename Deleter>
  void operator()(const std::unique_ptr<T, Deleter>& p) {
    bool present = (p != nullptr);
    (*this)(present);
    if (present) {
      save_object_reference(*p);
    }
  }

  // Shared pointers.
  template <typename T>
  void operator()(const std::shared_ptr<T>& p) {
    // Short-circuit nullptr.
    if (!p) {
      write_variant(0);
      return;
    }

    auto it = shared_object_map_.find(p.get());
    bool first_time = false;
    if (it == shared_object_map_.end()) {
      std::size_t next_object_id = shared_object_map_.size();
      it = shared_object_map_.emplace(p.get(), next_object_id).first;
      first_time = true;
    }
    write_variant(it->second);
    if (first_time) {
      save_object_reference(*p);
    }
  }

  // Weak pointers.
  template <typename T>
  void operator()(const std::weak_ptr<T>& p) {
    std::shared_ptr<T> shared = p.lock();
    (*this)(shared);
  }

 private:
  struct seen_class_info {
    std::size_t class_id;
    typename registry::dynamic_encoder_registry<MyClass>::encoder_function_type
        encoder_function;
  };

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

  void save_dynamic_object_reference(const ::ash::dynamic_base_class& o) {
    const char* class_name = o.portable_class_name();
    auto it = class_info_map_.find(class_name);
    if (it == class_info_map_.end()) {
      // Not cached yet. Need to interrogate the registries for this class.
      typename registry::dynamic_object_factory::info class_info;
      typename registry::dynamic_encoder_registry<MyClass>::info encoder_info;
      class_info = registry::dynamic_object_factory::get()[class_name];
      encoder_info =
          registry::dynamic_encoder_registry<MyClass>::get()[class_name];
      std::size_t next_class_id = class_info_map_.size();
      it = class_info_map_
               .emplace(class_name,
                        seen_class_info{next_class_id,
                                        encoder_info.encoder_function})
               .first;
      write_variant(it->second.class_id);
      (*this)(class_name);
      (*this)(class_info.type_hash);
    } else {
      write_variant(it->second.class_id);
    }
    it->second.encoder_function(static_cast<MyClass&>(*this), o);
  }

  template <typename T>
  std::enable_if_t<!is_dynamic_v<T>, void> save_object_reference(const T& o) {
    (*this)(o);
  }

  template <typename T>
  std::enable_if_t<is_dynamic_v<T>, void> save_object_reference(const T& o) {
    save_dynamic_object_reference(o);
  }

  struct tuple_element_saver {
    template <typename T, typename S>
    void operator()(const T& v, S& s) {
      s(v);
    }
  };

  // Save base classes, if they exist.
  struct base_class_saver {
    template <typename T, typename B, typename S>
    void operator()(mpt::wrap_type<B>, const T& o, S& s) {
      const B& base = static_cast<const B&>(o);
      s(base);
    }
  };

  template <typename T>
  void save_base_classes(const T& o) {
    mpt::for_each(traits::get_base_classes_t<T>{}, base_class_saver{}, o,
                  *this);
  }

  // Save fields from field_descriptors, if present.
  struct field_saver {
    template <typename T, typename FD, typename S>
    void operator()(mpt::wrap_type<FD>, const T& o, S& s) {
      s(o.*(FD::data_ptr));
    }
  };

  template <typename T>
  void save_fields(const T& o) {
    mpt::for_each(traits::get_field_descriptors_t<T>{}, field_saver{}, o,
                  *this);
  }

  // Invoke the save method, if present.
  template <typename T>
  std::enable_if_t<traits::has_custom_serialization_v<T>, void> invoke_save(
      const T& o) {
    static_assert(traits::get_custom_serialization_version_v<T> != 0,
                  "Custom serialization version must be non-zero.");
    o.save(*this);
  }

  template <typename T>
  std::enable_if_t<!traits::has_custom_serialization_v<T>, void> invoke_save(
      const T& o) {}

  // Write some container's size... or not (if it's known at compile time).
  template <typename T>
  typename std::enable_if<traits::has_static_size_v<T>, void>::type
  maybe_write_size(const T& t) {}

  template <typename T>
  std::enable_if_t<!traits::has_static_size_v<T>, void> maybe_write_size(
      const T& t) {
    write_variant(t.size());
  }

  // For contiguous sequences of scalars, just treat them as a block of
  // contiguous
  // memory.
  template <typename T>
  std::enable_if_t<traits::is_bit_transferrable_scalar_v<T>, void>
  write_sequence(const T* p, std::size_t l) {
    write_block(p, l);
  }

  // For non-trivial objects, call the whole serialization machinery.
  template <typename T>
  std::enable_if_t<!traits::is_bit_transferrable_scalar_v<T>, void>
  write_sequence(const T* p, std::size_t l) {
    while (l-- > 0) {
      (*this)(*p++);
    }
  }

  // If we need to reverse the data, write it out byte by byte.
  template <typename T>
  std::enable_if_t<reverse_bytes && sizeof(T) != 1, void> write_block(
      const T* p_, std::size_t l) {
    const char* p = reinterpret_cast<const char*>(p_);
    constexpr std::size_t data_size = sizeof(T);
    for (std::size_t i = 0; i < l; i++, p += data_size) {
      for (std::size_t j = 1; j <= data_size; j++) {
        out_.putc(p[data_size - j]);
      }
    }
  }

  // Just write out a whole block of memory if we don't need to reverse,
  // either because memory has the right endianness or the data type is
  // byte-sized.
  template <typename T>
  std::enable_if_t<!reverse_bytes || sizeof(T) == 1, void> write_block(
      const T* p, std::size_t l) {
    out_.write(reinterpret_cast<const char*>(p), l * sizeof(T));
  }

  // Here we depend on portable_class_name() returning identical pointers for
  // speed.
  ash::flat_map<const char*, seen_class_info> class_info_map_;
  ash::flat_map<void*, std::size_t> shared_object_map_ = {{nullptr, 0}};

 protected:
  Adapter out_;
};

// Binary decoder.
template <typename MyClass, typename Adapter, bool reverse_bytes>
class binary_decoder {
 private:
  class update_pointer {
   public:
    explicit update_pointer(std::shared_ptr<void>& ptr) : ptr_(ptr) {}
    template <typename T>
    void operator()(const std::shared_ptr<T>& ptr) {
      ptr_ = ptr;
    }

   private:
    std::shared_ptr<void>& ptr_;
  };

  class no_pointer_update {
   public:
    template <typename T>
    void operator()(const std::unique_ptr<T>& ptr) {}
  };

  struct shared_object_info {
    std::shared_ptr<void> ptr;
    registry::type_id type_id;
  };

 public:
  explicit binary_decoder(Adapter in) : in_(in) {}

  // Tag processing variant that'll read a structure hash to the input and
  // verify it.
  template <typename T, typename... Tags>
  void operator()(T& o, verify_structure, Tags... tags) {
    std::uint32_t type_hash;
    (*this)(type_hash);
    if (type_hash != traits::type_hash_v<T>) {
      throw errors::data_mismatch("Wrong type hash in verified read");
    }
    (*this)(o, tags...);
  }

  // Serializable scalar.
  template <typename T>
  std::enable_if_t<traits::is_bit_transferrable_scalar_v<T>, void> operator()(
      T& v) {
    read_block(&v, 1);
  }

  // Contiguous sequences with fixed size (that is, std::array).
  template <typename T>
  std::enable_if_t<traits::is_iterable_v<T> &&
                       traits::is_contiguous_sequence_v<T> &&
                       traits::has_static_size_v<T>,
                   void>
  operator()(T& sequence) {
    read_sequence(&*std::begin(sequence), traits::static_size_v<T>);
  }

  // Contiguous sequences that can be resized, and contain just scalars.
  // These we don't mind resizing and default-initializing, so that we can
  // overwrite the values in a single block read. This should match strings
  // and vectors of scalars.
  template <typename T>
  std::enable_if_t<
      traits::is_iterable_v<T> && !traits::has_static_size_v<T> &&
          traits::is_contiguous_sequence_v<T> && traits::can_be_resized_v<T> &&
          traits::is_bit_transferrable_scalar_v<typename T::value_type> &&
          !traits::is_associative_v<T>,
      void>
  operator()(T& sequence) {
    std::size_t l = read_size();
    // No need to reserve as we are setting the size directly.
    sequence.resize(l);
    read_sequence(&*std::begin(sequence), sequence.size());
  }

  // Containers where we need to read elements one by one and push_back them.
  // This should apply to deques, lists and vectors of non-scalars.
  template <typename T>
  std::enable_if_t<
      traits::is_iterable_v<T> && !traits::has_static_size_v<T> &&
          (!traits::is_contiguous_sequence_v<T> ||
           !traits::can_be_resized_v<T> ||
           !traits::is_bit_transferrable_scalar_v<
               typename T::value_type>)&&!traits::is_associative_v<T>,
      void>
  operator()(T& sequence) {
    std::size_t l = read_size();
    sequence.clear();
    maybe_reserve(sequence, l);
    while (l-- > 0) {
      traits::writable_value_type_t<typename T::value_type> v;
      (*this)(v);
      sequence.push_back(std::move(v));
    }
  }

  // Associative containers.
  template <typename T>
  std::enable_if_t<traits::is_iterable_v<T> && !traits::has_static_size_v<T> &&
                       traits::is_associative_v<T>,
                   void>
  operator()(T& sequence) {
    std::size_t l = read_size();
    sequence.clear();
    maybe_reserve(sequence, l);
    while (l-- > 0) {
      traits::writable_value_type_t<typename T::value_type> v;
      (*this)(v);
      sequence.insert(std::move(v));
    }
  }

  // Pairs.
  template <typename U, typename V>
  void operator()(std::pair<U, V>& p) {
    (*this)(p.first);
    (*this)(p.second);
  }

  // Tuples.
  template <typename... T>
  void operator()(std::tuple<T...>& t) {
    mpt::for_each(t, tuple_element_loader(), *this);
  }

  // Loadable objects.
  template <typename T>
  std::enable_if_t<traits::can_be_serialized_v<T>, void> operator()(T& o) {
    load_base_classes(o);
    load_fields(o);
    invoke_load(o);
  }

  // Unique pointers.
  template <typename T, typename Deleter>
  void operator()(std::unique_ptr<T, Deleter>& p) {
    bool present;
    (*this)(present);
    if (present) {
      load_object_reference(p, no_pointer_update());
    } else {
      p.reset();
    }
  }

  // Shared pointers.
  template <typename T>
  void operator()(std::shared_ptr<T>& p) {
    // Read the object numeric ID for this stream.
    std::size_t object_id = read_variant();

    // Short-circuit nullptr.
    if (object_id == 0) {
      p.reset();
      return;
    }

    // Are we reading a new object?
    bool first_time = false;

    // Check for garbage (unsorted object IDs) in the input.
    if (object_id > shared_object_vector_.size()) {
      // We didn't get either a known ID or the next one to assign.
      throw errors::data_mismatch("Wrong object_id in stream");
    }

    // Enter a new object pointer if needed.
    if (object_id == shared_object_vector_.size()) {
      shared_object_vector_.push_back(
          shared_object_info{nullptr, registry::get_type_id<T>()});
      first_time = true;
    }

    // Get the shared object info.
    auto& object_info = shared_object_vector_[object_id];

    if (first_time) {
      // Deserialize the object the first time.
      load_object_reference(p, update_pointer(object_info.ptr));
    } else {
      // Check for type compatibility.
      if (object_info.type_id != registry::get_type_id<T>() ||
          !registry::check_dynamic_compatibility<T>(object_info.ptr.get())) {
        // We recorded an incompatible type when we saw this object before,
        // or the object's dynamic type can't be stored in this pointer.
        throw errors::data_mismatch("Data shared among incompatible pointers");
      }
      // Cast the object reference we already had.
      p = std::static_pointer_cast<T>(object_info.ptr);
    }
  }

  // Weak pointers.
  template <typename T>
  void operator()(std::weak_ptr<T>& p) {
    std::shared_ptr<T> shared;
    (*this)(shared);
    p = shared;
  }

 private:
  struct seen_class_info {
    std::string class_name;
    registry::dynamic_object_factory::factory_function_type factory_function;
    typename registry::dynamic_decoder_registry<MyClass>::decoder_function_type
        decoder_function;
  };

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

  template <typename P, typename O>
  void load_dynamic_object_reference(P& ptr, O o) {
    using T = typename P::element_type;

    // Read the class numeric ID for this stream.
    std::size_t class_id = read_variant();

    // Check for garbage (unsorted class IDs) in the input.
    if (class_id > class_info_vector_.size()) {
      // We didn't get either a known ID or the next one to assign.
      throw errors::data_mismatch("Received wrong class_id in stream");
    }

    // Construct the required info structure if needed.
    if (class_id == class_info_vector_.size()) {
      // Not seen yet. Need to read a class name and interrogate the
      // dynamic_object_factory and dynamic_decoder_registry for this class.
      std::string class_name;
      (*this)(class_name);
      std::uint32_t type_hash;
      (*this)(type_hash);
      registry::dynamic_object_factory::info class_info;
      typename registry::dynamic_decoder_registry<MyClass>::info decoder_info;
      class_info = registry::dynamic_object_factory::get()[class_name.c_str()];
      decoder_info = registry::dynamic_decoder_registry<
          MyClass>::get()[class_name.c_str()];
      if (type_hash != class_info.type_hash) {
        throw errors::data_mismatch("Wrong type hash in dynamic class");
      }
      class_info_vector_.push_back(
          seen_class_info{class_name, class_info.factory_function,
                          decoder_info.decoder_function});
    }

    // Info structure for the concrete class.
    const auto& info = class_info_vector_[class_id];

    // Check that the subclass is compatible (slow).
    if (!registry::dynamic_subclass_registry<T>::get().is_subclass(
            info.class_name.c_str()))
      throw errors::data_mismatch(
          "The class that was read is not a subclass of pointed-to type");

    // Instantiate the object.
    ptr.reset(static_cast<T*>(info.factory_function()));
    o(ptr);

    // Perform the decoding.
    info.decoder_function(static_cast<MyClass&>(*this), *ptr);
  }

  template <typename P, typename O>
  std::enable_if_t<!is_dynamic_v<typename P::element_type>, void>
  load_object_reference(P& ptr, O o) {
    ptr.reset(new typename P::element_type());
    o(ptr);
    (*this)(*ptr);
  }

  template <typename P, typename O>
  std::enable_if_t<is_dynamic_v<typename P::element_type>, void>
  load_object_reference(P& ptr, O o) {
    load_dynamic_object_reference(ptr, o);
  }

  struct tuple_element_loader {
    template <typename T, typename S>
    void operator()(T& v, S& s) {
      s(v);
    }
  };

  // Load base classes, if they exist.
  struct base_class_loader {
    template <typename T, typename B, typename S>
    void operator()(mpt::wrap_type<B>, T& o, S& s) {
      B& base = static_cast<B&>(o);
      s(base);
    }
  };

  template <typename T>
  void load_base_classes(T& o) {
    mpt::for_each(traits::get_base_classes_t<T>{}, base_class_loader{}, o,
                  *this);
  }

  // Load fields from field_descriptors, if present.
  struct field_loader {
    template <typename T, typename FD, typename S>
    void operator()(mpt::wrap_type<FD>, T& o, S& s) {
      s(o.*(FD::data_ptr));
    }
  };

  template <typename T>
  void load_fields(T& o) {
    mpt::for_each(traits::get_field_descriptors_t<T>{}, field_loader{}, o,
                  *this);
  }

  // Invoke the load method, if present.
  template <typename T>
  std::enable_if_t<traits::has_custom_serialization_v<T>, void> invoke_load(
      T& o) {
    static_assert(traits::get_custom_serialization_version_v<T> != 0,
                  "Custom serialization version must be non-zero.");
    o.load(*this);
  }

  template <typename T>
  std::enable_if_t<!traits::has_custom_serialization_v<T>, void> invoke_load(
      T& o) {}

  // Reserve space in a container that supports a reserve method.
  template <typename T>
  std::enable_if_t<traits::can_reserve_capacity_v<T>, void> maybe_reserve(
      T& t, typename T::size_type l) {
    t.reserve(l);
  }

  template <typename T>
  std::enable_if_t<!traits::can_reserve_capacity_v<T>, void> maybe_reserve(
      T& t, typename T::size_type l) {}

  std::size_t read_size() { return read_variant(); }

  // For contiguous sequences of scalars, just treat them as a block of
  // contiguous
  // memory.
  template <typename T>
  std::enable_if_t<traits::is_bit_transferrable_scalar_v<T>, void>
  read_sequence(T* p, std::size_t l) {
    read_block(p, l);
  }

  // For non-trivial objects, call the whole serialization machinery.
  template <typename T>
  std::enable_if_t<!traits::is_bit_transferrable_scalar_v<T>, void>
  read_sequence(T* p, std::size_t l) {
    while (l-- > 0) {
      (*this)(*p++);
    }
  }

  // If we need to reverse the data, read it into place and reverse.
  template <typename T>
  std::enable_if_t<reverse_bytes && sizeof(T) != 1, void> read_block(
      T* p, std::size_t l) {
    in_.read_fully(reinterpret_cast<char*>(p), l * sizeof(T));
    for (std::size_t i = 0; i < l; i++, p++) {
      char* bp = reinterpret_cast<char*>(p);
      std::reverse(bp, bp + sizeof(T));
    }
  }

  // Just read out a whole block of memory if we don't need to reverse,
  // either because memory has the right endianness or the data type is
  // byte-sized.
  template <typename T>
  std::enable_if_t<!reverse_bytes || sizeof(T) == 1, void> read_block(
      T* p, std::size_t l) {
    in_.read_fully(reinterpret_cast<char*>(p), l * sizeof(T));
  }

  std::vector<seen_class_info> class_info_vector_;
  std::vector<shared_object_info> shared_object_vector_ = {
      shared_object_info{nullptr, nullptr}};

 protected:
  Adapter in_;
};

// Sizing OutputEncoder
class binary_sizer : public binary_encoder<binary_sizer, output_sizer, false> {
 public:
  binary_sizer()
      : binary_encoder<binary_sizer, output_sizer, false>(output_sizer()) {}

  // Get the total number of bytes written so far.
  std::size_t size() { return out_.size(); }

  // Reset the byte count so that we can reuse the object.
  void reset() { out_.reset(); }
};

class native_binary_encoder
    : public binary_encoder<native_binary_encoder, output_adapter, false> {
  using binary_encoder<native_binary_encoder, output_adapter,
                       false>::binary_encoder;
};
class reversing_binary_encoder
    : public binary_encoder<reversing_binary_encoder, output_adapter, true> {
  using binary_encoder<reversing_binary_encoder, output_adapter,
                       true>::binary_encoder;
};
using little_endian_binary_encoder =
    std::conditional_t<traits::target_is_little_endian, native_binary_encoder,
                       reversing_binary_encoder>;
using big_endian_binary_encoder =
    std::conditional_t<traits::target_is_big_endian, native_binary_encoder,
                       reversing_binary_encoder>;

class native_binary_decoder
    : public binary_decoder<native_binary_decoder, input_adapter, false> {
  using binary_decoder<native_binary_decoder, input_adapter,
                       false>::binary_decoder;
};
class reversing_binary_decoder
    : public binary_decoder<reversing_binary_decoder, input_adapter, true> {
  using binary_decoder<reversing_binary_decoder, input_adapter,
                       true>::binary_decoder;
};
using little_endian_binary_decoder =
    std::conditional_t<traits::target_is_little_endian, native_binary_decoder,
                       reversing_binary_decoder>;
using big_endian_binary_decoder =
    std::conditional_t<traits::target_is_big_endian, native_binary_decoder,
                       reversing_binary_decoder>;

}  // namespace ash

#endif  // INCLUDE_ASH_BINARY_CODECS_H_
