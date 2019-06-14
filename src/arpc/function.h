/// \file
/// \brief Helpers for behaviour function composition.
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

#ifndef ARPC_FUNCTION_H_
#define ARPC_FUNCTION_H_

#include <type_traits>
#include <utility>
#include "function2/function2.hpp"

namespace arpc {
template <typename... Args, typename OF, typename MF>
auto compose_pipe(OF&& of, MF&& mf) {
  using original_return_type = std::invoke_result_t<OF, Args...>;
  using new_return_type = typename std::conditional_t<
      std::is_same_v<original_return_type, void>, std::invoke_result<MF>,
      std::invoke_result<MF, original_return_type>>::type;

  return [mof(std::move(of)), mmf(std::move(mf))](Args&&... args) mutable {
    if constexpr (std::is_same_v<original_return_type, void>) {
      if constexpr (std::is_same_v<new_return_type, void>) {
        mof(std::forward<Args>(args)...);
        mmf();
      } else {
        mof(std::forward<Args>(args)...);
        return mmf();
      }
    } else {
      if constexpr (std::is_same_v<new_return_type, void>) {
        mmf(mof(std::forward<Args>(args)...));
      } else {
        return mmf(mof(std::forward<Args>(args)...));
      }
    }
  };
}

template <typename... Args, typename OF, typename WF>
auto compose_wrap(OF&& of, WF&& wf) {
  using new_return_type = std::invoke_result_t<WF, OF&, Args...>;

  return [mof(std::move(of)), mwf(std::move(wf))](Args&&... args) mutable {
    if constexpr (std::is_same_v<new_return_type, void>) {
      mwf(mof, std::forward<Args>(args)...);
    } else {
      return mwf(mof, std::forward<Args>(args)...);
    }
  };
}

template <typename E, typename... Args, typename OF, typename HF>
auto compose_catch(OF&& of, HF&& hf) {
  using original_return_type = std::invoke_result_t<OF, Args...>;
  using new_return_type = std::invoke_result_t<HF, const E&>;
  static_assert(std::is_same_v<original_return_type, new_return_type>,
                "Exception handlers must have the same return type as the "
                "original function");

  return [mof(std::move(of)), mhf(std::move(hf))](Args&&... args) mutable {
    if constexpr (std::is_same_v<original_return_type, void>) {
      try {
        mof(std::forward<Args>(args)...);
      } catch (const E& e) {
        mhf(e);
      }
    } else {
      try {
        return mof(std::forward<Args>(args)...);
      } catch (E& e) {
        return mhf(e);
      }
    }
  };
}
}  // namespace arpc

#endif  // ARPC_FUNCTION_H_
