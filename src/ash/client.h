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

#ifndef INCLUDE_ASH_CLIENT_H_
#define INCLUDE_ASH_CLIENT_H_

#include <condition_variable>
#include <future>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include "ash/binary_codecs.h"
#include "ash/connection.h"
#include "ash/errors.h"
#include "ash/interface.h"
#include "ash/string_adapters.h"
#include "ash/traits/type_traits.h"
#include "ash/type_hash.h"

namespace ash {

template <typename Encoder = little_endian_binary_encoder,
          typename Decoder = little_endian_binary_decoder>
class client_connection {
 private:
  struct remote_object {
    remote_object(client_connection<Encoder, Decoder>& connection,
                  std::string_view name)
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

    client_connection<Encoder, Decoder>& connection_;
    const std::string name_;
  };

 public:
  explicit client_connection(packet_connection& connection)
      : sequence_(0),
        stopping_(false),
        connection_(connection),
        receiver_(&client_connection<Encoder, Decoder>::receive, this) {}

  ~client_connection() {
    {
      std::scoped_lock lock(mu_);
      stopping_ = true;
      connection_.disconnect();
    }
    start_stop_cond_.notify_all();
    receiver_.join();
  }

  template <typename I>
  auto get_proxy(std::string_view name) {
    return I::make_proxy(remote_object(*this, name));
  }

 private:
  using pending_map_type =
      std::unordered_map<std::uint32_t, std::promise<std::string>>;

  std::string send(std::string request) {
    std::future<std::string> result;
    {
      std::scoped_lock lock(mu_);
      auto req_id = sequence_++;
      request.reserve(request.size() + sizeof(req_id));
      for (std::size_t i = 0; i < sizeof(req_id); i++) {
        request.push_back((req_id >> (8 * i)) & 0xff);
      }
      try {
        connection_.connect();
        connection_.send(std::move(request));
      } catch (...) {
        // Disconnect the connection.
        connection_.disconnect();
        throw;
      }
      result = pending_[req_id].get_future();
    }
    start_stop_cond_.notify_all();
    return result.get();
  }

  void receive() {
    auto exc = std::current_exception();

    while (true) {
      {
        std::unique_lock lock(mu_);

        // Disconnect the connection.
        connection_.disconnect();

        // Report the exception for all pending requests and unqueue them.
        for (auto& p : pending_) {
          p.second.set_exception(exc);
        }
        pending_.clear();

        start_stop_cond_.wait(
            lock, [this]() { return stopping_ || !pending_.empty(); });
        if (stopping_) {
          return;
        }
      }

      try {
        while (true) {
          // Read the next packet.
          auto response = connection_.receive();
          std::uint32_t req_id = 0;
          if (response.size() >= sizeof(req_id)) {
            // Find the req_id.
            const std::uint8_t* ptr = reinterpret_cast<const std::uint8_t*>(
                response.data() + response.size() - sizeof(req_id));
            for (std::size_t i = 0; i < sizeof(req_id); i++) {
              req_id <<= 8;
              req_id |= static_cast<std::uint32_t>(*ptr++);
            }
            response.resize(response.size() - sizeof(req_id));

            typename pending_map_type::node_type n;
            {
              std::scoped_lock lock(mu_);
              n = pending_.extract(req_id);
            }
            if (!n.empty()) {
              n.mapped().set_value(response);
            }
          }
        }
      } catch (...) {
        // Record the exception
        exc = std::current_exception();
      }
    }
  }

  std::mutex mu_;
  std::uint32_t sequence_;
  bool stopping_;
  std::condition_variable start_stop_cond_;
  packet_connection& connection_;
  std::thread receiver_;
  pending_map_type pending_;
};

}  // namespace ash

#endif  // INCLUDE_ASH_CLIENT_H_
