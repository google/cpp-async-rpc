/// \file
/// \brief Interfaces for sending and receiving "packets" as `std::string`
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

#ifndef LASR_PACKET_PROTOCOLS_H_
#define LASR_PACKET_PROTOCOLS_H_

#include <limits>
#include <string>
#include <utility>

#include "lasr/binary_codecs.h"
#include "lasr/errors.h"
#include "lasr/io_adapters.h"
#include "lasr/packet_codecs.h"

/// Base class for all packet protocols.
namespace lasr {

template <std::size_t max_packet_size = std::numeric_limits<std::size_t>::max()>
class serial_line_packet_protocol {
 public:
  void send(output_stream& out, std::string data) {
    std::string packet = std::move(data);
    mac_.encode(packet);
    cobs_.encode(packet);
    out.write(packet.data(), packet.size());
    out.putc('\0');
    out.flush();
  }

  std::string receive(input_stream& in) {
    std::string packet;
    while (char c = in.getc()) {
      if (packet.size() >= max_packet_size) {
        throw errors::out_of_range("Exceeded max packet size when reading.");
      }
      packet.push_back(c);
    }
    cobs_.decode(packet);
    mac_.decode(packet);
    return packet;
  }

 private:
  mac_codec mac_;
  cobs_codec cobs_;
};

template <typename Encoder = little_endian_binary_encoder,
          typename Decoder = little_endian_binary_decoder>
class protected_stream_packet_protocol {
 public:
  virtual ~protected_stream_packet_protocol() {}

  void send(output_stream& out, std::string data) {
    std::string packet = std::move(data);
    Encoder encoder(out);
    encoder(packet);
    out.flush();
  }

  std::string receive(input_stream& in) {
    std::string packet;
    Decoder decoder(in);
    decoder(packet);
    return packet;
  }
};

}  // namespace lasr

#endif  // LASR_PACKET_PROTOCOLS_H_
