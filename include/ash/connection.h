/// \file
/// \brief Connections are bidirectional streams.
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

#ifndef INCLUDE_ASH_CONNECTION_H_
#define INCLUDE_ASH_CONNECTION_H_

#include <string>
#include <utility>
#include "ash/errors.h"
#include "ash/io_adapters.h"

namespace ash {

class connection : public input_stream, public output_stream {
 public:
  /// Destructor.
  virtual ~connection() {}

  /// (Re)connect to the connection's target.
  virtual void connect() {
    throw errors::not_implemented("Constructor-only connection");
  }

  /// Disconnect if the connection was active.
  virtual void disconnect() = 0;

  /// Check whether the connection is active.
  virtual bool connected() = 0;
};

class packet_connection {
 public:
  /// Destructor.
  virtual ~packet_connection() {}

  /// (Re)connect to the connection's target.
  virtual void connect() {
    throw errors::not_implemented("Constructor-only connection");
  }

  /// Disconnect if the connection was active.
  virtual void disconnect() = 0;

  /// Check whether the connection is active.
  virtual bool connected() = 0;

  /// Send a packet.
  virtual void send(std::string data) = 0;

  /// Receive a packet.
  virtual std::string receive() = 0;
};

template <typename Connection, typename PacketProtocol>
class packet_connection_impl : public packet_connection {
 public:
  template <typename... Args>
  explicit packet_connection_impl(Args&&... args)
      : connection_(std::forward<Args>(args)...) {}
  ~packet_connection_impl() override { disconnect(); }

  void connect() override { connection_.connect(); }
  void disconnect() override { connection_.disconnect(); }
  bool connected() override { return connection_.connected(); }
  void send(std::string data) override {
    protocol_.send(connection_, std::move(data));
  };
  std::string receive() override { return protocol_.receive(connection_); };

 private:
  Connection connection_;
  PacketProtocol protocol_;
};

}  // namespace ash

#endif  // INCLUDE_ASH_CONNECTION_H_
