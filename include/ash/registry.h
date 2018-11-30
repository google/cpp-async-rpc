/// \file
/// \brief Registry classes for encoders, decoders and dynamic class factories.
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

#ifndef INCLUDE_ASH_REGISTRY_H_
#define INCLUDE_ASH_REGISTRY_H_

#include <cstdint>
#include <cstring>
#include <map>
#include <memory>
#include <set>
#include <vector>
#include "ash/container/flat_map.h"
#include "ash/container/flat_set.h"
#include "ash/dynamic_base_class.h"
#include "ash/errors.h"
#include "ash/singleton.h"
#include "ash/type_hash.h"

namespace ash {

namespace registry {

namespace detail {
struct const_char_ptr_compare {
  bool operator()(const char* a, const char* b) const {
    return std::strcmp(a, b) < 0;
  }
};
}  // namespace detail

template <typename T>
class dynamic_subclass_registry
    : public singleton<dynamic_subclass_registry<T>> {
 public:
  void register_subclass(const char* class_name);
  bool is_subclass(const char* class_name) {
    return dynamic_subclass_set_.count(class_name) == 1;
  }

 private:
  ash::flat_set<const char*, detail::const_char_ptr_compare>
      dynamic_subclass_set_;
};

namespace detail {
struct register_subclass {
  template <typename T>
  void operator()(mpt::wrap_type<T>, const char* class_name) {
    dynamic_subclass_registry<T>::get().register_subclass(class_name);
  }
};
}  // namespace detail

template <typename T>
void dynamic_subclass_registry<T>::register_subclass(const char* class_name) {
  dynamic_subclass_set_.insert(class_name);
  mpt::for_each(typename T::dynamic_base_classes{}, detail::register_subclass{},
                class_name);
}

template <typename S>
class dynamic_encoder_registry : public singleton<dynamic_encoder_registry<S>> {
 public:
  using encoder_function_type = void (*)(S&, const ::ash::dynamic_base_class&);
  struct info {
    encoder_function_type encoder_function;
  };

  template <typename T>
  void register_class(const char* class_name) {
    encoder_function_type f = [](S& s, const ::ash::dynamic_base_class& o) {
      return s(static_cast<const T&>(o));
    };

    if (!encoder_info_map_.emplace(class_name, info{(f)}).second)
      throw errors::invalid_state("Duplicate encoder function registered");
  }

  info operator[](const char* class_name) const {
    const auto it = encoder_info_map_.find(class_name);
    if (it == encoder_info_map_.end())
      throw errors::not_found("Encoder function not found");
    return it->second;
  }

 private:
  ash::flat_map<const char*, info, detail::const_char_ptr_compare>
      encoder_info_map_;
};

namespace detail {
template <typename T>
struct register_encoder {
  template <typename S>
  void operator()(mpt::wrap_type<S>, const char* class_name) {
    dynamic_encoder_registry<S>::get().template register_class<T>(class_name);
  }
};
}  // namespace detail

template <typename S>
class dynamic_decoder_registry : public singleton<dynamic_decoder_registry<S>> {
 public:
  using decoder_function_type = void (*)(S&, ::ash::dynamic_base_class&);
  struct info {
    decoder_function_type decoder_function;
  };

  template <typename T>
  void register_class(const char* class_name) {
    decoder_function_type f = [](S& s, ::ash::dynamic_base_class& o) {
      return s(static_cast<T&>(o));
    };

    if (!decoder_info_map_.emplace(class_name, info{(f)}).second)
      throw errors::not_found("Duplicate decoder function registered");
  }

  info operator[](const char* class_name) const {
    const auto it = decoder_info_map_.find(class_name);
    if (it == decoder_info_map_.end())
      throw errors::not_found("Decoder function not found");
    return it->second;
  }

 private:
  ash::flat_map<const char*, info, detail::const_char_ptr_compare>
      decoder_info_map_;
};

namespace detail {
template <typename T>
struct register_decoder {
  template <typename S>
  void operator()(mpt::wrap_type<S>, const char* class_name) {
    dynamic_decoder_registry<S>::get().template register_class<T>(class_name);
  }
};
}  // namespace detail

class dynamic_object_factory : public singleton<dynamic_object_factory> {
 public:
  using factory_function_type = ::ash::dynamic_base_class* (*)();
  struct info {
    factory_function_type factory_function;
    std::uint32_t type_hash;
  };

  template <typename T, typename Encoders, typename Decoders>
  const char* register_class(const char* class_name) {
    static_assert(is_dynamic<T>::value,
                  "Only classes inheriting from ash::dynamic_base_class can be "
                  "registered for polymorphism");

    // Register the class into this factory for object creation.
    factory_function_type f = []() {
      return static_cast<::ash::dynamic_base_class*>(new T());
    };
    if (!factory_function_map_
             .emplace(class_name, info{(f), traits::type_hash<T>::value})
             .second)
      throw errors::not_found("Duplicate class registration");

    // Register the class into the class hierarchy.
    dynamic_subclass_registry<T>::get().register_subclass(class_name);

    // Register the encoders.
    mpt::for_each(Encoders{}, detail::register_encoder<T>{}, class_name);

    // Register the decoders.
    mpt::for_each(Decoders{}, detail::register_decoder<T>{}, class_name);

    return class_name;
  }

  info operator[](const char* class_name) const {
    const auto it = factory_function_map_.find(class_name);
    if (it == factory_function_map_.end())
      throw errors::not_found("Class factory function not found");
    return it->second;
  }

 private:
  ash::flat_map<const char*, info, detail::const_char_ptr_compare>
      factory_function_map_;
};

using type_id = const void*;

template <typename T>
typename std::enable_if<!is_dynamic<T>::value, type_id>::type get_type_id() {
  static const char c = 0;
  return &c;
}

template <typename T>
typename std::enable_if<is_dynamic<T>::value, type_id>::type get_type_id() {
  return nullptr;
}

template <typename T>
typename std::enable_if<!is_dynamic<T>::value, bool>::type
check_dynamic_compatibility(void* obj) {
  return true;
}

template <typename T>
typename std::enable_if<is_dynamic<T>::value, bool>::type
check_dynamic_compatibility(void* obj) {
  return dynamic_subclass_registry<T>::get().is_subclass(
      (static_cast<::ash::dynamic_base_class*>(obj))->portable_class_name());
}

}  // namespace registry

}  // namespace ash

#endif  // INCLUDE_ASH_REGISTRY_H_
