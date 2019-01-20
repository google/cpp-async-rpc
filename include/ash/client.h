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

#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include "ash/binary_codecs.h"
#include "ash/connection.h"
#include "ash/errors.h"
#include "ash/interface.h"
#include "ash/string_adapters.h"
#include "ash/traits/type_traits.h"
#include "ash/type_hash.h"

namespace ash {

#define ASH_CALL(...) call<decltype(&__VA_ARGS__), &__VA_ARGS__>

template <typename Encoder = little_endian_binary_encoder,
          typename Decoder = little_endian_binary_decoder>
class client_connection {
 private:
  struct remote_object {
    remote_object(client_connection<Encoder, Decoder>& connection,
                  const std::string& name)
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
        ash::error_factory::get().throw_error(exception_type,
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
      : connection_(connection) {}

  template <typename I>
  auto get_proxy(const std::string& name) {
    return I::make_proxy(remote_object(*this, name));
  }

 private:
  std::string send(std::string request) {
    connection_.connect();
    connection_.send(std::move(request));
    return connection_.receive();
  }

  packet_connection& connection_;
};

}  // namespace ash

#endif  // INCLUDE_ASH_CLIENT_H_
