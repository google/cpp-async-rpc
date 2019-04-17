/// \file
/// \brief RPC server support.
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

#ifndef ASH_SERVER_H_
#define ASH_SERVER_H_

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include "ash/container/flat_map.h"
#include "ash/mpt.h"
#include "ash/thread.h"
#include "ash/type_hash.h"
#include "ash/usage_lock.h"

namespace ash {

struct server_options {
  // Timeout applied to each request (defaults to 1 hour).
  std::optional<std::chrono::milliseconds> request_timeout =
      std::chrono::hours(1);

  // Number of threads in the server's thread pool.
  unsigned int num_worker_threads = 2 * thread::hardware_concurrency();

  // Queue size for the server requests (default negative for unlimited).
  int queue_size = -1;
};

template <typename ImplClass>
class server_object : public ImplClass {
 public:
  using ImplClass::ImplClass;

  std::shared_ptr<ImplClass> get_ref() { return usage_lock_.get(); }

 private:
  usage_lock<ImplClass> usage_lock_{this};
};

class server_context {
 private:
  template <typename Interface>
  void register_object_interface(std::string_view name,
                                 const std::shared_ptr<Interface>& ref) {
    mpt::for_each(typename Interface::extended_interfaces{}, [this, name, &ref](
                                                                 auto wrapped) {
      using extended_interface = typename decltype(wrapped)::type;
      register_object_interface(
          name, std::static_pointer_cast<extended_interface>(ref));
    });

    mpt::for_each(typename Interface::method_descriptors{}, [this, name, &ref](
                                                                auto wrapped) {
      using method_info = typename decltype(wrapped)::type;
      constexpr auto method_hash =
          traits::type_hash_v<typename method_info::method_type>;

      object_fns_[object_fn_key{name, method_info::name(), method_hash}] =
          nullptr;
    });
  }

  template <typename Interface>
  void unregister_object_interface(std::string_view name,
                                   const std::shared_ptr<Interface>& ref) {
    mpt::for_each(typename Interface::extended_interfaces{}, [this, name, &ref](
                                                                 auto wrapped) {
      using extended_interface = typename decltype(wrapped)::type;
      unregister_object_interface(
          name, std::static_pointer_cast<extended_interface>(ref));
    });

    mpt::for_each(typename Interface::method_descriptors{}, [this, name, &ref](
                                                                auto wrapped) {
      using method_info = typename decltype(wrapped)::type;
      constexpr auto method_hash =
          traits::type_hash_v<typename method_info::method_type>;

      object_fns_.erase(object_fn_key{name, method_info::name(), method_hash});
    });
  }

 public:
  template <typename Impl>
  void register_object(std::string_view name, server_object<Impl>& object) {
    register_object_interface(name, object.get_ref());
  }

  template <typename Impl>
  void unregister_object(std::string_view name, server_object<Impl>& object) {
    unregister_object_interface(name, object.get_ref());
  }

 private:
  using object_fn = std::function<std::string(std::string)>;
  using object_fn_key =
      std::tuple<std::string, std::string, traits::type_hash_t>;
  using object_fn_map = flat_map<object_fn_key, object_fn>;

  object_fn_map object_fns_;
};

}  // namespace ash

#endif  // ASH_SERVER_H_
