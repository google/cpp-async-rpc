/// \file
/// \brief Class that can hold a return value (possibly void) or an exception.
///
/// \copyright
///   Copyright 2018 by Google Inc. All Rights Reserved.
///
/// \copyright
///   Licensed under the Apache License, Version 2.0 (the "License"); you may
///   not use this channel except in compliance with the License. You may obtain
///   a copy of the License at
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

#ifndef ASH_RESULT_HOLDER_H_
#define ASH_RESULT_HOLDER_H_

#include <algorithm>
#include <exception>
#include <optional>
#include <utility>
#include "ash/errors.h"

namespace ash {

template <typename T>
class result_holder {
 public:
  using value_type = T;

  result_holder() noexcept = default;
  result_holder(result_holder<value_type>&&) noexcept = default;
  result_holder& operator=(result_holder<value_type>&&) noexcept = default;

  template <typename U>
  void set_value(U&& u) {
    value_ = std::forward<U>(u);
  }

  void set_exception(std::exception_ptr exception) { exception_ = exception; }

  constexpr explicit operator bool() const noexcept {
    return value_ || exception_;
  }

  constexpr bool has_value() const noexcept { return value_ || exception_; }

  constexpr const value_type* operator->() const {
    maybe_throw();
    return &(*value_);
  }

  constexpr value_type* operator->() {
    maybe_throw();
    return &(*value_);
  }

  constexpr const value_type& operator*() const& {
    maybe_throw();
    return *value_;
  }

  constexpr value_type& operator*() & {
    maybe_throw();
    return *value_;
  }

  constexpr const value_type&& operator*() const&& {
    maybe_throw();
    return std::move(*value_);
  }

  constexpr value_type&& operator*() && {
    maybe_throw();
    return std::move(*value_);
  }

  constexpr const value_type& value() const& {
    maybe_throw();
    return *value_;
  }

  constexpr value_type& value() & {
    maybe_throw();
    return *value_;
  }

  constexpr const value_type&& value() const&& {
    maybe_throw();
    return std::move(*value_);
  }

  constexpr value_type&& value() && {
    maybe_throw();
    return std::move(*value_);
  }

  template <typename U>
  constexpr value_type value_or(U&& default_value) const& {
    return *this ? **this : static_cast<T>(std::forward<U>(default_value));
  }

  template <typename U>
  constexpr value_type value_or(U&& default_value) && {
    return *this ? std::move(**this)
                 : static_cast<T>(std::forward<U>(default_value));
  }

  void swap(result_holder<value_type>& other) noexcept {
    value_.swap(other.value_);
    exception_.swap(other.exception_);
  }

  friend void swap(result_holder<value_type>& a,
                   result_holder<value_type>& b) noexcept {
    a.swap(b);
  }

  void reset() noexcept {
    result_holder<value_type> dummy;
    swap(dummy);
  }

 private:
  void maybe_throw() const {
    if (exception_) std::rethrow_exception(exception_);
    if (!value_) throw errors::invalid_state("Empty result holder accessed");
  }

  std::optional<value_type> value_;
  std::exception_ptr exception_;
};

template <>
class result_holder<void> {
 public:
  using value_type = void;

  result_holder() noexcept = default;
  result_holder(result_holder<value_type>&&) noexcept = default;
  result_holder& operator=(result_holder<value_type>&&) noexcept = default;

  void set_value() { value_ = true; }

  void set_exception(std::exception_ptr exception) { exception_ = exception; }

  constexpr explicit operator bool() const noexcept {
    return value_ || exception_;
  }

  constexpr bool has_value() const noexcept { return value_ || exception_; }

  void swap(result_holder<value_type>& other) noexcept {
    std::swap(value_, other.value_);
    exception_.swap(other.exception_);
  }

  friend void swap(result_holder<value_type>& a,
                   result_holder<value_type>& b) noexcept {
    a.swap(b);
  }

  void reset() noexcept {
    result_holder<value_type> dummy;
    swap(dummy);
  }

 private:
  void maybe_throw() const {
    if (exception_) std::rethrow_exception(exception_);
    if (!value_) throw errors::invalid_state("Empty result holder accessed");
  }

  bool value_ = false;
  std::exception_ptr exception_;
};

}  // namespace ash

#endif  // ASH_RESULT_HOLDER_H_
