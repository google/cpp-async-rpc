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
#include "ash/interface.h"
#include "ash/packet_protocols.h"
#include "ash/string_adapters.h"
#include "ash/traits/type_traits.h"
#include "ash/type_hash.h"

namespace ash {

#define ASH_CALL(...) call<decltype(&__VA_ARGS__), &__VA_ARGS__>

template <typename Encoder, typename Decoder>
class remote_object;

template <typename Encoder = little_endian_binary_encoder,
          typename Decoder = little_endian_binary_decoder>
class client_connection {
 public:
  explicit client_connection(packet_protocol& protocol) : protocol_(protocol) {}

  remote_object<Encoder, Decoder> object(const std::string& name);

 private:
  friend class remote_object<Encoder, Decoder>;

  std::string send(std::string&& request) {
    protocol_.send(std::move(request));
    return protocol_.receive();
  }

  packet_protocol& protocol_;
};

template <typename Encoder, typename Decoder>
class remote_object {
 public:
  template <typename MFP, MFP mptr, typename... A>
  typename traits::member_function_pointer_traits<MFP, mptr>::return_type call(
      A... args) {
    using return_type =
        typename traits::member_function_pointer_traits<MFP, mptr>::return_type;
    using method_type =
        typename traits::member_function_pointer_traits<MFP, mptr>::method_type;

    // Get a serializable tuple with the args.
    using args_ref_tuple_type = typename traits::member_function_pointer_traits<
        MFP, mptr>::args_ref_tuple_type;
    args_ref_tuple_type args_refs(args...);

    // Serialize to a string.
    std::string request;
    string_output_stream request_os(request);
    Encoder encoder(request_os);
    // Name of the remote object.
    encoder(name_);
    // Method name.
    using method_descriptor = typename get_method_descriptor<MFP, mptr>::type;
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
    return decode<return_type>(decoder);
  }

 private:
  template <typename R>
  typename std::enable_if<std::is_same<void, R>::value, R>::type decode(
      Decoder& decoder) {}

  template <typename R>
  typename std::enable_if<!std::is_same<void, R>::value, R>::type decode(
      Decoder& decoder) {
    R result;
    decoder(result);
    return result;
  }

  friend class client_connection<Encoder, Decoder>;

  remote_object(client_connection<Encoder, Decoder>& connection,
                const std::string& name)
      : connection_(connection), name_(name) {}

  client_connection<Encoder, Decoder>& connection_;
  const std::string name_;
};

template <typename Encoder, typename Decoder>
remote_object<Encoder, Decoder> client_connection<Encoder, Decoder>::object(
    const std::string& name) {
  return {*this, name};
}

}  // namespace ash

#endif  // INCLUDE_ASH_CLIENT_H_
