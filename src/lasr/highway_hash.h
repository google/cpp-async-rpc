/// \file
/// \brief Port of Google's HighwayHash
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

#ifndef LASR_HIGHWAY_HLASR_H_
#define LASR_HIGHWAY_HLASR_H_

#include <cstdint>

namespace lasr {

/// \brief Implementation of Google's HighwayHash.
class highway_hash {
 public:
  /// \brief Hash `size` bytes starting at `data` into 64 bits.
  static std::uint64_t hash64(const std::uint8_t* data, std::size_t size,
                              const uint64_t key[4]);

  /// \brief Hash `size` bytes starting at `data` into 128 bits.
  static void hash128(const std::uint8_t* data, std::size_t size,
                      const std::uint64_t key[4], std::uint64_t hash[2]);

  /// \brief Hash `size` bytes starting at `data` into 256 bits.
  static void hash256(const std::uint8_t* data, std::size_t size,
                      const std::uint64_t key[4], std::uint64_t hash[4]);

  /// \brief Construct a new object to accumulate data for hashing.
  explicit highway_hash(const std::uint64_t key[4]);

  /// \brief Restart the hash state.
  void reset(const std::uint64_t key[4]);

  /// \brief Append `size` bytes starting at `data` into the state.
  void append(const std::uint8_t* bytes, std::size_t num);

  /// \brief Get the 64 bits hash for the data so far.
  std::uint64_t finish64();

  /// \brief Get the 128 bits hash for the data so far.
  void finish128(std::uint64_t hash[2]);

  /// \brief Get the 256 bits hash for the data so far.
  void finish256(std::uint64_t hash[4]);

 private:
  void update_packet(const std::uint8_t* packet);

  void update_remainder(const std::uint8_t* bytes,
                        const std::size_t size_mod32);

  static void zipper_merge_and_add(const std::uint64_t v1,
                                   const std::uint64_t v0, std::uint64_t& add1,
                                   std::uint64_t& add0);

  void update(const std::uint64_t lanes[4]);

  static std::uint64_t read64(const std::uint8_t* src);

  static void rotate32by(std::uint64_t count, std::uint64_t lanes[4]);

  static void permute(const std::uint64_t v[4], std::uint64_t* permuted);

  void permute_and_update();

  static void modular_reduction(std::uint64_t a3_unmasked, std::uint64_t a2,
                                std::uint64_t a1, std::uint64_t a0,
                                std::uint64_t& m1, std::uint64_t& m0);

  std::uint64_t finalize64();

  void finalize128(std::uint64_t hash[2]);

  void finalize256(std::uint64_t hash[4]);

  void process_all(const std::uint8_t* data, std::size_t size);

  std::uint64_t v0_[4];
  std::uint64_t v1_[4];
  std::uint64_t mul0_[4];
  std::uint64_t mul1_[4];

  std::uint8_t packet_[32];
  std::size_t buffered_bytes_;
};
}  // namespace lasr

#endif  // LASR_HIGHWAY_HLASR_H_
