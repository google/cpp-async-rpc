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
#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include "ash/binary_codecs.h"
#include "ash/connection.h"
#include "ash/container/flat_map.h"
#include "ash/message_defs.h"
#include "ash/mpt.h"
#include "ash/object_name.h"
#include "ash/queue.h"
#include "ash/result_holder.h"
#include "ash/semaphore.h"
#include "ash/socket.h"
#include "ash/string_adapters.h"
#include "ash/thread.h"
#include "ash/type_hash.h"
#include "ash/usage_lock.h"

namespace ash {

class listener_connection_factory : private listener {
 public:
  using connection_type = channel_connection;

  using listener::listener;

  std::unique_ptr<connection_type> operator()() {
    auto s = accept();
    return std::make_unique<connection_type>(std::move(s));
  }
};

template <typename ConnectionFactory,
          std::size_t max_connections = std::numeric_limits<std::size_t>::max()>
class connection_producer {
 public:
  using connection_type = typename ConnectionFactory::connection_type;

  template <typename... Args>
  explicit connection_producer(Args&&... args)
      : factory_(std::forward<Args>(args)...) {}

  ~connection_producer() { stop(); }

  void start() {
    std::scoped_lock lock(mu_);

    if (!acceptor_.joinable()) {
      acceptor_ = daemon_thread(&connection_producer::produce, this);
    }
  }

  void stop() {
    std::scoped_lock lock(mu_);

    if (acceptor_.joinable()) {
      acceptor_.get_context().cancel();
      acceptor_.join();
    }
  }

  awaitable<std::unique_ptr<connection_type>> get_connection() {
    return output_.get();
  }

  void return_connection(std::unique_ptr<connection_type> returned) {
    returned.reset();
    connections_given_.get();
  }

 private:
  void produce() {
    while (true) {
      connections_given_.put();
      output_.put(factory_());
    }
  }

  std::mutex mu_;
  ConnectionFactory factory_;
  semaphore connections_given_{max_connections};
  queue<std::unique_ptr<connection_type>> output_{max_connections};
  daemon_thread acceptor_;
};

using listener_connection_producer =
    connection_producer<listener_connection_factory>;

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

template <typename ConnectionProducer = listener_connection_producer,
          typename Encoder = little_endian_binary_encoder,
          typename Decoder = little_endian_binary_decoder,
          typename ObjectNameEncoder = Encoder>
class server {
 public:
 private:
  using connection_type = typename ConnectionProducer::connection_type;

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

    mpt::for_each(
        typename Interface::method_descriptors{}, [&entry, &ref](auto wrapped) {
          using method_info = typename decltype(wrapped)::type;
          constexpr auto method_hash =
              traits::type_hash_v<typename method_info::method_type>;

          entry.methods[method_fn_key{method_info::name(), method_hash}] =
              [ref](rpc_defs::request_id_type req_id, std::string request) {
                using return_type = typename method_info::return_type;
                result_holder<return_type> result;

                try {
                  // Decode the call arguments.
                  using args_tuple_type = typename method_info::args_tuple_type;
                  string_input_stream response_is(request);
                  Decoder decoder(response_is);
                  args_tuple_type args;
                  decoder(args);

                  // Call the method and set the result value.
                  if constexpr (std::is_same_v<void, return_type>) {
                    std::apply(method_info::method_ptr,
                               std::tuple_cat(std::forward_as_tuple(*ref),
                                              std::move(args)));
                    result.set_value();
                  } else {
                    result.set_value(
                        std::apply(method_info::method_ptr,
                                   std::tuple_cat(std::forward_as_tuple(*ref),
                                                  std::move(args))));
                  }
                } catch (...) {
                  // Set the return exception if either deserialization or
                  // execution threw.
                  result.set_exception(std::current_exception());
                }

                std::string response;
                string_output_stream request_os(request);
                Encoder encoder(request_os);

                // Message type: RPC request.
                encoder(rpc_defs::message_type::RESPONSE);
                // Request ID.
                encoder(req_id);
                // Result.
                encoder(result);

                return response;
              };
        });
  }

  std::string execute(std::string request) {
    string_input_stream request_is(request);
    Decoder decoder(request_is);

    // Request ID.
    rpc_defs::request_id_type req_id;
    // We just let the exception bubble if we can't deserialize even the request
    // id.
    decoder(req_id);

    try {
      // Name of the remote object.
      std::string object_name;
      decoder(object_name);
      // Method name.
      std::string method_name;
      decoder(method_name);
      // Method signature hash.
      traits::type_hash_t method_hash;
      decoder(method_hash);

      method_fn method;
      {
        std::scoped_lock lock(objects_mu_);

        // Find the object entry.
        auto oit = objects_.find(object_name);
        if (oit == objects_.end()) {
          throw errors::not_found("Object not found");
        }

        // Find the method function.
        auto mit = oit->second.methods.find({method_name, method_hash});
        if (oit == oit->second.methods.end()) {
          throw errors::not_found("Method not found in object");
        }

        method = oit->second();
      }

      // Current context.
      context ctx;
      decoder(ctx);

      // Set any timeout.
      if (options_.request_timeout) {
        ctx.set_timeout(*options_.request_timeout);
      }

      // What remains is the arguments for the actual object method. Trim the
      // request data and call the method.
      request.erase(0, request_is.pos());
      return method(req_id, std::move(request));
    } catch (...) {
      // We directly return the result here if we caught an exception before
      // getting to the object's function.
      result_holder<void> exception_result;
      exception_result.set_exception(std::current_exception());

      std::string result_str;
      string_output_stream result_is(result_str);
      Encoder encoder(result_is);

      // Message type: RPC request.
      encoder(rpc_defs::message_type::RESPONSE);
      // Request ID.
      encoder(req_id);
      // Result.
      encoder(exception_result);

      return result_str;
    }
  }

 public:
  template <typename... Args>
  explicit server(const server_options& options, Args&&... args)
      : options_(options), acceptor_(std::forward<Args>(args)...) {}

  ~server() { stop(); }

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

  void start() {
    std::scoped_lock lock(mu_);

    acceptor_.start();
  }

  void stop() {
    std::scoped_lock lock(mu_);

    acceptor_.stop();
  }

 private:
  server_options options_;
  std::mutex mu_;
  std::mutex objects_mu_;
  object_map objects_;
  ConnectionProducer acceptor_;
};

}  // namespace ash

#endif  // ASH_SERVER_H_
