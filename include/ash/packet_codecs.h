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
#include <cassert>
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

/// Codec provides a HighwayHash-based MAC encapsulation based on a 256-bit
/// secret and a random nonce.
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
    highway_hash hasher(key_);
    hasher.append(reinterpret_cast<const uint8_t*>(data.data()) + original_size,
                  8);
    hasher.append(reinterpret_cast<const uint8_t*>(data.data()), original_size);
    std::uint64_t hash = hasher.finish64();
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

    // Get the hash; verify it.
    highway_hash hasher(key_);
    hasher.append(reinterpret_cast<const uint8_t*>(data.data()) + original_size,
                  8);
    hasher.append(reinterpret_cast<const uint8_t*>(data.data()), original_size);
    std::uint64_t hash = hasher.finish64();
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

/// Codec that applies Consistent Overhead Byte Stuffing.
class cobs_codec : public packet_codec {
 public:
  void encode(std::string& data) override {
    // For n < 254 we'll need 1 extra byte (the ending 0 can be overwritten with
    // the first span counter). For n = 254 we'll need 2 extra bytes (as we'll
    // be using a second span). So we'll have a max overhead of 1 + (length /
    // 254).
    std::size_t original_size = data.size();
    std::size_t overhead = 1 + original_size / 254;
    data.resize(original_size + overhead * 2);
    data[original_size] = '\0';
    auto next_counter = data.rbegin();
    auto current_char = data.begin();
    auto end = current_char + original_size;
    std::size_t zeroes = 0;
    while (current_char < end) {
      std::size_t run_length = 0;
      while (*current_char && run_length < 254) {
        ++current_char;
        ++run_length;
      }
      *next_counter++ = run_length;
      if (run_length < 254) {
        // Skip the zero.
        ++current_char;
        ++zeroes;
      }
    }
    std::size_t num_counters = next_counter - data.rbegin();
    auto source_char = data.begin() + original_size;
    current_char = source_char + num_counters - zeroes;
    next_counter--;
    for (std::size_t c = 0; c < num_counters; ++c) {
      auto counter = static_cast<std::uint8_t>(*next_counter--);
      for (std::size_t i = 0; i < counter; ++i) {
        *--current_char = *--source_char;
      }
      *--current_char = 1 + counter;
      if (counter < 254) {
        --source_char;
      }
    }

    assert(current_char == data.begin());
    assert(source_char == data.begin());

    data.resize(original_size + num_counters - zeroes);
  }

  void decode(std::string& data) override {
    auto src = data.begin();
    auto dst = data.begin();
    while (src < data.end()) {
      std::size_t count = static_cast<std::uint8_t>(*src++) - 1;
      for (std::size_t i = 0; i < count; i++) {
        if (src >= data.end()) {
          throw errors::eof("Truncated COBS-encoded data.");
        }
        *dst++ = *src++;
      }
      if (count < 254) {
        *dst++ = '\0';
      }
    }
    data.resize(dst - data.begin());
  }
};
}  // namespace ash

#endif  // INCLUDE_ASH_PACKET_CODECS_H_
