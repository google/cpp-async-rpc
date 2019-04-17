/// \file
/// \brief RPC client support.
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

#ifndef ASH_CLIENT_H_
#define ASH_CLIENT_H_

#include <algorithm>
#include <chrono>
#include <cstddef>
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
#include "ash/message_defs.h"
#include "ash/mutex.h"
#include "ash/object_name.h"
#include "ash/packet_protocols.h"
#include "ash/queue.h"
#include "ash/result_holder.h"
#include "ash/select.h"
#include "ash/string_adapters.h"
#include "ash/thread.h"
#include "ash/traits/type_traits.h"
#include "ash/type_hash.h"

namespace ash {

struct client_options {
  // Timeout applied to each request (default to 1 hour).
  std::optional<std::chrono::milliseconds> request_timeout =
      std::chrono::hours(1);
};

template <typename Connection = client_socket_connection,
          typename Encoder = little_endian_binary_encoder,
          typename Decoder = little_endian_binary_decoder,
          typename PacketProtocol =
              protected_stream_packet_protocol<Encoder, Decoder>,
          typename ObjectNameEncoder = Encoder,
          std::size_t max_queued_events = 256>
class client_connection {
 private:
  struct remote_object {
    remote_object(client_connection& connection, std::string_view name,
                  const client_options& options = {})
        : connection_(connection), name_(name), options_(options) {}

    template <auto mptr, typename... A>
    std::pair<future<typename traits::member_function_pointer_traits<
                  mptr>::return_type>,
              rpc_defs::request_id_type>
    async_call(A&&... args) {
      // Propagate any timeout in client_options.
      context ctx;
      if (options_.request_timeout) {
        ctx.set_timeout(*options_.request_timeout);
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

      // Get the placeholder for the request.
      auto req_id = connection_.new_request_id();

      // Serialize to a string.
      std::string request;
      {
        string_output_stream request_os(request);
        Encoder encoder(request_os);

        // Message type: RPC request.
        encoder(rpc_defs::message_type::REQUEST);
        // Request ID.
        encoder(req_id);
        // Name of the remote object.
        encoder(name_);
        // Method name.
        using method_descriptor = method_descriptor<mptr>;
        encoder(method_descriptor::name());
        // Method signature hash.
        constexpr auto method_hash = traits::type_hash_v<method_type>;
        encoder(method_hash);
        // Current context.
        encoder(context::current());
        // Actual arguments.
        encoder(args_refs);
      }

      // Send the request.
      auto response_future =
          connection_.send_request(req_id, std::move(request));

      return {response_future.then([](std::string response) {
                string_input_stream response_is(response);
                Decoder decoder(response_is);
                result_holder<return_type> result;
                decoder(result);
                return std::move(result).value();
              }),
              req_id};
    }

    template <auto mptr, typename... A>
    typename traits::member_function_pointer_traits<mptr>::return_type call(
        A&&... args) {
      auto [result, req_id] = async_call<mptr>(std::forward<A>(args)...);
      try {
        return result.get();
      } catch (const errors::cancelled&) {
        connection_.cancel_request(req_id);
        throw;
      }
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
        receiver_(&client_connection::receive, this),
        new_deadline_(max_queued_events),
        cancelled_requests_(max_queued_events),
        timeout_and_cancellation_handler_(
            &client_connection::handle_timeouts_and_cancellations, this) {}

  ~client_connection() {
    receiver_.get_context().cancel();
    connection_.disconnect();
    receiver_.join();

    timeout_and_cancellation_handler_.get_context().cancel();
    timeout_and_cancellation_handler_.join();
  }

  template <typename I, typename N>
  auto get_proxy(N&& name) {
    return I::make_proxy(remote_object(
        *this, object_name<ObjectNameEncoder>(std::forward<N>(name))));
  }

  void cancel_request(rpc_defs::request_id_type req_id) {
    abandon_request(req_id);

    try {
      cancelled_requests_.maybe_put(req_id);
    } catch (const errors::try_again&) {
    }
  }

 private:
  struct pending_request {
    std::optional<context::time_point> deadline;
    promise<std::string> result;
  };
  using pending_map_type = flat_map<std::uint32_t, pending_request>;

  void gc() {
    std::scoped_lock lock(pending_mu_);
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

  rpc_defs::request_id_type new_request_id() {
    std::scoped_lock lock(pending_mu_);
    return sequence_++;
  }

  void abandon_request(rpc_defs::request_id_type req_id) {
    std::scoped_lock lock(pending_mu_);
    auto it = pending_.find(req_id);
    if (it != pending_.end()) {
      it->second.result.set_exception(
          std::make_exception_ptr(errors::cancelled("Request cancelled")));
      pending_.erase(it);
    }
  }

  void set_response(rpc_defs::request_id_type req_id, std::string response) {
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

  void send(std::string data) {
    std::scoped_lock lock(sending_mu_);
    try {
      connection_.connect();
      connection_.send(std::move(data));
      ready_.set();
    } catch (...) {
      // Disconnect the connection.
      ready_.reset();
      connection_.disconnect();
      throw;
    }
  }

  future<std::string> send_request(rpc_defs::request_id_type req_id,
                                   std::string request) {
    future<std::string> result;

    {
      std::scoped_lock lock(pending_mu_);
      auto& pending = pending_[req_id];
      pending.deadline = context::current().deadline();
      result = pending.result.get_future();

      if (pending.deadline) {
        try {
          new_deadline_.maybe_put();
        } catch (const errors::try_again&) {
        }
      }
    }

    try {
      send(request);
    } catch (...) {
      abandon_request(req_id);
      throw;
    }

    return result;
  }

  void receive() {
    while (true) {
      // Wait until we actually need to receive something.
      select(ready_.wait_set());

      try {
        while (true) {
          // Read the next packet.
          auto response = connection_.receive();
          string_input_stream response_is(response);
          Decoder decoder(response_is);

          // Decode the message type.
          rpc_defs::message_type message_type;
          decoder(message_type);

          switch (message_type) {
            case rpc_defs::message_type::RESPONSE:
              // Decode the request id.
              rpc_defs::request_id_type req_id;
              decoder(req_id);

              response.erase(0, response_is.pos());
              set_response(req_id, std::move(response));
              break;
            default:
              // Unknown message received.
              throw errors::data_mismatch("Received invalid message type");
              break;
          }
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

  std::optional<context::time_point> get_earliest_deadline() {
    std::scoped_lock lock(pending_mu_);
    std::optional<context::time_point> earliest_deadline;
    for (const auto& p : pending_) {
      if (p.second.deadline) {
        if (earliest_deadline) {
          earliest_deadline = std::min(*earliest_deadline, *p.second.deadline);
        } else {
          earliest_deadline = *p.second.deadline;
        }
      }
    }
    return earliest_deadline;
  }

  void handle_timeouts_and_cancellations() {
    while (true) {
      auto earliest_deadline = get_earliest_deadline();

      auto [new_deadline_added, cancelled_request, time_to_do_gc] =
          select(new_deadline_.async_get(), cancelled_requests_.async_get(),
                 earliest_deadline ? deadline(*earliest_deadline) : never());

      if (time_to_do_gc) {
        gc();
      }

      if (cancelled_request) {
        auto req_id = *cancelled_request;

        // Serialize to a string.
        std::string cancel_request;
        {
          string_output_stream cancel_request_os(cancel_request);
          Encoder encoder(cancel_request_os);

          // Message type: RPC request.
          encoder(rpc_defs::message_type::CANCEL_REQUEST);
          // Request ID.
          encoder(req_id);
        }
        // Send the cancellation request. Ignore errors.
        try {
          send(cancel_request);
        } catch (...) {
        }
      }
    }
  }

  mutex pending_mu_, sending_mu_;
  rpc_defs::request_id_type sequence_;
  flag ready_;
  connection_type connection_;
  pending_map_type pending_;
  daemon_thread receiver_;
  queue<void> new_deadline_;
  queue<rpc_defs::request_id_type> cancelled_requests_;
  daemon_thread timeout_and_cancellation_handler_;
};

}  // namespace ash

#endif  // ASH_CLIENT_H_
