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

#ifndef ASH_CONNECTION_H_
#define ASH_CONNECTION_H_

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include "ash/errors.h"
#include "ash/io.h"
#include "ash/io_adapters.h"
#include "ash/mpt.h"
#include "ash/sync.h"

namespace ash {

class connection : public input_stream, public output_stream {
 public:
  /// Destructor.
  virtual ~connection();

  /// (Re)connect to the connection's target.
  virtual void connect();

  /// Disconnect if the connection was active.
  virtual void disconnect() = 0;

  /// Check whether the connection is active.
  virtual bool connected() = 0;
};

template <typename Connection>
class reconnectable_connection : public connection {
 private:
  std::shared_ptr<Connection> get() {
    auto res = ptr_;
    if (!res) throw errors::io_error("Connection is closed");
    return res;
  }

 public:
  template <typename... Args>
  explicit reconnectable_connection(Args&&... args)
      : connection_factory_(
            [args_tuple = std::make_tuple(std::forward<Args>(args)...),
             this]() {
              return std::apply(
                  [this](const auto&... args) { connection_.emplace(args...); },
                  args_tuple);
            }) {}

  ~reconnectable_connection() override { disconnect(); }

  void connect() override {
    std::scoped_lock lock(mu_);
    if (!ptr_ || !ptr_->connected()) {
      connection_factory_();
      ptr_.reset(&*connection_, [this](Connection*) { done_.notify_all(); });
    }
  }

  void disconnect() override {
    std::unique_lock lock(mu_);

    if (ptr_) {
      ptr_->disconnect();
      ptr_.reset();
      done_.wait(lock, [this]() { return !connection_; });
      connection_.reset();
    }
  }

  bool connected() override {
    auto ptr = ptr_;
    return ptr && ptr->connected();
  }

  void write(const char* data, std::size_t size) override {
    get()->write(data, size);
  }

  void putc(char c) override { get()->putc(c); }

  void flush() override { get()->flush(); }

  std::size_t read(char* data, std::size_t size) override {
    return get()->read(data, size);
  }

  char getc() override { return get()->getc(); }

 private:
  std::optional<Connection> connection_;
  std::shared_ptr<Connection> ptr_;
  std::mutex mu_;
  std::condition_variable done_;
  std::function<void()> connection_factory_;
};

class packet_connection {
 public:
  /// Destructor.
  virtual ~packet_connection();

  /// (Re)connect to the connection's target.
  virtual void connect();

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

class channel_connection : public connection {
 protected:
  class channel_lock {
   public:
    explicit channel_lock(channel_connection& conn);
    ~channel_lock();

   private:
    channel_connection& conn_;
  };

 public:
  explicit channel_connection(channel&& ch);
  ~channel_connection() override;
  bool connected() override;
  void disconnect() override;
  void write(const char* data, std::size_t size) override;
  void putc(char c) override;
  void flush() override;
  std::size_t read(char* data, std::size_t size) override;
  char getc() override;

 protected:
  void check_connected();

  std::mutex mu_;
  std::condition_variable idle_;
  channel channel_;
  flag closing_;
  int lock_count_ = 0;
};  // namespace ash

class char_dev_connection : public channel_connection {
 public:
  explicit char_dev_connection(const std::string& path);

 protected:
  static channel open_path(const std::string& path);
};

}  // namespace ash

#endif  // ASH_CONNECTION_H_
