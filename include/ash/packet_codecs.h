/// \file
/// \brief Transformations on buffers represented as `std::string` objects.
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

#ifndef INCLUDE_ASH_PACKET_CODECS_H_
#define INCLUDE_ASH_PACKET_CODECS_H_

#include <algorithm>
#include <cstdint>
#include <random>
#include <string>
#include "ash/errors.h"
#include "ash/highway_hash.h"

/// Base class for all packet codecs.
namespace ash {
class packet_codec {
 public:
  virtual ~packet_codec() {}

  virtual void encode(std::string& data) = 0;
  virtual void decode(std::string& data) = 0;
};

class mac_codec : public packet_codec {
 public:
  explicit mac_codec(std::uint64_t key[4])
      : gen_(std::random_device()()), dis_(0, 255) {
    std::copy(key, key + 4, key_);
  }

  void encode(std::string& data) override {
    std::size_t original_size = data.size();

    // Add space for the nonce and the hash.
    data.reserve(original_size + 16);

    // Add a nonce.
    for (std::size_t i = 0; i < 8; i++) {
      data.push_back(dis_(gen_));
    }

    // Get the hash; append it.
    std::uint64_t hash = highway_hash::hash64(
        reinterpret_cast<const uint8_t*>(data.data()), original_size + 8, key_);
    for (std::size_t i = 0; i < 8; i++) {
      data.push_back(hash & 0xff);
      hash >>= 8;
    }
  }

  void decode(std::string& data) override {
    if (data.size() < 16) {
      throw errors::data_mismatch("Packet too short for MAC decode");
    }
    std::size_t original_size = data.size() - 16;

    std::uint64_t hash = highway_hash::hash64(
        reinterpret_cast<const uint8_t*>(data.data()), original_size + 8, key_);
    const uint8_t* ptr =
        reinterpret_cast<const uint8_t*>(data.data()) + original_size + 8;
    for (std::size_t i = 0; i < 8; i++) {
      if (*ptr++ != (hash & 0xff)) {
        throw errors::data_mismatch("Hash mismatch in MAC decode");
      }
      hash >>= 8;
    }

    // Cut the tail.
    data.resize(original_size);
  }

 private:
  std::uint64_t key_[4];
  std::mt19937 gen_;
  std::uniform_int_distribution<int> dis_;
};
}  // namespace ash

#endif  // INCLUDE_ASH_PACKET_CODECS_H_
