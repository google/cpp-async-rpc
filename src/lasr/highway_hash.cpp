/// \file
/// \brief Port of Google's HighwayHash
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

#include "lasr/highway_hash.h"

#include <algorithm>

namespace lasr {

/// \brief Hash `size` bytes starting at `data` into 64 bits.
std::uint64_t highway_hash::hash64(const std::uint8_t* data, std::size_t size,
                                   const uint64_t key[4]) {
  highway_hash state(key);
  state.process_all(data, size);
  return state.finalize64();
}

/// \brief Hash `size` bytes starting at `data` into 128 bits.
void highway_hash::hash128(const std::uint8_t* data, std::size_t size,
                           const std::uint64_t key[4], std::uint64_t hash[2]) {
  highway_hash state(key);
  state.process_all(data, size);
  state.finalize128(hash);
}

/// \brief Hash `size` bytes starting at `data` into 256 bits.
void highway_hash::hash256(const std::uint8_t* data, std::size_t size,
                           const std::uint64_t key[4], std::uint64_t hash[4]) {
  highway_hash state(key);
  state.process_all(data, size);
  state.finalize256(hash);
}

highway_hash::highway_hash(const std::uint64_t key[4]) { reset(key); }

void highway_hash::reset(const std::uint64_t key[4]) {
  buffered_bytes_ = 0;
  std::fill(packet_, packet_ + 32, 0);

  mul0_[0] = 0xdbe6d5d5fe4cce2full;
  mul0_[1] = 0xa4093822299f31d0ull;
  mul0_[2] = 0x13198a2e03707344ull;
  mul0_[3] = 0x243f6a8885a308d3ull;
  mul1_[0] = 0x3bd39e10cb0ef593ull;
  mul1_[1] = 0xc0acf169b5f18a8cull;
  mul1_[2] = 0xbe5466cf34e90c6cull;
  mul1_[3] = 0x452821e638d01377ull;
  v0_[0] = mul0_[0] ^ key[0];
  v0_[1] = mul0_[1] ^ key[1];
  v0_[2] = mul0_[2] ^ key[2];
  v0_[3] = mul0_[3] ^ key[3];
  v1_[0] = mul1_[0] ^ ((key[0] >> 32) | (key[0] << 32));
  v1_[1] = mul1_[1] ^ ((key[1] >> 32) | (key[1] << 32));
  v1_[2] = mul1_[2] ^ ((key[2] >> 32) | (key[2] << 32));
  v1_[3] = mul1_[3] ^ ((key[3] >> 32) | (key[3] << 32));
}

void highway_hash::append(const std::uint8_t* bytes, std::size_t num) {
  size_t i;
  if (buffered_bytes_ != 0) {
    size_t num_add =
        num > (32u - buffered_bytes_) ? (32u - buffered_bytes_) : num;
    for (i = 0; i < num_add; i++) {
      packet_[buffered_bytes_ + i] = bytes[i];
    }
    buffered_bytes_ += num_add;
    num -= num_add;
    bytes += num_add;
    if (buffered_bytes_ == 32) {
      update_packet(packet_);
      buffered_bytes_ = 0;
    }
  }
  while (num >= 32) {
    update_packet(bytes);
    num -= 32;
    bytes += 32;
  }
  for (i = 0; i < num; i++) {
    packet_[buffered_bytes_] = bytes[i];
    buffered_bytes_++;
  }
}

/// \brief Get the 64 bits hash for the data so far.
std::uint64_t highway_hash::finish64() {
  auto copy = *this;
  if (buffered_bytes_) {
    copy.update_remainder(packet_, buffered_bytes_);
  }
  return copy.finalize64();
}

/// \brief Get the 128 bits hash for the data so far.
void highway_hash::finish128(std::uint64_t hash[2]) {
  auto copy = *this;
  if (buffered_bytes_) {
    copy.update_remainder(packet_, buffered_bytes_);
  }
  copy.finalize128(hash);
}

/// \brief Get the 256 bits hash for the data so far.
void highway_hash::finish256(std::uint64_t hash[4]) {
  auto copy = *this;
  if (buffered_bytes_) {
    copy.update_remainder(packet_, buffered_bytes_);
  }
  copy.finalize256(hash);
}

void highway_hash::update_packet(const std::uint8_t* packet) {
  std::uint64_t lanes[4];
  lanes[0] = read64(packet + 0);
  lanes[1] = read64(packet + 8);
  lanes[2] = read64(packet + 16);
  lanes[3] = read64(packet + 24);
  update(lanes);
}

void highway_hash::update_remainder(const std::uint8_t* bytes,
                                    const std::size_t size_mod32) {
  int i;
  const std::size_t size_mod4 = size_mod32 & 3;
  const std::uint8_t* remainder = bytes + (size_mod32 & ~3);
  std::fill(packet_, packet_ + 32, 0);
  for (i = 0; i < 4; ++i) {
    v0_[i] += (static_cast<std::uint64_t>(size_mod32) << 32) + size_mod32;
  }
  rotate32by(size_mod32, v1_);
  for (i = 0; i < remainder - bytes; i++) {
    packet_[i] = bytes[i];
  }
  if (size_mod32 & 16) {
    for (i = 0; i < 4; i++) {
      packet_[28 + i] = remainder[i + size_mod4 - 4];
    }
  } else {
    if (size_mod4) {
      packet_[16 + 0] = remainder[0];
      packet_[16 + 1] = remainder[size_mod4 >> 1];
      packet_[16 + 2] = remainder[size_mod4 - 1];
    }
  }
  update_packet(packet_);
}

void highway_hash::zipper_merge_and_add(const std::uint64_t v1,
                                        const std::uint64_t v0,
                                        std::uint64_t& add1,
                                        std::uint64_t& add0) {
  add0 += (((v0 & 0xff000000ull) | (v1 & 0xff00000000ull)) >> 24) |
          (((v0 & 0xff0000000000ull) | (v1 & 0xff000000000000ull)) >> 16) |
          (v0 & 0xff0000ull) | ((v0 & 0xff00ull) << 32) |
          ((v1 & 0xff00000000000000ull) >> 8) | (v0 << 56);
  add1 += (((v1 & 0xff000000ull) | (v0 & 0xff00000000ull)) >> 24) |
          (v1 & 0xff0000ull) | ((v1 & 0xff0000000000ull) >> 16) |
          ((v1 & 0xff00ull) << 24) | ((v0 & 0xff000000000000ull) >> 8) |
          ((v1 & 0xffull) << 48) | (v0 & 0xff00000000000000ull);
}

void highway_hash::update(const std::uint64_t lanes[4]) {
  int i;
  for (i = 0; i < 4; ++i) {
    v1_[i] += mul0_[i] + lanes[i];
    mul0_[i] ^= (v1_[i] & 0xffffffff) * (v0_[i] >> 32);
    v0_[i] += mul1_[i];
    mul1_[i] ^= (v0_[i] & 0xffffffff) * (v1_[i] >> 32);
  }
  zipper_merge_and_add(v1_[1], v1_[0], v0_[1], v0_[0]);
  zipper_merge_and_add(v1_[3], v1_[2], v0_[3], v0_[2]);
  zipper_merge_and_add(v0_[1], v0_[0], v1_[1], v1_[0]);
  zipper_merge_and_add(v0_[3], v0_[2], v1_[3], v1_[2]);
}

std::uint64_t highway_hash::read64(const std::uint8_t* src) {
  return static_cast<std::uint64_t>(src[0]) |
         (static_cast<std::uint64_t>(src[1]) << 8) |
         (static_cast<std::uint64_t>(src[2]) << 16) |
         (static_cast<std::uint64_t>(src[3]) << 24) |
         (static_cast<std::uint64_t>(src[4]) << 32) |
         (static_cast<std::uint64_t>(src[5]) << 40) |
         (static_cast<std::uint64_t>(src[6]) << 48) |
         (static_cast<std::uint64_t>(src[7]) << 56);
}

void highway_hash::rotate32by(std::uint64_t count, std::uint64_t lanes[4]) {
  int i;
  for (i = 0; i < 4; ++i) {
    std::uint32_t half0 = lanes[i] & 0xffffffff;
    std::uint32_t half1 = (lanes[i] >> 32);
    lanes[i] = (half0 << count) | (half0 >> (32 - count));
    lanes[i] |=
        static_cast<std::uint64_t>((half1 << count) | (half1 >> (32 - count)))
        << 32;
  }
}

void highway_hash::permute(const std::uint64_t v[4], std::uint64_t* permuted) {
  permuted[0] = (v[2] >> 32) | (v[2] << 32);
  permuted[1] = (v[3] >> 32) | (v[3] << 32);
  permuted[2] = (v[0] >> 32) | (v[0] << 32);
  permuted[3] = (v[1] >> 32) | (v[1] << 32);
}

void highway_hash::permute_and_update() {
  std::uint64_t permuted[4];
  permute(v0_, permuted);
  update(permuted);
}

void highway_hash::modular_reduction(std::uint64_t a3_unmasked,
                                     std::uint64_t a2, std::uint64_t a1,
                                     std::uint64_t a0, std::uint64_t& m1,
                                     std::uint64_t& m0) {
  std::uint64_t a3 = a3_unmasked & 0x3FFFFFFFFFFFFFFFull;
  m1 = a1 ^ ((a3 << 1) | (a2 >> 63)) ^ ((a3 << 2) | (a2 >> 62));
  m0 = a0 ^ (a2 << 1) ^ (a2 << 2);
}

std::uint64_t highway_hash::finalize64() {
  int i;
  for (i = 0; i < 4; i++) {
    permute_and_update();
  }
  return v0_[0] + v1_[0] + mul0_[0] + mul1_[0];
}

void highway_hash::finalize128(std::uint64_t hash[2]) {
  int i;
  for (i = 0; i < 6; i++) {
    permute_and_update();
  }
  hash[0] = v0_[0] + mul0_[0] + v1_[2] + mul1_[2];
  hash[1] = v0_[1] + mul0_[1] + v1_[3] + mul1_[3];
}

void highway_hash::finalize256(std::uint64_t hash[4]) {
  int i;
  /* We anticipate that 256-bit hashing will be mostly used with long messages
     because storing and using the 256-bit hash (in contrast to 128-bit)
     carries a larger additional constant cost by itself. Doing extra rounds
     here hardly increases the per-byte cost of long messages. */
  for (i = 0; i < 10; i++) {
    permute_and_update();
  }
  modular_reduction(v1_[1] + mul1_[1], v1_[0] + mul1_[0], v0_[1] + mul0_[1],
                    v0_[0] + mul0_[0], hash[1], hash[0]);
  modular_reduction(v1_[3] + mul1_[3], v1_[2] + mul1_[2], v0_[3] + mul0_[3],
                    v0_[2] + mul0_[2], hash[3], hash[2]);
}

void highway_hash::process_all(const std::uint8_t* data, std::size_t size) {
  std::size_t i;
  for (i = 0; i + 32 <= size; i += 32) {
    update_packet(data + i);
  }
  if ((size & 31) != 0) update_remainder(data + i, size & 31);
}

}  // namespace lasr
