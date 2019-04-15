/// \file
/// \brief RPC client support.
///
/// \copyright
///   Copyright 2018 by Google Inc. All Rights Reserved.
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

#ifndef ASH_CLIENT_H_
#define ASH_CLIENT_H_

#include <chrono>
#include <exception>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include "ash/binary_codecs.h"
#include "ash/connection.h"
#include "ash/container/flat_map.h"
#include "ash/context.h"
#include "ash/errors.h"
#include "ash/flag.h"
#include "ash/future.h"
#include "ash/interface.h"
#include "ash/mutex.h"
#include "ash/object_name.h"
#include "ash/packet_protocols.h"
#include "ash/result_holder.h"
#include "ash/select.h"
#include "ash/string_adapters.h"
#include "ash/thread.h"
#include "ash/traits/type_traits.h"
#include "ash/type_hash.h"

namespace ash {

struct client_options {
  // Deadline applied to each request (default to 1 hour).
  std::optional<std::chrono::milliseconds> request_deadline =
      std::chrono::hours(1);
};

template <typename Connection = client_socket_connection,
          typename Encoder = little_endian_binary_encoder,
          typename Decoder = little_endian_binary_decoder,
          typename PacketProtocol =
              protected_stream_packet_protocol<Encoder, Decoder>,
          typename ObjectNameEncoder = Encoder>
class client_connection {
 private:
  struct remote_object {
    remote_object(client_connection& connection, std::string_view name,
                  const client_options& options = {})
        : connection_(connection), name_(name), options_(options) {}

    template <auto mptr, typename... A>
    future<typename traits::member_function_pointer_traits<mptr>::return_type>
    async_call(A&&... args) {
      // Propagate any timeout in client_options.
      context ctx;
      if (options_.request_deadline) {
        ctx.set_timeout(*options_.request_deadline);
      }

      using return_type =
          typename traits::member_function_pointer_traits<mptr>::return_type;
      using method_type =
          typename traits::member_function_pointer_traits<mptr>::method_type;

      // Get a serializable tuple with the args.
      using args_ref_tuple_type =
          typename traits::member_function_pointer_traits<
              mptr>::args_ref_tuple_type;
      args_ref_tuple_type args_refs(std::forward<A>(args)...);

      // Serialize to a string.
      std::string request;
      string_output_stream request_os(request);
      Encoder encoder(request_os);

      // Get the placeholder for the request.
      auto [req_id, response_future] = connection_.start_request(encoder);
      try {
        // Name of the remote object.
        encoder(name_);
        // Method name.
        using method_descriptor = method_descriptor<mptr>;
        encoder(method_descriptor::name());
        // Method signature hash.
        constexpr auto method_hash = traits::type_hash_v<method_type>;
        encoder(method_hash);
        // Actual arguments.
        encoder(args_refs);
        // Current context.
        encoder(context::current());

        // Send the request.
        connection_.send(std::move(request));

        return response_future.then([](std::string response) {
          string_input_stream response_is(response);
          Decoder decoder(response_is);
          result_holder<return_type> result;
          decoder(result);
          return std::move(result).value();
        });
      } catch (...) {
        connection_.cancel_request(req_id);
        throw;
      }
    }

    template <auto mptr, typename... A>
    typename traits::member_function_pointer_traits<mptr>::return_type call(
        A&&... args) {
      return async_call<mptr>(std::forward<A>(args)...).get();
    }

    client_connection& connection_;
    const std::string name_;
    client_options options_;
  };

  using connection_type =
      ash::packet_connection_impl<ash::reconnectable_connection<Connection>,
                                  PacketProtocol>;

 public:
  template <typename... Args>
  explicit client_connection(Args&&... args)
      : sequence_(0),
        connection_(std::forward<Args>(args)...),
        receiver_(&client_connection::receive, this) {}

  ~client_connection() {
    receiver_.get_context().cancel();
    connection_.disconnect();
    receiver_.join();
  }

  template <typename I, typename N>
  auto get_proxy(N&& name) {
    return I::make_proxy(remote_object(
        *this, object_name<ObjectNameEncoder>(std::forward<N>(name))));
  }

 private:
  struct pending_request {
    std::optional<context::time_point> deadline;
    promise<std::string> result;
  };
  using pending_map_type = flat_map<std::uint32_t, pending_request>;
  using sequence_type = std::uint32_t;

  void maybe_gc() {
    if (++gc_count_ >= pending_.size()) {
      gc_count_ = 0;
      auto now = std::chrono::system_clock::now();
      auto it = pending_.begin();
      while (it != pending_.end()) {
        if (it->second.deadline && *(it->second.deadline) < now) {
          it->second.result.set_exception(std::make_exception_ptr(
              errors::deadline_exceeded("Request timed out")));
          it = pending_.erase(it);
        } else {
          ++it;
        }
      }
    }
  }

  std::pair<sequence_type, future<std::string>> start_request(Encoder& e) {
    std::scoped_lock lock(pending_mu_);
    maybe_gc();

    auto req_id = sequence_++;
    auto& pending = pending_[req_id];
    pending.deadline = context::current().deadline();
    auto result = pending.result.get_future();
    e(req_id);
    return {req_id, std::move(result)};
  }

  void cancel_request(sequence_type req_id) {
    std::scoped_lock lock(pending_mu_);
    auto it = pending_.find(req_id);
    if (it != pending_.end()) {
      it->second.result.set_exception(
          std::make_exception_ptr(errors::cancelled("Request cancelled")));
      pending_.erase(it);
    }
  }

  void set_response(sequence_type req_id, std::string response) {
    std::scoped_lock lock(pending_mu_);
    auto it = pending_.find(req_id);
    if (it != pending_.end()) {
      it->second.result.set_value(std::move(response));
      pending_.erase(it);
    }
  }

  void broadcast_exception(std::exception_ptr exc) {
    std::scoped_lock lock(pending_mu_);
    // Report the exception for all pending requests and unqueue them.
    for (auto& p : pending_) {
      p.second.result.set_exception(exc);
    }
    pending_.clear();
  }

  void send(std::string request) {
    {
      std::scoped_lock lock(sending_mu_);
      try {
        connection_.connect();
        connection_.send(std::move(request));
        ready_.set();
      } catch (...) {
        // Disconnect the connection.
        ready_.reset();
        connection_.disconnect();
        throw;
      }
    }
  }

  void receive() {
    while (true) {
      // Wait until we actually need to receive something.
      select(ready_.wait_set());

      try {
        while (true) {
          // Read the next packet.
          auto response = connection_.receive();

          // Decode the request id.
          string_input_stream response_is(response);
          Decoder decoder(response_is);
          sequence_type req_id;
          decoder(req_id);
          response.erase(0, response_is.pos());

          set_response(req_id, response);
        }
      } catch (...) {
        // Prevent sending more requests while we disconnect.
        std::scoped_lock lock(sending_mu_);

        // Disconnect the connection.
        ready_.reset();
        connection_.disconnect();

        // Broadcast the exception to all the pending requests.
        auto exc = std::current_exception();
        broadcast_exception(exc);
      }
    }
  }

  mutex pending_mu_, sending_mu_;
  sequence_type sequence_;
  flag ready_;
  connection_type connection_;
  pending_map_type pending_;
  typename pending_map_type::size_type gc_count_ = 0;
  ash::daemon_thread receiver_;
};

}  // namespace ash

#endif  // ASH_CLIENT_H_
