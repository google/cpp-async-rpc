/// \file
/// \brief Transformations on buffers represented as `std::string` objects.
///
/// \copyright
///   Copyright 2019 by Google LLC. All Rights Reserved.
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

#include "lasr/packet_codecs.h"
#include <algorithm>
#include <cassert>
#include <vector>
#include "lasr/errors.h"
#include "lasr/highway_hash.h"
namespace lasr {

packet_codec::~packet_codec() {}

mac_codec::mac_codec(const std::uint64_t key[4]) {
  std::copy(key, key + 4, key_);
}

void mac_codec::encode(std::string& data) {
  std::size_t original_size = data.size();

  // Add space for the hash.
  data.reserve(original_size + sizeof(std::uint64_t));

  // Get the hash; append it.
  std::uint64_t hash = highway_hash::hash64(
      reinterpret_cast<const std::uint8_t*>(data.data()), original_size, key_);
  for (std::size_t i = 0; i < sizeof(std::uint64_t); i++) {
    data.push_back(hash & 0xff);
    hash >>= 8;
  }
}

void mac_codec::decode(std::string& data) {
  if (data.size() < 8) {
    throw errors::data_mismatch("Packet too short for MAC decode");
  }
  std::size_t original_size = data.size() - sizeof(std::uint64_t);

  // Get the hash; verify it.
  std::uint64_t hash = highway_hash::hash64(
      reinterpret_cast<const std::uint8_t*>(data.data()), original_size, key_);
  const std::uint8_t* ptr =
      reinterpret_cast<const std::uint8_t*>(data.data()) + original_size;
  for (std::size_t i = 0; i < sizeof(std::uint64_t); i++) {
    if (*ptr++ != (hash & 0xff)) {
      throw errors::data_mismatch("Hash mismatch in MAC decode");
    }
    hash >>= 8;
  }

  // Cut the tail.
  data.resize(original_size);
}

constexpr std::uint64_t mac_codec::default_key[4];

void cobs_codec::encode(std::string& data) {
  std::size_t extra_bytes = 0;
  std::size_t original_size = data.size();
  std::size_t overhead = 1 + original_size / 254;
  data.resize(original_size + overhead * +1);
  data[original_size] = '\0';

  std::vector<std::size_t> counts;
  counts.push_back(254);
  auto current_char = data.begin();
  auto end = current_char + original_size + 1;
  while (current_char < end) {
    std::size_t run_length = 0;
    while (*current_char && run_length < 254) {
      ++current_char;
      ++run_length;
    }
    if (counts.back() == 254) {
      ++extra_bytes;
    }
    counts.push_back(run_length);
    if (run_length < 254) {
      // Skip the zero.
      ++current_char;
    }
  }
  auto source_char = data.begin() + original_size;
  current_char = source_char + extra_bytes;
  auto count_it = counts.rbegin();
  while (count_it < counts.rend() - 1) {
    auto counter = *count_it++;
    for (std::size_t i = 0; i < counter; ++i) {
      *--current_char = *--source_char;
    }
    *--current_char = 1 + counter;
    if (*count_it < 254) {
      --source_char;
    }
  }

  assert(current_char == data.begin());
  assert(source_char == data.begin());

  data.resize(original_size + extra_bytes);
}

void cobs_codec::decode(std::string& data) {
  auto src = data.begin();
  auto dst = data.begin();
  while (src < data.end()) {
    std::size_t count = static_cast<std::uint8_t>(*src++) - 1;
    for (std::size_t i = 0; i < count; i++) {
      if (src >= data.end()) {
        throw errors::data_mismatch("Truncated COBS-encoded data");
      }
      *dst++ = *src++;
    }
    if (count < 254) {
      *dst++ = '\0';
    }
  }
  data.resize((dst - data.begin()) - 1);
}
}  // namespace ash
