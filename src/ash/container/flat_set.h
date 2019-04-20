/// \file
/// \brief `std::vector`-backed `set` and `multiset`.
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

#ifndef LASR_CONTAINER_FLAT_SET_H_
#define LASR_CONTAINER_FLAT_SET_H_

#include <functional>
#include <memory>

#include "ash/container/detail/flat_tree.h"

namespace ash {

/// \brief `std::vector`-backed replacement for `std::set`.
///
/// These replacement containers do binary search over vectors to find the
/// required keys and perform fewer allocations than standard containers, which
/// is advantageous for embedded environments.
///
/// The trade-off is slower insertion due to vector tail element copy.
template <typename Key, typename Compare = std::less<Key>,
          typename Allocator = std::allocator<Key> >
class flat_set : public detail::flat_tree<Key, false, Key, Compare, Allocator> {
 public:
  // Inherit constructors.
  using detail::flat_tree<Key, false, Key, Compare, Allocator>::flat_tree;
};

/// \brief `std::vector`-backed replacement for `std::multiset`.
///
/// These replacement containers do binary search over vectors to find the
/// required keys and perform fewer allocations than standard containers, which
/// is advantageous for embedded environments.
///
/// The trade-off is slower insertion due to vector tail element copy.
template <typename Key, typename Compare = std::less<Key>,
          typename Allocator = std::allocator<Key> >
class flat_multiset
    : public detail::flat_tree<Key, true, Key, Compare, Allocator> {
 public:
  // Inherit constructors.
  using detail::flat_tree<Key, true, Key, Compare, Allocator>::flat_tree;
};

}  // namespace ash

#endif  // LASR_CONTAINER_FLAT_SET_H_
