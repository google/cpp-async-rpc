/// \file
/// \brief Interfaces for sending and receiving "packets" as `std::string`
/// buffers.
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

#ifndef INCLUDE_ASH_PACKET_PROTOCOLS_H_
#define INCLUDE_ASH_PACKET_PROTOCOLS_H_

#include <iostream>
#include <limits>
#include <string>
#include "ash/binary_codecs.h"
#include "ash/errors.h"
#include "ash/io_adapters.h"
#include "ash/packet_codecs.h"

/// Base class for all packet protocols.
namespace ash {
class packet_protocol {
 public:
  virtual ~packet_protocol() {}

  virtual void send(std::string data) = 0;
  virtual std::string receive() = 0;
  virtual void close() = 0;
};

template <std::size_t max_packet_size = std::numeric_limits<std::size_t>::max()>
class stream_packet_protocol : public packet_protocol {
 public:
  virtual ~stream_packet_protocol() {}

  stream_packet_protocol(input_stream& in, output_stream& out)
      : in_(in), out_(out) {}

  void close() override {
    in_.close();
    out_.close();
  }

 protected:
  input_stream& in_;
  output_stream& out_;
};

template <std::size_t max_packet_size = std::numeric_limits<std::size_t>::max()>
class serial_line_packet_protocol
    : public stream_packet_protocol<max_packet_size> {
 public:
  virtual ~serial_line_packet_protocol() {}

  using stream_packet_protocol<max_packet_size>::stream_packet_protocol;

  void send(std::string data) override {
    std::string packet = std::move(data);
    mac_.encode(packet);
    cobs_.encode(packet);
    this->out_.write(packet.data(), packet.size());
    this->out_.putc('\0');
    this->out_.flush();
  }

  std::string receive() override {
    std::string packet;
    while (char c = this->in_.getc()) {
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
          typename Decoder = little_endian_binary_decoder,
          std::size_t max_packet_size = std::numeric_limits<std::size_t>::max()>
class protected_stream_packet_protocol
    : public stream_packet_protocol<max_packet_size> {
 public:
  virtual ~protected_stream_packet_protocol() {}

  protected_stream_packet_protocol(input_stream& in, output_stream& out)
      : stream_packet_protocol<max_packet_size>(in, out),
        decoder_(this->in_),
        encoder_(this->out_) {}

  void send(std::string data) override {
    std::string packet = std::move(data);
    encoder_(packet);
    this->out_.flush();
  }

  std::string receive() override {
    std::string packet;
    decoder_(packet);
    return packet;
  }

 private:
  Decoder decoder_;
  Encoder encoder_;
  mac_codec mac_;
  cobs_codec cobs_;
};

}  // namespace ash

#endif  // INCLUDE_ASH_PACKET_PROTOCOLS_H_
