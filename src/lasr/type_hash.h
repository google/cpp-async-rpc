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

#ifndef LASR_TYPE_HASH_H_
#define LASR_TYPE_HASH_H_

#include <array>
#include <chrono>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include "lasr/mpt.h"
#include "lasr/serializable_base.h"
#include "lasr/traits/container_traits.h"
#include "lasr/traits/serialization_traits.h"

namespace lasr {

/// Trait classes for type hashing, based on 32-bit FNV-1.
namespace traits {

using type_hash_t = std::uint32_t;

template <typename T, typename Seen = mpt::pack<>,
          type_hash_t base = 2166136261, typename Enable = void>
struct type_hash;

namespace detail {

enum class type_family : std::uint8_t {
  VOID = 0,
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
  CLASS,
  BASE_CLASS,
  FIELD,
  CUSTOM_SERIALIZATION,
  SEEN_TYPE_BACKREFERENCE,
  FUNCTION,
};

constexpr std::size_t FAMILY_OFFSET = 0;
constexpr std::size_t SIGN_OFFSET = 7;
constexpr std::size_t SIZE_OFFSET = 8;

constexpr type_hash_t type_hash_compose(type_hash_t base, type_hash_t more) {
  return (base * 16777619) ^ more;
}

constexpr type_hash_t type_hash_add(type_hash_t base, type_family type,
                                    bool is_signed, std::size_t size) {
  return type_hash_compose(
      base, (static_cast<type_hash_t>(type) << detail::FAMILY_OFFSET) |
                (static_cast<type_hash_t>(is_signed) << detail::SIGN_OFFSET) |
                (static_cast<type_hash_t>(size) << detail::SIZE_OFFSET));
}

template <type_hash_t base, typename Seen, typename... T>
struct compose_with_types;

template <type_hash_t base, typename Seen, typename... T>
struct compose_with_types<base, Seen, mpt::pack<T...>> {
  static constexpr type_hash_t value =
      compose_with_types<base, Seen, T...>::value;
};

template <type_hash_t base, typename Seen>
struct compose_with_types<base, Seen> {
  static constexpr type_hash_t value = base;
};

template <type_hash_t base, typename Seen, typename T, typename... TN>
struct compose_with_types<base, Seen, T, TN...> {
  static constexpr type_hash_t value =
      compose_with_types<type_hash<std::remove_cv_t<std::remove_reference_t<T>>,
                                   Seen, base>::value,
                         Seen, TN...>::value;
};

}  // namespace detail

// If we see a type that we already saw before there's a cyclic reference
// somewhere,
// so insert a back-reference.
template <typename T, typename Seen, type_hash_t base>
struct type_hash<T, Seen, base, std::enable_if_t<mpt::is_type_in_v<T, Seen>>> {
  static constexpr type_hash_t value = detail::type_hash_add(
      base, detail::type_family::SEEN_TYPE_BACKREFERENCE, false,
      mpt::at<0>(mpt::find_if_t<Seen, mpt::is_type<T>>{}));
};

template <typename Seen, type_hash_t base>
struct type_hash<void, Seen, base,
                 std::enable_if_t<!mpt::is_type_in_v<void, Seen>>> {
  static constexpr type_hash_t value =
      detail::type_hash_add(base, detail::type_family::VOID, false, 0);
};

template <typename T, typename Seen, type_hash_t base>
struct type_hash<
    T, Seen, base,
    std::enable_if_t<!std::is_same_v<T, char> && std::is_integral_v<T> &&
                     !mpt::is_type_in_v<T, Seen>>> {
  static constexpr type_hash_t value = detail::type_hash_add(
      base, detail::type_family::INTEGER, std::is_signed_v<T>, sizeof(T));
};

// Special-case char, which can differ in signedness across architectures.
template <typename Seen, type_hash_t base>
struct type_hash<char, Seen, base,
                 std::enable_if_t<!mpt::is_type_in_v<char, Seen>>> {
  static constexpr type_hash_t value =
      detail::type_hash_add(base, detail::type_family::CHARACTER, false, 1);
};

template <typename T, typename Seen, type_hash_t base>
struct type_hash<T, Seen, base,
                 std::enable_if_t<std::is_floating_point_v<T> &&
                                  !mpt::is_type_in_v<T, Seen>>> {
  static constexpr type_hash_t value = detail::type_hash_add(
      base, detail::type_family::FLOAT, std::is_signed_v<T>, sizeof(T));
};

template <typename T, typename Seen, type_hash_t base>
struct type_hash<
    T, Seen, base,
    std::enable_if_t<std::is_enum_v<T> && !mpt::is_type_in_v<T, Seen>>> {
  static constexpr type_hash_t value = detail::type_hash_add(
      base, detail::type_family::ENUM, std::is_signed_v<T>, sizeof(T));
};

template <typename U, typename V, typename Seen, type_hash_t base>
struct type_hash<std::pair<U, V>, Seen, base,
                 std::enable_if_t<!mpt::is_type_in_v<std::pair<U, V>, Seen>>> {
  static constexpr type_hash_t value = detail::compose_with_types<
      detail::type_hash_add(base, detail::type_family::TUPLE, false, 2),
      mpt::insert_type_into_t<std::pair<U, V>, Seen>, U, V>::value;
};

template <typename... T, typename Seen, type_hash_t base>
struct type_hash<std::tuple<T...>, Seen, base,
                 std::enable_if_t<!mpt::is_type_in_v<std::tuple<T...>, Seen>>> {
  static constexpr type_hash_t value = detail::compose_with_types<
      detail::type_hash_add(base, detail::type_family::TUPLE, false,
                            sizeof...(T)),
      mpt::insert_type_into_t<std::tuple<T...>, Seen>, T...>::value;
};

template <typename T, typename Seen, type_hash_t base>
struct type_hash<
    T, Seen, base,
    std::enable_if_t<is_const_iterable_v<T> && has_static_size_v<T> &&
                     !mpt::is_type_in_v<T, Seen>>> {
  static constexpr type_hash_t value = detail::compose_with_types<
      detail::type_hash_add(base, detail::type_family::ARRAY, false,
                            static_size<T>::value),
      mpt::insert_type_into_t<T, Seen>,
      typename std::iterator_traits<decltype(
          std::begin(std::declval<T&>()))>::value_type>::value;
};

template <typename T, typename Seen, type_hash_t base>
struct type_hash<
    T, Seen, base,
    std::enable_if_t<is_const_iterable_v<T> && !has_static_size_v<T> &&
                     !is_associative_v<T> && !mpt::is_type_in_v<T, Seen>>> {
  static constexpr type_hash_t value = detail::compose_with_types<
      detail::type_hash_add(base, detail::type_family::SEQUENCE, false, 0),
      mpt::insert_type_into_t<T, Seen>,
      writable_value_type_t<typename T::value_type>>::value;
};

template <typename T, typename Seen, type_hash_t base>
struct type_hash<
    T, Seen, base,
    std::enable_if_t<
        is_const_iterable_v<T> && !has_static_size_v<T> &&
        is_associative_v<T> &&
        !std::is_same_v<typename T::key_type, typename T::value_type> &&
        !mpt::is_type_in_v<T, Seen>>> {
  static constexpr type_hash_t value = detail::compose_with_types<
      detail::type_hash_add(base, detail::type_family::MAP, false, 0),
      mpt::insert_type_into_t<T, Seen>,
      writable_value_type_t<typename T::value_type>>::value;
};

template <typename T, typename Seen, type_hash_t base>
struct type_hash<
    T, Seen, base,
    std::enable_if_t<
        is_const_iterable_v<T> && !has_static_size_v<T> &&
        is_associative_v<T> &&
        std::is_same_v<typename T::key_type, typename T::value_type> &&
        !mpt::is_type_in_v<T, Seen>>> {
  static constexpr type_hash_t value = detail::compose_with_types<
      detail::type_hash_add(base, detail::type_family::SET, false, 0),
      mpt::insert_type_into_t<T, Seen>,
      writable_value_type_t<typename T::value_type>>::value;
};

template <typename T, typename Seen, type_hash_t base>
struct type_hash<std::optional<T>, Seen, base,
                 std::enable_if_t<!mpt::is_type_in_v<std::optional<T>, Seen>>> {
  static constexpr type_hash_t value = detail::compose_with_types<
      detail::type_hash_add(base, detail::type_family::OPTIONAL, false, 0),
      mpt::insert_type_into_t<std::optional<T>, Seen>, T>::value;
};

template <typename Rep, std::intmax_t num, std::intmax_t denom, typename Seen,
          type_hash_t base>
struct type_hash<
    std::chrono::duration<Rep, std::ratio<num, denom>>, Seen, base,
    std::enable_if_t<!mpt::is_type_in_v<
        std::chrono::duration<Rep, std::ratio<num, denom>>, Seen>>> {
  static constexpr type_hash_t value =
      detail::type_hash_add(base, detail::type_family::DURATION, denom > num,
                            (denom > num) ? (denom - num) : (num - denom));
};

template <typename Clock, typename Duration, typename Seen, type_hash_t base>
struct type_hash<std::chrono::time_point<Clock, Duration>, Seen, base,
                 std::enable_if_t<!mpt::is_type_in_v<
                     std::chrono::time_point<Clock, Duration>, Seen>>> {
  static constexpr type_hash_t value = detail::type_hash_add(
      base, detail::type_family::TIME_POINT,
      Duration::period::den > Duration::period::num,
      (Duration::period::den > Duration::period::num)
          ? (Duration::period::den - Duration::period::num)
          : (Duration::period::num - Duration::period::den));
};

template <typename T, typename Seen, typename Deleter, type_hash_t base>
struct type_hash<
    std::unique_ptr<T, Deleter>, Seen, base,
    std::enable_if_t<!mpt::is_type_in_v<std::unique_ptr<T, Deleter>, Seen>>> {
  static constexpr type_hash_t value = detail::compose_with_types<
      detail::type_hash_add(base, detail::type_family::UNIQUE_PTR, false, 0),
      mpt::insert_type_into_t<std::unique_ptr<T, Deleter>, Seen>, T>::value;
};

template <typename T, typename Seen, type_hash_t base>
struct type_hash<
    std::shared_ptr<T>, Seen, base,
    std::enable_if_t<!mpt::is_type_in_v<std::shared_ptr<T>, Seen>>> {
  static constexpr type_hash_t value = detail::compose_with_types<
      detail::type_hash_add(base, detail::type_family::SHARED_PTR, false, 0),
      mpt::insert_type_into_t<std::shared_ptr<T>, Seen>, T>::value;
};

template <typename T, typename Seen, type_hash_t base>
struct type_hash<std::weak_ptr<T>, Seen, base,
                 std::enable_if_t<!mpt::is_type_in_v<std::weak_ptr<T>, Seen>>> {
  static constexpr type_hash_t value = detail::compose_with_types<
      detail::type_hash_add(base, detail::type_family::WEAK_PTR, false, 0),
      mpt::insert_type_into_t<std::weak_ptr<T>, Seen>, T>::value;
};

template <auto mptr, typename Seen, type_hash_t base>
struct type_hash<::lasr::field_descriptor<mptr>, Seen, base, void> {
  static constexpr type_hash_t value =
      type_hash<typename ::lasr::field_descriptor<mptr>::data_type, Seen,
                base>::value;
};

template <typename T, typename Seen, type_hash_t base>
struct type_hash<
    T, Seen, base,
    std::enable_if_t<can_be_serialized_v<T> && !mpt::is_type_in_v<T, Seen>>> {
 private:
  static constexpr type_hash_t class_header_value =
      detail::type_hash_add(base, detail::type_family::CLASS, false,
                            mpt::size_v<get_base_classes_t<T>> +
                                mpt::size_v<get_field_descriptors_t<T>> +
                                get_custom_serialization_version_v<T>);

  static constexpr type_hash_t with_base_classes = detail::compose_with_types<
      detail::type_hash_add(class_header_value, detail::type_family::BASE_CLASS,
                            false, mpt::size_v<get_base_classes_t<T>>),
      mpt::insert_type_into_t<T, Seen>, get_base_classes_t<T>>::value;

  static constexpr type_hash_t with_fields = detail::compose_with_types<
      detail::type_hash_add(with_base_classes, detail::type_family::FIELD,
                            false, mpt::size_v<get_field_descriptors_t<T>>),
      mpt::insert_type_into_t<T, Seen>, get_field_descriptors_t<T>>::value;

 public:
  static constexpr type_hash_t value = detail::type_hash_add(
      with_fields, detail::type_family::CUSTOM_SERIALIZATION, false,
      get_custom_serialization_version_v<T>);
};

template <typename R, typename... A, typename Seen, type_hash_t base>
struct type_hash<R(A...), Seen, base,
                 std::enable_if_t<!mpt::is_type_in_v<R(A...), Seen>>> {
  static constexpr type_hash_t value = detail::compose_with_types<
      detail::type_hash_add(base, detail::type_family::FUNCTION, false, 0),
      mpt::insert_type_into_t<R(A...), Seen>, R, A...>::value;
};

template <typename R, typename... A, typename Seen, type_hash_t base>
struct type_hash<R(A...) const, Seen, base,
                 std::enable_if_t<!mpt::is_type_in_v<R(A...) const, Seen>>> {
  static constexpr type_hash_t value = detail::compose_with_types<
      detail::type_hash_add(base, detail::type_family::FUNCTION, true, 0),
      mpt::insert_type_into_t<R(A...) const, Seen>, R, A...>::value;
};

template <typename T>
inline constexpr type_hash_t type_hash_v = type_hash<T>::value;
}  // namespace traits

}  // namespace lasr

#endif  // LASR_TYPE_HASH_H_
