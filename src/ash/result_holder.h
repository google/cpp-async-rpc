/// \file
/// \brief Class that can hold a return value (possibly void) or an exception.
///
/// \copyright
///   Copyright 2019 by Google LLC. All Rights Reserved.
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

#ifndef LASR_RESULT_HOLDER_H_
#define LASR_RESULT_HOLDER_H_

#include <algorithm>
#include <exception>
#include <optional>
#include <string>
#include <utility>
#include "ash/errors.h"
#include "ash/serializable.h"

namespace ash {

template <typename T>
class result_holder : public serializable<result_holder<T>> {
 public:
  LASR_CUSTOM_SERIALIZATION_VERSION(1);

  template <typename E>
  void save(E& e) const {
    e(value_);
    if (exception_) {
      e(true);
      auto [type, message] = error_factory::analyze_exception(exception_);
      e(type);
      e(message);
    } else {
      e(false);
    }
  }

  template <typename D>
  void load(D& d) {
    d(value_);
    bool has_exception;
    exception_ = nullptr;
    d(has_exception);
    if (has_exception) {
      std::string type, message;
      d(type);
      d(message);
      try {
        error_factory::get().throw_error(type, message.c_str());
      } catch (...) {
        exception_ = std::current_exception();
      }
    }
  }

  using value_type = T;

  result_holder() noexcept = default;
  result_holder(result_holder&&) = default;
  result_holder& operator=(result_holder&&) = default;
  explicit result_holder(std::exception_ptr exception)
      : exception_(exception) {}
  template <typename U>
  explicit result_holder(U&& u) : value_(std::move(u)) {}

  template <typename U>
  void set_value(U&& u) {
    value_ = std::forward<U>(u);
  }

  void set_exception(std::exception_ptr exception) { exception_ = exception; }

  constexpr explicit operator bool() const noexcept {
    return value_ || exception_;
  }

  constexpr bool has_value() const noexcept { return value_ || exception_; }

  const value_type* operator->() const {
    maybe_throw();
    return &(*value_);
  }

  value_type* operator->() {
    maybe_throw();
    return &(*value_);
  }

  const value_type& operator*() const& {
    maybe_throw();
    return *value_;
  }

  value_type& operator*() & {
    maybe_throw();
    return *value_;
  }

  const value_type&& operator*() const&& {
    maybe_throw();
    return std::move(*value_);
  }

  value_type&& operator*() && {
    maybe_throw();
    return std::move(*value_);
  }

  const value_type& value() const& {
    maybe_throw();
    return *value_;
  }

  value_type& value() & {
    maybe_throw();
    return *value_;
  }

  const value_type&& value() const&& {
    maybe_throw();
    return std::move(*value_);
  }

  value_type&& value() && {
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
    if (exception_) {
      std::rethrow_exception(exception_);
    }
    if (!value_) throw errors::invalid_state("Empty result holder accessed");
  }

  std::optional<value_type> value_;
  std::exception_ptr exception_;
};

template <>
class result_holder<void> : public serializable<result_holder<void>> {
 public:
  LASR_CUSTOM_SERIALIZATION_VERSION(1);

  template <typename E>
  void save(E& e) const {
    e(value_);
    if (exception_) {
      e(true);
      auto [type, message] = error_factory::analyze_exception(exception_);
      e(type);
      e(message);
    } else {
      e(false);
    }
  }

  template <typename D>
  void load(D& d) {
    d(value_);
    bool has_exception;
    exception_ = nullptr;
    d(has_exception);
    if (has_exception) {
      std::string type, message;
      d(type);
      d(message);
      try {
        error_factory::get().throw_error(type, message.c_str());
      } catch (...) {
        exception_ = std::current_exception();
      }
    }
  }

  using value_type = void;

  result_holder() noexcept = default;
  result_holder(result_holder<value_type>&&) = default;
  result_holder& operator=(result_holder<value_type>&&) = default;
  explicit result_holder(std::exception_ptr exception)
      : exception_(exception) {}

  void set_value() { value_ = true; }

  void set_exception(std::exception_ptr exception) { exception_ = exception; }

  constexpr explicit operator bool() const noexcept {
    return value_ || exception_;
  }

  void operator*() const& { maybe_throw(); }

  void operator*() & { maybe_throw(); }

  void operator*() const&& { maybe_throw(); }

  void operator*() && { maybe_throw(); }

  void value() const& { maybe_throw(); }

  void value() & { maybe_throw(); }

  void value() const&& { maybe_throw(); }

  void value() && { maybe_throw(); }

  constexpr void value_or() const& {
    if (*this) **this;
  }

  constexpr void value_or() && {
    if (*this) **this;
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
    if (exception_) {
      std::rethrow_exception(exception_);
    }
    if (!value_) throw errors::invalid_state("Empty result holder accessed");
  }

  bool value_ = false;
  std::exception_ptr exception_;
};

}  // namespace ash

#endif  // LASR_RESULT_HOLDER_H_
