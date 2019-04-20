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

#ifndef LASR_PACKET_CODECS_H_
#define LASR_PACKET_CODECS_H_

#include <cstdint>
#include <string>

/// Base class for all packet codecs.
namespace lasr {
class packet_codec {
 public:
  virtual ~packet_codec();

  virtual void encode(std::string& data) = 0;
  virtual void decode(std::string& data) = 0;
};

/// Codec that provides a HighwayHash-based MAC encapsulation based on a 256-bit
/// shared secret.
///
/// Please notice that this scheme doesn't protect against replay attacks (no
/// nonces are added by the scheme itself; the user of this codec should embed
/// adequately obtained nonces in the data if this aspect is important.
///
/// A default key value is provided in `default_key`
class mac_codec : public packet_codec {
 public:
  /// Convenience "random" default key.
  static constexpr std::uint64_t default_key[4] = {
      0xb6b9bb544bfd7e87ULL, 0xd5c3f7ccc7c7dfd4ULL, 0x807dbb0023c7c781ULL,
      0x13473d620bd5426cULL};

  explicit mac_codec(const std::uint64_t key[4] = default_key);

  void encode(std::string& data) override;

  void decode(std::string& data) override;

 private:
  std::uint64_t key_[4];
};
// constexpr std::uint64_t mac_codec::default_key[4];

/// Codec that applies Consistent Overhead Byte Stuffing.
class cobs_codec : public packet_codec {
 public:
  void encode(std::string& data) override;

  void decode(std::string& data) override;
};
}  // namespace ash

#endif  // LASR_PACKET_CODECS_H_
