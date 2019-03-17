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
#include "ash/select.h"
#include "ash/string_adapters.h"
#include "ash/thread.h"
#include "ash/traits/type_traits.h"
#include "ash/type_hash.h"

namespace ash {

template <typename Connection = client_socket_connection,
          typename Encoder = little_endian_binary_encoder,
          typename Decoder = little_endian_binary_decoder,
          typename PacketProtocol =
              protected_stream_packet_protocol<Encoder, Decoder>,
          typename ObjectNameEncoder = Encoder>
class client_connection {
 private:
  struct remote_object {
    remote_object(client_connection& connection, std::string_view name)
        : connection_(connection), name_(name) {}

    template <auto mptr, typename... A>
    typename traits::member_function_pointer_traits<mptr>::return_type call(
        A&&... args) {
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
      // Name of the remote object.
      encoder(name_);
      // Method name.
      using method_descriptor = method_descriptor<mptr>;
      encoder(method_descriptor::name());
      // Method signature hash.
      constexpr auto method_hash = traits::type_hash<method_type>::value;
      encoder(method_hash);
      // Actual arguments.
      encoder(args_refs);
      // Current context.
      encoder(context::current());

      // Send the request, receive and deserialize the result.
      std::string response(connection_.send(std::move(request)));
      string_input_stream response_is(response);
      Decoder decoder(response_is);
      bool got_exception;
      decoder(got_exception);
      if (got_exception) {
        std::string exception_type, exception_message;
        decoder(exception_type);
        decoder(exception_message);
        ash::error_factory::get().throw_error(exception_type.c_str(),
                                              exception_message.c_str());
      }
      if constexpr (!std::is_same_v<return_type, void>) {
        return_type result;
        decoder(result);
        return result;
      }
    }

    client_connection& connection_;
    const std::string name_;
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
  using pending_map_type = flat_map<std::uint32_t, promise<std::string>>;
  using sequence_type = std::uint32_t;

  std::string send(std::string request) {
    future<std::string> result;
    {
      std::scoped_lock lock(mu_);
      auto req_id = sequence_++;
      try {
        result = pending_[req_id].get_future();

        request.reserve(request.size() + sizeof(req_id));
        for (std::size_t i = 0; i < sizeof(req_id); i++) {
          request.push_back((req_id >> (8 * i)) & 0xff);
        }
        connection_.connect();
        connection_.send(std::move(request));
        ready_.set();
      } catch (...) {
        // Disconnect the connection.
        ready_.reset();
        connection_.disconnect();

        // Cleanup the expected result slot.
        pending_.erase(req_id);

        throw;
      }
    }
    return result.get();
  }

  void receive() {
    auto exc = std::current_exception();

    while (true) {
      // Wait until we actually need to receive something.
      select(ready_.wait_set());

      try {
        while (true) {
          // Read the next packet.
          auto response = connection_.receive();
          sequence_type req_id = 0;
          if (response.size() >= sizeof(req_id)) {
            // Find the req_id.
            const std::uint8_t* ptr = reinterpret_cast<const std::uint8_t*>(
                response.data() + response.size() - sizeof(req_id));
            for (std::size_t i = 0; i < sizeof(req_id); i++) {
              req_id <<= 8;
              req_id |= static_cast<sequence_type>(*ptr++);
            }
            response.resize(response.size() - sizeof(req_id));

            {
              std::scoped_lock lock(mu_);
              auto it = pending_.find(req_id);
              if (it != pending_.end()) {
                it->second.set_value(std::move(response));
                pending_.erase(it);
              }
            }
          }
        }
      } catch (...) {
        // Broadcast the exception to all the pending requests.
        auto exc = std::current_exception();
        std::unique_lock lock(mu_);

        // Disconnect the connection.
        ready_.reset();
        connection_.disconnect();

        // Report the exception for all pending requests and unqueue them.
        for (auto& p : pending_) {
          p.second.set_exception(exc);
        }
        pending_.clear();
      }
    }
  }

  mutex mu_;
  sequence_type sequence_;
  flag ready_;
  connection_type connection_;
  pending_map_type pending_;
  ash::thread receiver_;
};
}  // namespace ash

#endif  // ASH_CLIENT_H_
