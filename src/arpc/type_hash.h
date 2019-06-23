/// \file
/// \brief Compile-time type hashing for automatic data schema checks.
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

#ifndef ARPC_TYPE_HASH_H_
#define ARPC_TYPE_HASH_H_

#include <array>
#include <chrono>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include "arpc/mpt.h"
#include "arpc/serializable_base.h"
#include "arpc/traits/container_traits.h"
#include "arpc/traits/serialization_traits.h"

namespace arpc {

/// Trait classes for type hashing, based on 32-bit FNV-1.
namespace traits {

using type_hash_t = std::uint32_t;

namespace detail {

enum class type_family : std::uint8_t {
  VOID = 0,
  BOOLEAN,
  CHARACTER,
  INTEGER,
  FLOAT,
  ENUM,
  ARRAY,
  TUPLE,
  SEQUENCE,
  SET,
  MAP,
  OPTIONAL,
  DURATION,
  TIME_POINT,
  UNIQUE_PTR,
  SHARED_PTR,
  WEAK_PTR,
  FUNCTION,
  CLASS,
  BASE_CLASS,
  FIELD,
  CUSTOM_SERIALIZATION,
  SEEN_TYPE_BACKREFERENCE,
};

constexpr std::size_t FAMILY_OFFSET = 0;
constexpr std::size_t SIGN_OFFSET = 7;
constexpr std::size_t SIZE_OFFSET = 8;

template <typename... Args>
constexpr type_hash_t type_hash_add(Args... args) {
  return mpt::accumulate(0, std::make_tuple(args...),
                         [](type_hash_t base, type_hash_t more) {
                           return (base * 16777619) ^ more;
                         });
}

constexpr type_hash_t type_hash_leaf(type_family type, bool is_signed,
                                     std::size_t size) {
  return type_hash_t{
      (static_cast<std::uint32_t>(type) << detail::FAMILY_OFFSET) |
      (static_cast<std::uint32_t>(is_signed) << detail::SIGN_OFFSET) |
      (static_cast<std::uint32_t>(size) << detail::SIZE_OFFSET)};
}

template <typename T, typename Seen = mpt::pack<>>
struct type_hash;

template <typename T, typename Seen = mpt::pack<>, typename Enable = void>
struct new_type_hash;

template <typename T, typename Seen = mpt::pack<>>
inline constexpr type_hash_t type_hash_v = type_hash<T, Seen>::value;

template <typename T, typename Seen>
struct type_hash {
  static constexpr type_hash_t hash() {
    using CT = traits::remove_cvref_t<T>;
    if constexpr (mpt::is_type_in_v<CT, Seen>) {
      return type_hash_leaf(
          type_family::SEEN_TYPE_BACKREFERENCE, false,
          mpt::at<0>(mpt::find_if_t<Seen, mpt::is_type<CT>>{}));
    } else {
      using NewSeen = mpt::insert_type_into_t<CT, Seen>;
      return new_type_hash<CT, NewSeen>::value;
    }
  }

  static constexpr type_hash_t value = hash();
};

template <typename T>
constexpr type_hash_t unknown_hash() {
  static_assert(std::is_same_v<T, void>, "No hash defined for type");
  return type_hash_t{0};
}

template <typename T, typename Seen, typename Enable>
struct new_type_hash {
  static constexpr type_hash_t hash() {
    if constexpr (std::is_same_v<T, void>) {
      return type_hash_leaf(type_family::VOID, false, 0);
    } else if constexpr (std::is_same_v<T, bool>) {
      return type_hash_leaf(type_family::BOOLEAN, false, 1);
    } else if constexpr (std::is_same_v<T, char>) {
      return type_hash_leaf(type_family::CHARACTER, false, 1);
    } else if constexpr (std::is_integral_v<T>) {
      return type_hash_leaf(type_family::INTEGER, std::is_signed_v<T>,
                            sizeof(T));
    } else if constexpr (std::is_floating_point_v<T>) {
      return type_hash_leaf(type_family::FLOAT, std::is_signed_v<T>, sizeof(T));
    } else if constexpr (std::is_enum_v<T>) {
      return type_hash_leaf(type_family::ENUM,
                            std::is_signed_v<std::underlying_type_t<T>>,
                            sizeof(std::underlying_type_t<T>));
    } else if constexpr (is_map_v<T>) {
      return type_hash_add(
          type_hash_leaf(detail::type_family::MAP,
                         is_multi_key_associative_v<T>, 0),
          type_hash_v<writable_value_type_t<typename T::value_type>>);
    } else if constexpr (is_set_v<T>) {
      return type_hash_add(
          type_hash_leaf(detail::type_family::SET,
                         is_multi_key_associative_v<T>, 0),
          type_hash_v<writable_value_type_t<typename T::value_type>>);
    } else if constexpr (is_const_iterable_v<T>) {
      if constexpr (has_static_size_v<T>) {
        return type_hash_add(
            type_hash_leaf(detail::type_family::ARRAY, false, static_size_v<T>),
            type_hash_v<typename std::iterator_traits<decltype(
                std::begin(std::declval<T&>()))>::value_type>);
      } else {
        return type_hash_add(
            type_hash_leaf(detail::type_family::SEQUENCE, false, 0),
            type_hash_v<writable_value_type_t<typename T::value_type>>);
      }
    } else if constexpr (can_be_serialized_v<T>) {
      return type_hash_add(
          type_hash_leaf(
              type_family::CLASS, false,
              static_cast<std::size_t>(mpt::size_v<get_base_classes_t<T>> +
                                       mpt::size_v<get_field_descriptors_t<T>> +
                                       get_custom_serialization_version_v<T>)),
          type_hash_leaf(type_family::BASE_CLASS, false,
                         mpt::size_v<get_base_classes_t<T>>),
          type_hash_v<get_base_classes_t<T>, Seen>,
          type_hash_leaf(type_family::FIELD, false,
                         mpt::size_v<get_field_descriptors_t<T>>),
          type_hash_v<get_field_descriptors_t<T>, Seen>,
          type_hash_leaf(type_family::CUSTOM_SERIALIZATION, false,
                         get_custom_serialization_version_v<T>));
    } else {
      return unknown_hash<T>();
    }
  }

  static constexpr type_hash_t value = hash();
};

template <typename... T, typename Seen>
struct new_type_hash<mpt::pack<T...>, Seen> {
  static constexpr type_hash_t value = type_hash_add(type_hash_v<T, Seen>...);
};

template <typename U, typename V, typename Seen>
struct new_type_hash<std::pair<U, V>, Seen> {
  static constexpr type_hash_t value =
      type_hash_add(type_hash_leaf(type_family::TUPLE, false, 2),
                    type_hash_v<U, Seen>, type_hash_v<V, Seen>);
};

template <typename... T, typename Seen>
struct new_type_hash<std::tuple<T...>, Seen> {
  static constexpr type_hash_t value =
      type_hash_add(type_hash_leaf(type_family::TUPLE, false, sizeof...(T)),
                    type_hash_v<T, Seen>...);
};

template <typename T, typename Seen>
struct new_type_hash<std::optional<T>, Seen> {
  static constexpr type_hash_t value = type_hash_add(
      type_hash_leaf(type_family::OPTIONAL, false, 0), type_hash_v<T, Seen>);
};

template <typename Rep, std::intmax_t num, std::intmax_t denom, typename Seen>
struct new_type_hash<std::chrono::duration<Rep, std::ratio<num, denom>>, Seen> {
  static constexpr type_hash_t value =
      type_hash_leaf(detail::type_family::DURATION, denom > num,
                     (denom > num) ? (denom - num) : (num - denom));
};

template <typename Clock, typename Duration, typename Seen>
struct new_type_hash<std::chrono::time_point<Clock, Duration>, Seen> {
  static constexpr type_hash_t value =
      type_hash_leaf(detail::type_family::TIME_POINT,
                     Duration::period::den > Duration::period::num,
                     (Duration::period::den > Duration::period::num)
                         ? (Duration::period::den - Duration::period::num)
                         : (Duration::period::num - Duration::period::den));
};

template <typename T, typename Seen, typename Deleter>
struct new_type_hash<std::unique_ptr<T, Deleter>, Seen> {
  static constexpr type_hash_t value = type_hash_add(
      type_hash_leaf(type_family::UNIQUE_PTR, false, 0), type_hash_v<T, Seen>);
};

template <typename T, typename Seen>
struct new_type_hash<std::shared_ptr<T>, Seen> {
  static constexpr type_hash_t value = type_hash_add(
      type_hash_leaf(type_family::SHARED_PTR, false, 0), type_hash_v<T, Seen>);
};

template <typename T, typename Seen>
struct new_type_hash<std::weak_ptr<T>, Seen> {
  static constexpr type_hash_t value = type_hash_add(
      type_hash_leaf(type_family::WEAK_PTR, false, 0), type_hash_v<T, Seen>);
};

template <auto mptr, typename Seen>
struct new_type_hash<::arpc::field_descriptor<mptr>, Seen> {
  static constexpr type_hash_t value =
      type_hash_v<typename ::arpc::field_descriptor<mptr>::data_type, Seen>;
};

template <typename R, typename... A, typename Seen>
struct new_type_hash<R(A...), Seen> {
  static constexpr type_hash_t value =
      type_hash_add(type_hash_leaf(type_family::FUNCTION, false, 0),
                    type_hash_v<R, Seen>, type_hash_v<A, Seen>...);
};

template <typename R, typename... A, typename Seen>
struct new_type_hash<R(A...) const, Seen> {
  static constexpr type_hash_t value =
      type_hash_add(type_hash_leaf(type_family::FUNCTION, true, 0),
                    type_hash_v<R, Seen>, type_hash_v<A, Seen>...);
};
}  // namespace detail

template <typename T, typename Seen = mpt::pack<>>
using type_hash = detail::type_hash<T, Seen>;

template <typename T>
inline constexpr type_hash_t type_hash_v = type_hash<T>::value;
}  // namespace traits

}  // namespace arpc

#endif  // ARPC_TYPE_HASH_H_
