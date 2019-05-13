/// \file
/// \brief Registry classes for encoders, decoders and dynamic class factories.
///
/// \copyright
///   Copyright 2019 by Google LLC.
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

#ifndef ARPC_REGISTRY_H_
#define ARPC_REGISTRY_H_

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string_view>
#include <vector>
#include "arpc/container/flat_map.h"
#include "arpc/container/flat_set.h"
#include "arpc/dynamic_base_class.h"
#include "arpc/errors.h"
#include "arpc/singleton.h"
#include "arpc/type_hash.h"

namespace arpc {

namespace registry {

template <typename T>
class dynamic_subclass_registry
    : public singleton<dynamic_subclass_registry<T>> {
 public:
  void register_subclass(std::string_view class_name);
  bool is_subclass(std::string_view class_name) {
    return dynamic_subclass_set_.count(class_name) == 1;
  }

 private:
  arpc::flat_set<std::string_view> dynamic_subclass_set_;
};

namespace detail {
struct register_subclass {
  template <typename T>
  void operator()(mpt::wrap_type<T>, std::string_view class_name) {
    dynamic_subclass_registry<T>::get().register_subclass(class_name);
  }
};
}  // namespace detail

template <typename T>
void dynamic_subclass_registry<T>::register_subclass(
    std::string_view class_name) {
  dynamic_subclass_set_.insert(class_name);
  mpt::for_each(typename T::dynamic_base_classes{}, detail::register_subclass{},
                class_name);
}

template <typename S>
class dynamic_encoder_registry : public singleton<dynamic_encoder_registry<S>> {
 public:
  using encoder_function_type = void (*)(S&, const ::arpc::dynamic_base_class&);
  struct info {
    encoder_function_type encoder_function;
  };

  template <typename T>
  void register_class(std::string_view class_name) {
    encoder_function_type f = [](S& s, const ::arpc::dynamic_base_class& o) {
      return s(static_cast<const T&>(o));
    };

    if (!encoder_info_map_.emplace(class_name, info{(f)}).second)
      throw errors::invalid_state("Duplicate encoder function registered");
  }

  info operator[](std::string_view class_name) const {
    const auto it = encoder_info_map_.find(class_name);
    if (it == encoder_info_map_.end())
      throw errors::not_found("Encoder function not found");
    return it->second;
  }

 private:
  arpc::flat_map<std::string_view, info> encoder_info_map_;
};

namespace detail {
template <typename T>
struct register_encoder {
  template <typename S>
  void operator()(mpt::wrap_type<S>, std::string_view class_name) {
    dynamic_encoder_registry<S>::get().template register_class<T>(class_name);
  }
};
}  // namespace detail

template <typename S>
class dynamic_decoder_registry : public singleton<dynamic_decoder_registry<S>> {
 public:
  using decoder_function_type = void (*)(S&, ::arpc::dynamic_base_class&);
  struct info {
    decoder_function_type decoder_function;
  };

  template <typename T>
  void register_class(std::string_view class_name) {
    decoder_function_type f = [](S& s, ::arpc::dynamic_base_class& o) {
      return s(static_cast<T&>(o));
    };

    if (!decoder_info_map_.emplace(class_name, info{(f)}).second)
      throw errors::invalid_state("Duplicate decoder function registered");
  }

  info operator[](std::string_view class_name) const {
    const auto it = decoder_info_map_.find(class_name);
    if (it == decoder_info_map_.end())
      throw errors::not_found("Decoder function not found");
    return it->second;
  }

 private:
  arpc::flat_map<std::string_view, info> decoder_info_map_;
};

namespace detail {
template <typename T>
struct register_decoder {
  template <typename S>
  void operator()(mpt::wrap_type<S>, std::string_view class_name) {
    dynamic_decoder_registry<S>::get().template register_class<T>(class_name);
  }
};
}  // namespace detail

class dynamic_object_factory : public singleton<dynamic_object_factory> {
 public:
  using factory_function_type = ::arpc::dynamic_base_class* (*)();
  struct info {
    factory_function_type factory_function;
    std::uint32_t type_hash;
  };

  template <typename T, typename Encoders, typename Decoders>
  std::string_view register_class(std::string_view class_name) {
    static_assert(
        is_dynamic<T>::value,
        "Only classes inheriting from arpc::dynamic_base_class can be "
        "registered for polymorphism");

    // Register the class into this factory for object creation.
    factory_function_type f = []() {
      return static_cast<::arpc::dynamic_base_class*>(new T());
    };
    if (!factory_function_map_
             .emplace(class_name, info{(f), traits::type_hash_v<T>})
             .second)
      throw errors::invalid_state("Duplicate class registration");

    // Register the class into the class hierarchy.
    dynamic_subclass_registry<T>::get().register_subclass(class_name);

    // Register the encoders.
    mpt::for_each(Encoders{}, detail::register_encoder<T>{}, class_name);

    // Register the decoders.
    mpt::for_each(Decoders{}, detail::register_decoder<T>{}, class_name);

    return class_name;
  }

  info operator[](std::string_view class_name) const;

 private:
  arpc::flat_map<std::string_view, info> factory_function_map_;
};

using type_id = const void*;

template <typename T>
std::enable_if_t<!is_dynamic_v<T>, type_id> get_type_id() {
  static const char c = 0;
  return &c;
}

template <typename T>
std::enable_if_t<is_dynamic_v<T>, type_id> get_type_id() {
  return nullptr;
}

template <typename T>
std::enable_if_t<!is_dynamic_v<T>, bool> check_dynamic_compatibility(
    void* obj) {
  return true;
}

template <typename T>
std::enable_if_t<is_dynamic_v<T>, bool> check_dynamic_compatibility(void* obj) {
  return dynamic_subclass_registry<T>::get().is_subclass(
      (static_cast<::arpc::dynamic_base_class*>(obj))->portable_class_name());
}

}  // namespace registry

}  // namespace arpc

#endif  // ARPC_REGISTRY_H_
