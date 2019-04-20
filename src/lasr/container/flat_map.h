/// \file
/// \brief `std::vector`-backed `map` and `multimap`.
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

#ifndef LASR_CONTAINER_FLAT_MAP_H_
#define LASR_CONTAINER_FLAT_MAP_H_

#include <functional>
#include <memory>
#include <utility>

#include "lasr/container/detail/flat_tree.h"

namespace lasr {

/// \brief `std::vector`-backed replacement for `std::map`.
///
/// These replacement containers do binary search over vectors to find the
/// required keys and perform fewer allocations than standard containers, which
/// is advantageous for embedded environments.
///
/// The trade-off is slower insertion due to vector tail element copy.
template <typename Key, typename T, typename Compare = std::less<Key>,
          typename Allocator = std::allocator<std::pair<Key, T> > >
class flat_map : public detail::flat_tree<Key, false, std::pair<Key, T>,
                                          Compare, Allocator> {
 public:
  using typename detail::flat_tree<Key, false, std::pair<Key, T>, Compare,
                                   Allocator>::underlying_container;
  using mapped_type = T;

  // Inherit constructors.
  using detail::flat_tree<Key, false, std::pair<Key, T>, Compare,
                          Allocator>::flat_tree;

  // operator []
  T& operator[](const Key& key) {
    auto it = this->lower_bound(key);
    if (it != this->end() && this->eq_(*it, key)) {
      return it->second;
    }
    it = underlying_container::emplace(it, key, T());
    return it->second;
  }
  T& operator[](Key&& key) {
    auto it = this->lower_bound(key);
    if (it != this->end() && this->eq_(*it, key)) {
      return it->second;
    }
    it = underlying_container::emplace(it, std::forward<Key>(key), T());
    return it->second;
  }

  // at
  T& at(const Key& key) {
    auto it = this->find(key);
    if (it == this->end()) {
      throw std::out_of_range("key not found.");
    }
    return it->second;
  }
  const T& at(const Key& key) const {
    auto it = this->find(key);
    if (it == this->end()) {
      throw std::out_of_range("key not found.");
    }
    return it->second;
  }
};

/// \brief `std::vector`-backed replacement for `std::multimap`.
///
/// These replacement containers do binary search over vectors to find the
/// required keys and perform fewer allocations than standard containers, which
/// is advantageous for embedded environments.
///
/// The trade-off is slower insertion due to vector tail element copy.
template <typename Key, typename T, typename Compare = std::less<Key>,
          typename Allocator = std::allocator<std::pair<Key, T> > >
class flat_multimap : public detail::flat_tree<Key, true, std::pair<Key, T>,
                                               Compare, Allocator> {
 public:
  using mapped_type = T;

  // Inherit constructors.
  using detail::flat_tree<Key, true, std::pair<Key, T>, Compare,
                          Allocator>::flat_tree;
};

}  // namespace lasr

#endif  // LASR_CONTAINER_FLAT_MAP_H_
