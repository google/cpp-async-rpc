/// \file
/// \brief Thread subclass with a context allowing for cancellation.
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

#ifndef ASH_THREAD_H_
#define ASH_THREAD_H_

#include <thread>
#include <type_traits>
#include <utility>
#include "ash/context.h"

namespace ash {

namespace detail {
template <class T>
std::decay_t<T> decay_copy(T&& v) {
  return std::forward<T>(v);
}
}  // namespace detail

class thread : public std::thread {
 public:
  using std::thread::thread;

  template <class Function, class... Args>
  explicit thread(Function&& f, Args&&... args)
      : std::thread(),
        context_(&context::current(), context::time_point::max(), false) {
    static_cast<std::thread&>(*this) =
        std::thread(([this, f(detail::decay_copy(std::forward<Function>(f)))](
                         std::decay_t<Args>&&... args) mutable {
                      auto new_context = context_.make_child();
                      f(std::forward<std::decay_t<Args>>(args)...);
                    }),
                    std::forward<Args>(args)...);
  }

  context& get_context();

  void detach();

 private:
  context context_;
};

}  // namespace ash

#endif  // ASH_THREAD_H_
