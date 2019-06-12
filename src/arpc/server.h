/// \file
/// \brief RPC server support.
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

#ifndef ARPC_SERVER_H_
#define ARPC_SERVER_H_

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include "arpc/binary_codecs.h"
#include "arpc/connection.h"
#include "arpc/container/flat_map.h"
#include "arpc/executor.h"
#include "arpc/flag.h"
#include "arpc/future.h"
#include "arpc/message_defs.h"
#include "arpc/mpt.h"
#include "arpc/object_name.h"
#include "arpc/packet_protocols.h"
#include "arpc/queue.h"
#include "arpc/result_holder.h"
#include "arpc/select.h"
#include "arpc/semaphore.h"
#include "arpc/socket.h"
#include "arpc/string_adapters.h"
#include "arpc/thread.h"
#include "arpc/type_hash.h"
#include "arpc/usage_lock.h"

namespace arpc {

template <typename PacketProtocol>
class listener_connection_factory : private listener {
 public:
  using connection_type = packet_connection_impl<channel_connection, PacketProtocol>;

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
  explicit connection_producer(Args&&... args) : factory_(std::forward<Args>(args)...) {}

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

  awaitable<std::unique_ptr<connection_type>> get_connection() { return output_.async_get(); }

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

template <typename PacketProtocol>
using listener_connection_producer =
    connection_producer<listener_connection_factory<PacketProtocol>>;

struct server_options {
  // Timeout applied to each request (defaults to 1 hour).
  std::optional<std::chrono::milliseconds> request_timeout = std::chrono::hours(1);

  // Number of threads in the server's thread pool.
  unsigned int num_worker_threads = 2 * std::max(thread::hardware_concurrency(), 1U);

  // Queue size for the server requests (default negative for unlimited, zero for same as
  // num_worker_threads).
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
          typename PacketProtocol = protected_stream_packet_protocol<Encoder, Decoder>,
          typename ObjectNameEncoder = Encoder,
          typename ConnectionProducer = listener_connection_producer<PacketProtocol>>
class server {
 private:
  using method_fn = std::function<std::string(rpc_defs::request_id_type, std::string)>;
  using method_fn_key = std::tuple<std::string, traits::type_hash_t>;
  using method_fn_map = flat_map<method_fn_key, method_fn>;

  struct object_entry {
    std::shared_ptr<void> ref;
    method_fn_map methods;
  };
  using object_map = flat_map<std::string, object_entry>;

  using connection_key = std::size_t;
  using connection_type = typename ConnectionProducer::connection_type;
  class connection_wrapper {
   public:
    explicit connection_wrapper(server& server, connection_key key,
                                std::unique_ptr<connection_type> connection)
        : server_(server), key_(key), connection_(std::move(connection)) {
      can_receive_.set();
      can_send_.set();
    }

    awaitable<void> receive() {
      std::scoped_lock lock(mu_);
      if (failed_receive_) {
        return never();
      }

      if (!can_receive_) {
        return can_receive_.async_wait();
      }

      return connection_->data_available().then([this]() {
        can_receive_.reset();

        server_.pool_.run([this]() {
          try {
            auto request = connection_->receive();
            string_input_stream request_is(request);

            // A decoder for the header.
            Decoder header_decoder(request_is);
            rpc_defs::message_type message_type;
            header_decoder(message_type);
            rpc_defs::request_id_type req_id;
            header_decoder(req_id);

            switch (message_type) {
              case rpc_defs::message_type::REQUEST:
                request.erase(0, request_is.pos());
                server_.queue_request(request_key{key_, req_id}, request);
                break;
              case rpc_defs::message_type::CANCEL_REQUEST:
                server_.cancel_request(request_key{key_, req_id});
                break;
              default:
                // Unknown message received.
                throw errors::data_mismatch("Received unknown message type");
                break;
            }

            receive_done();
          } catch (...) {
            receive_done(true);
          }
        });
      });
    }

    awaitable<void> send() {
      std::scoped_lock lock(mu_);
      if (failed_send_) {
        return never();
      }

      if (!can_send_) {
        return can_send_.async_wait();
      }

      return responses_.async_get().then([this](std::string response) {
        can_send_.reset();

        server_.pool_.run([this, response(std::move(response))]() mutable {
          try {
            connection_->send(std::move(response));

            send_done();
          } catch (...) {
            send_done(true);
          }
        });
      });
    }

    awaitable<void> remove() {
      return can_remove_.async_wait().then([this]() {
        server_.acceptor_->return_connection(std::move(connection_));
        server_.connections_.erase(key_);
      });
    }

    void add_response(std::string response) { responses_.put(std::move(response)); }

   private:
    void receive_done(bool failed = false) {
      bool removed = false;
      {
        std::scoped_lock lock(mu_);
        if (failed) {
          failed_receive_ = true;
        }
        if (failed_send_ || (can_send_ && failed)) {
          failed_receive_ = true;
          removed = true;
        }
        can_receive_.set();
      }
      if (removed) {
        can_remove_.set();
      }
    }

    void send_done(bool failed = false) {
      bool removed = false;
      {
        std::scoped_lock lock(mu_);
        if (failed) {
          failed_send_ = true;
        }
        if (failed_receive_ || (can_receive_ && failed)) {
          failed_send_ = true;
          removed = true;
        }
        can_send_.set();
      }
      if (removed) {
        can_remove_.set();
      }
    }

    server& server_;
    connection_key key_;
    std::mutex mu_;
    std::unique_ptr<connection_type> connection_;
    queue<std::string> responses_;
    bool failed_receive_ = false, failed_send_ = false;
    flag can_receive_, can_send_, can_remove_;
  };

  using connection_map = flat_map<connection_key, std::unique_ptr<connection_wrapper>>;

  using request_key = std::pair<connection_key, rpc_defs::request_id_type>;
  using request_result = std::pair<request_key, std::string>;
  class request_wrapper {
   public:
    explicit request_wrapper(server& server, future<request_result> result)
        : server_(server), result_(std::move(result)), context_(context::top(), false) {}

    context& get_context() { return context_; }

    awaitable<void> finish() {
      return result_.async_get().then([this](request_result result) {
        auto& [request_key, data] = result;
        auto cit = server_.connections_.find(request_key.first);
        if (cit != server_.connections_.end()) {
          cit->second->add_response(std::move(data));
        }

        server_.remove_request(request_key);
      });
    }

   private:
    server& server_;
    future<request_result> result_;
    context context_;
  };
  using request_map = flat_map<request_key, std::unique_ptr<request_wrapper>>;

  void cancel_request(const request_key& key) {
    std::scoped_lock lock(requests_mu_);

    auto it = requests_.find(key);
    if (it != requests_.end()) {
      it->second->get_context().cancel();
    }
  }

  void remove_request(const request_key& key) {
    std::scoped_lock lock(requests_mu_);

    requests_.erase(key);
  }

  void queue_request(const request_key& key, std::string request) {
    std::scoped_lock lock(requests_mu_);

    if (requests_.count(key) > 0) {
      // A previous request with the same key was already registered... dupe?
      // Just drop the new one.
      return;
    }

    promise<request_result> promise;
    auto wrapper_ptr = std::make_unique<request_wrapper>(*this, promise.get_future());

    pool_.run([this, promise(std::move(promise)), key, request(std::move(request)),
               &parent_context(wrapper_ptr->get_context())]() mutable {
      try {
        context ctx(parent_context);
        promise.set_value(request_result{key, std::move(execute(key.second, std::move(request)))});
      } catch (...) {
        promise.set_exception(std::current_exception());
      }
    });

    requests_.insert({key, std::move(wrapper_ptr)});
  }

  template <typename Interface>
  static void register_object_interface(object_entry& entry,
                                        const std::shared_ptr<Interface>& ref) {
    mpt::for_each(typename Interface::extended_interfaces{}, [&entry, &ref](auto wrapped) {
      using extended_interface = typename decltype(wrapped)::type;
      register_object_interface(entry, std::static_pointer_cast<extended_interface>(ref));
    });

    mpt::for_each(typename Interface::method_descriptors{}, [&entry, &ref](auto wrapped) {
      using method_info = typename decltype(wrapped)::type;
      constexpr auto method_hash = traits::type_hash_v<typename method_info::method_type>;

      entry.methods[method_fn_key{method_info::name(), method_hash}] =
          [ref](rpc_defs::request_id_type req_id, std::string request) {
            using return_type = typename method_info::return_type;
            result_holder<return_type> result;

            try {
              // Decode the call arguments.
              using args_tuple_type = typename method_info::args_tuple_type;
              string_input_stream request_is(request);

              // A decoder for the arguments.
              Decoder args_decoder(request_is);
              args_tuple_type args;
              args_decoder(args);

              // Call the method and set the result value.
              if constexpr (std::is_same_v<void, return_type>) {
                std::apply(method_info::method_ptr,
                           std::tuple_cat(std::forward_as_tuple(*ref), std::move(args)));
                result.set_value();
              } else {
                result.set_value(
                    std::apply(method_info::method_ptr,
                               std::tuple_cat(std::forward_as_tuple(*ref), std::move(args))));
              }
            } catch (...) {
              // Set the return exception if either deserialization or
              // execution threw.
              result.set_exception(std::current_exception());
            }

            std::string response;
            string_output_stream request_os(response);

            {
              // An encoder for the header.
              Encoder header_encoder(request_os);
              // Message type: RPC request.
              header_encoder(rpc_defs::message_type::RESPONSE);
              // Request ID.
              header_encoder(req_id);
            }

            {
              // An encoder for the result.
              Encoder result_encoder(request_os);
              // Result.
              result_encoder(result);
            }

            return response;
          };
    });
  }

  std::string execute(rpc_defs::request_id_type req_id, std::string request) {
    try {
      string_input_stream request_is(request);

      // A decoder for the method id and the context.
      Decoder method_decoder(request_is);
      // Name of the remote object.
      std::string object_name;
      method_decoder(object_name);
      // Method name.
      std::string method_name;
      method_decoder(method_name);
      // Method signature hash.
      traits::type_hash_t method_hash;
      method_decoder(method_hash);

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
        if (mit == oit->second.methods.end()) {
          throw errors::not_found("Method not found in object");
        }

        method = mit->second;
      }

      // Current context.
      context ctx;
      method_decoder(ctx);

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

      {
        // A encoder for the header.
        Encoder header_encoder(result_is);
        // Message type: RPC request.
        header_encoder(rpc_defs::message_type::RESPONSE);
        // Request ID.
        header_encoder(req_id);
      }

      {
        // A encoder for the result.
        Encoder result_encoder(result_is);
        // Result.
        result_encoder(exception_result);
      }

      return result_str;
    }
  }

  awaitable<void> get_new_connection() {
    return acceptor_->get_connection().then(
        [this](std::unique_ptr<connection_type> new_connection) {
          while (connections_.count(++next_connection_key_) > 0) {
            // Ensure we prevent connection counter cycle collisions.
          }
          connections_.insert(
              {next_connection_key_, std::make_unique<connection_wrapper>(
                                         *this, next_connection_key_, std::move(new_connection))});
        });
  }

  void react() {
    std::vector<awaitable<void>> awaitables;
    while (true) {
      awaitables.clear();

      awaitables.push_back(get_new_connection());

      for (auto& cit : connections_) {
        auto& wrapper = cit.second;
        awaitables.push_back(wrapper->receive());
        awaitables.push_back(wrapper->send());
        awaitables.push_back(wrapper->remove());
      }
      {
        std::scoped_lock lock(requests_mu_);

        for (auto& rit : requests_) {
          auto& wrapper = rit.second;
          awaitables.push_back(wrapper->finish());
        }
      }

      auto [events] = select(awaitables);

      for (auto& event : events) {
        if (event) {
          *event;
        }
      }
    }
  }

 public:
  template <typename... Args>
  explicit server(const server_options& options, Args&&... args)
      : options_(options),
        acceptor_factory_([args_tuple = std::make_tuple(std::forward<Args>(args)...), this]() {
          return std::apply([this](const auto&... args) { acceptor_.emplace(args...); },
                            args_tuple);
        }),
        pool_(options_.num_worker_threads, options_.queue_size) {}

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

    if (!acceptor_) {
      acceptor_factory_();
    }

    if (!reactor_.joinable()) {
      reactor_ = daemon_thread(&server::react, this);
    }

    acceptor_->start();
  }

  void stop() {
    std::scoped_lock lock(mu_);

    if (acceptor_) {
      acceptor_->stop();
    }

    if (reactor_.joinable()) {
      reactor_.get_context().cancel();
      reactor_.join();
    }

    connections_.clear();

    acceptor_.reset();
  }

 private:
  server_options options_;
  std::mutex mu_;
  std::mutex objects_mu_;
  object_map objects_;
  std::optional<ConnectionProducer> acceptor_;
  std::function<void()> acceptor_factory_;
  connection_key next_connection_key_ = 0;
  connection_map connections_;
  std::mutex requests_mu_;
  request_map requests_;
  daemon_thread reactor_;
  thread_pool pool_;
};

}  // namespace arpc

#endif  // ARPC_SERVER_H_
