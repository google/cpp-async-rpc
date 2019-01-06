/// \file
/// \brief Compile-time type hashing for automatic data schema checks.
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

#ifndef INCLUDE_ASH_TYPE_HASH_H_
#define INCLUDE_ASH_TYPE_HASH_H_

#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include "ash/mpt.h"
#include "ash/serializable_base.h"
#include "ash/traits/container_traits.h"
#include "ash/traits/serialization_traits.h"

namespace ash {

/// Trait classes for type hashing, based on 32-bit FNV-1.
namespace traits {

template <typename T, typename Seen = mpt::pack<>,
          std::uint32_t base = 2166136261, typename Enable = void>
struct type_hash;

namespace detail {

enum class type_family : std::uint8_t {
  VOID = 0,
  INTEGER,
  FLOAT,
  ENUM,
  ARRAY,
  TUPLE,
  SEQUENCE,
  SET,
  MAP,
  UNIQUE_PTR,
  SHARED_PTR,
  WEAK_PTR,
  CLASS,
  BASE_CLASS,
  FIELD,
  CUSTOM_SERIALIZATION,
  SEEN_TYPE_BACKREFERENCE
};

constexpr std::size_t FAMILY_OFFSET = 0;
constexpr std::size_t SIGN_OFFSET = 7;
constexpr std::size_t SIZE_OFFSET = 8;

constexpr std::uint32_t type_hash_compose(std::uint32_t base,
                                          std::uint32_t more) {
  return (base * 16777619) ^ more;
}

constexpr std::uint32_t type_hash_add(std::uint32_t base, type_family type,
                                      bool is_signed, std::size_t size) {
  return type_hash_compose(
      base, (static_cast<std::uint32_t>(type) << detail::FAMILY_OFFSET) |
                (static_cast<std::uint32_t>(is_signed) << detail::SIGN_OFFSET) |
                (static_cast<std::uint32_t>(size) << detail::SIZE_OFFSET));
}

template <std::uint32_t base, typename Seen, typename... T>
struct compose_with_types;

template <std::uint32_t base, typename Seen, typename... T>
struct compose_with_types<base, Seen, mpt::pack<T...>> {
  static constexpr std::uint32_t value =
      compose_with_types<base, Seen, T...>::value;
};

template <std::uint32_t base, typename Seen>
struct compose_with_types<base, Seen> {
  static constexpr std::uint32_t value = base;
};

template <std::uint32_t base, typename Seen, typename T, typename... TN>
struct compose_with_types<base, Seen, T, TN...> {
  static constexpr std::uint32_t value = compose_with_types<
      type_hash<typename std::remove_cv<
                    typename std::remove_reference<T>::type>::type,
                Seen, base>::value,
      Seen, TN...>::value;
};
}  // namespace detail

// If we see a type that we already saw before there's a cyclic reference
// somewhere,
// so insert a back-reference.
template <typename T, typename Seen, std::uint32_t base>
struct type_hash<T, Seen, base,
                 typename std::enable_if<mpt::is_in<T, Seen>::value>::type> {
  static constexpr std::uint32_t value = detail::type_hash_add(
      base, detail::type_family::SEEN_TYPE_BACKREFERENCE, false,
      mpt::head(mpt::find_if(Seen{}, mpt::is<T>{})));
};

template <typename Seen, std::uint32_t base>
struct type_hash<
    void, Seen, base,
    typename std::enable_if<!mpt::is_in<void, Seen>::value>::type> {
  static constexpr std::uint32_t value =
      detail::type_hash_add(base, detail::type_family::VOID, false, 0);
};

template <typename T, typename Seen, std::uint32_t base>
struct type_hash<T, Seen, base,
                 typename std::enable_if<std::is_integral<T>::value &&
                                         !mpt::is_in<T, Seen>::value>::type> {
  static constexpr std::uint32_t value = detail::type_hash_add(
      base, detail::type_family::INTEGER, std::is_signed<T>::value, sizeof(T));
};

template <typename T, typename Seen, std::uint32_t base>
struct type_hash<T, Seen, base,
                 typename std::enable_if<std::is_floating_point<T>::value &&
                                         !mpt::is_in<T, Seen>::value>::type> {
  static constexpr std::uint32_t value = detail::type_hash_add(
      base, detail::type_family::FLOAT, std::is_signed<T>::value, sizeof(T));
};

template <typename T, typename Seen, std::uint32_t base>
struct type_hash<T, Seen, base,
                 typename std::enable_if<std::is_enum<T>::value &&
                                         !mpt::is_in<T, Seen>::value>::type> {
  static constexpr std::uint32_t value = detail::type_hash_add(
      base, detail::type_family::ENUM, std::is_signed<T>::value, sizeof(T));
};

template <typename U, typename V, typename Seen, std::uint32_t base>
struct type_hash<
    std::pair<U, V>, Seen, base,
    typename std::enable_if<!mpt::is_in<std::pair<U, V>, Seen>::value>::type> {
  static constexpr std::uint32_t value = detail::compose_with_types<
      detail::type_hash_add(base, detail::type_family::TUPLE, false, 2),
      mpt::insert_into_t<std::pair<U, V>, Seen>, U, V>::value;
};

template <typename... T, typename Seen, std::uint32_t base>
struct type_hash<
    std::tuple<T...>, Seen, base,
    typename std::enable_if<!mpt::is_in<std::tuple<T...>, Seen>::value>::type> {
  static constexpr std::uint32_t value = detail::compose_with_types<
      detail::type_hash_add(base, detail::type_family::TUPLE, false,
                            sizeof...(T)),
      mpt::insert_into_t<std::tuple<T...>, Seen>, T...>::value;
};

template <typename T, typename Seen, std::uint32_t base>
struct type_hash<T, Seen, base,
                 typename std::enable_if<is_const_iterable<T>::value &&
                                         has_static_size<T>::value &&
                                         !mpt::is_in<T, Seen>::value>::type> {
  static constexpr std::uint32_t value = detail::compose_with_types<
      detail::type_hash_add(base, detail::type_family::ARRAY, false,
                            static_size<T>::value),
      mpt::insert_into_t<T, Seen>,
      typename std::iterator_traits<decltype(
          std::begin(std::declval<T&>()))>::value_type>::value;
};

template <typename T, typename Seen, std::uint32_t base>
struct type_hash<
    T, Seen, base,
    typename std::enable_if<
        is_const_iterable<T>::value && !has_static_size<T>::value &&
        !is_associative<T>::value && !mpt::is_in<T, Seen>::value>::type> {
  static constexpr std::uint32_t value = detail::compose_with_types<
      detail::type_hash_add(base, detail::type_family::SEQUENCE, false, 0),
      mpt::insert_into_t<T, Seen>,
      writable_value_type_t<typename T::value_type>>::value;
};

template <typename T, typename Seen, std::uint32_t base>
struct type_hash<
    T, Seen, base,
    typename std::enable_if<
        is_const_iterable<T>::value && !has_static_size<T>::value &&
        is_associative<T>::value &&
        !std::is_same<typename T::key_type, typename T::value_type>::value &&
        !mpt::is_in<T, Seen>::value>::type> {
  static constexpr std::uint32_t value = detail::compose_with_types<
      detail::type_hash_add(base, detail::type_family::MAP, false, 0),
      mpt::insert_into_t<T, Seen>,
      writable_value_type_t<typename T::value_type>>::value;
};

template <typename T, typename Seen, std::uint32_t base>
struct type_hash<
    T, Seen, base,
    typename std::enable_if<
        is_const_iterable<T>::value && !has_static_size<T>::value &&
        is_associative<T>::value &&
        std::is_same<typename T::key_type, typename T::value_type>::value &&
        !mpt::is_in<T, Seen>::value>::type> {
  static constexpr std::uint32_t value = detail::compose_with_types<
      detail::type_hash_add(base, detail::type_family::SET, false, 0),
      mpt::insert_into_t<T, Seen>,
      writable_value_type_t<typename T::value_type>>::value;
};

template <typename T, typename Seen, typename Deleter, std::uint32_t base>
struct type_hash<std::unique_ptr<T, Deleter>, Seen, base,
                 typename std::enable_if<!mpt::is_in<
                     std::unique_ptr<T, Deleter>, Seen>::value>::type> {
  static constexpr std::uint32_t value = detail::compose_with_types<
      detail::type_hash_add(base, detail::type_family::UNIQUE_PTR, false, 0),
      mpt::insert_into_t<std::unique_ptr<T, Deleter>, Seen>, T>::value;
};

template <typename T, typename Seen, std::uint32_t base>
struct type_hash<std::shared_ptr<T>, Seen, base,
                 typename std::enable_if<
                     !mpt::is_in<std::shared_ptr<T>, Seen>::value>::type> {
  static constexpr std::uint32_t value = detail::compose_with_types<
      detail::type_hash_add(base, detail::type_family::SHARED_PTR, false, 0),
      mpt::insert_into_t<std::shared_ptr<T>, Seen>, T>::value;
};

template <typename T, typename Seen, std::uint32_t base>
struct type_hash<
    std::weak_ptr<T>, Seen, base,
    typename std::enable_if<!mpt::is_in<std::weak_ptr<T>, Seen>::value>::type> {
  static constexpr std::uint32_t value = detail::compose_with_types<
      detail::type_hash_add(base, detail::type_family::WEAK_PTR, false, 0),
      mpt::insert_into_t<std::weak_ptr<T>, Seen>, T>::value;
};

template <auto mptr, typename Seen, std::uint32_t base>
struct type_hash<::ash::field_descriptor<mptr>, Seen, base, void> {
  static constexpr std::uint32_t value =
      type_hash<typename ::ash::field_descriptor<mptr>::data_type, Seen,
                base>::value;
};

template <typename T, typename Seen, std::uint32_t base>
struct type_hash<T, Seen, base,
                 typename std::enable_if<can_be_serialized<T>::value &&
                                         !mpt::is_in<T, Seen>::value>::type> {
 private:
  static constexpr std::uint32_t class_header_value = detail::type_hash_add(
      base, detail::type_family::CLASS, false,
      mpt::size<typename get_base_classes<T>::type>::value +
          mpt::size<typename get_field_descriptors<T>::type>::value +
          get_custom_serialization_version<T>::value);

  static constexpr std::uint32_t with_base_classes = detail::compose_with_types<
      detail::type_hash_add(
          class_header_value, detail::type_family::BASE_CLASS, false,
          mpt::size<typename get_base_classes<T>::type>::value),
      mpt::insert_into_t<T, Seen>, typename get_base_classes<T>::type>::value;

  static constexpr std::uint32_t with_fields = detail::compose_with_types<
      detail::type_hash_add(
          with_base_classes, detail::type_family::FIELD, false,
          mpt::size<typename get_field_descriptors<T>::type>::value),
      mpt::insert_into_t<T, Seen>,
      typename get_field_descriptors<T>::type>::value;

 public:
  static constexpr std::uint32_t value = detail::type_hash_add(
      with_fields, detail::type_family::CUSTOM_SERIALIZATION, false,
      get_custom_serialization_version<T>::value);
};
}  // namespace traits

}  // namespace ash

#endif  // INCLUDE_ASH_TYPE_HASH_H_
