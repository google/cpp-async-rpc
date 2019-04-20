/// \file
/// \brief Thread subclass with a context allowing for cancellation.
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

#ifndef LASR_THREAD_H_
#define LASR_THREAD_H_

#include <functional>
#include <memory>
#include <thread>
#include <type_traits>
#include <utility>

#include "lasr/context.h"
#include "lasr/errors.h"

namespace lasr {

namespace detail {
template <class T>
std::decay_t<T> decay_copy(T&& v) {
  return std::forward<T>(v);
}
}  // namespace detail

template <bool daemon>
class base_thread : public std::thread {
 public:
  using std::thread::thread;

  template <typename Function, typename... Args,
            typename = std::enable_if_t<
                !std::is_same_v<std::decay_t<Function>, base_thread>>>
  explicit base_thread(Function&& f, Args&&... args)
      : std::thread(),
        context_(std::make_unique<context>(
            daemon ? context::top() : context::current(), false)) {
    static_cast<std::thread&>(*this) = std::thread(
        ([&parent_context(*context_),
          f(detail::decay_copy(std::forward<Function>(f)))](
             std::decay_t<Args>&&... args) mutable {
          context new_context(parent_context);
          try {
            std::invoke(f, std::forward<std::decay_t<Args>>(args)...);
          } catch (const errors::cancelled&) {
          } catch (const errors::deadline_exceeded&) {
          }
        }),
        std::forward<Args>(args)...);
  }

  context& get_context() { return *context_; }

  void detach() = delete;

 private:
  std::unique_ptr<context> context_;
};

using thread = base_thread<false>;
using daemon_thread = base_thread<true>;

}  // namespace lasr

#endif  // LASR_THREAD_H_
