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
#include <mutex>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include "ash/binary_codecs.h"
#include "ash/container/flat_map.h"
#include "ash/message_defs.h"
#include "ash/mpt.h"
#include "ash/mutex.h"
#include "ash/object_name.h"
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

template <typename Encoder = little_endian_binary_encoder,
          typename Decoder = little_endian_binary_decoder,
          typename ObjectNameEncoder = Encoder>
class server_context {
 private:
  using method_fn =
      std::function<std::string(rpc_defs::request_id_type, std::string)>;
  using method_fn_key = std::tuple<std::string, traits::type_hash_t>;
  using method_fn_map = flat_map<method_fn_key, method_fn>;

  struct object_entry {
    std::shared_ptr<void> ref;
    method_fn_map methods;
  };
  using object_map = flat_map<std::string, object_entry>;

  template <typename Interface>
  static void register_object_interface(object_entry& entry,
                                        const std::shared_ptr<Interface>& ref) {
    mpt::for_each(typename Interface::extended_interfaces{}, [&entry, &ref](
                                                                 auto wrapped) {
      using extended_interface = typename decltype(wrapped)::type;
      register_object_interface(
          entry, std::static_pointer_cast<extended_interface>(ref));
    });

    mpt::for_each(typename Interface::method_descriptors{}, [&entry, &ref](
                                                                auto wrapped) {
      using method_info = typename decltype(wrapped)::type;
      constexpr auto method_hash =
          traits::type_hash_v<typename method_info::method_type>;

      entry.methods[method_fn_key{method_info::name(), method_hash}] = nullptr;
    });
  }

 public:
  template <typename Impl, typename Name>
  void register_object(Name&& name, server_object<Impl>& object) {
    std::scoped_lock lock(objects_mu_);

    auto key = object_name<ObjectNameEncoder>(std::forward<Name>(name));
    auto& entry = objects_[key];
    entry.ref = object.get_ref();

    register_object_interface(entry, object.get_ref());
  }

  template <typename Impl>
  void unregister_object(server_object<Impl>& object) {
    std::scoped_lock lock(objects_mu_);

    auto ref = object.get_ref();
    for (auto it = objects_.begin(); it != objects_.end();) {
      if (it->second.ref == ref) {
        it = objects_.erase(it);
      } else {
        ++it;
      }
    }
  }

  template <typename Name>
  void unregister_object(Name&& name) {
    std::scoped_lock lock(objects_mu_);

    objects_.erase(object_name<ObjectNameEncoder>(std::forward<Name>(name)));
  }

 private:
  mutex objects_mu_;
  object_map objects_;
};

}  // namespace ash

#endif  // ASH_SERVER_H_
