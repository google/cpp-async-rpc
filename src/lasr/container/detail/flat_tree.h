/// \file
/// \brief Internal shared implementation for `flat_map` and `flat_set`.
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

#ifndef LASR_CONTAINER_DETAIL_FLAT_TREE_H_
#define LASR_CONTAINER_DETAIL_FLAT_TREE_H_

#include <algorithm>
#include <functional>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace lasr {
namespace detail {

template <typename Key, typename Value, typename Compare = std::less<Key> >
struct key_value_compare {
  explicit key_value_compare(const Compare& comp = Compare()) : comp_(comp) {}

  using value_type = Value;
  using key_type = Key;

  bool operator()(const key_type& left, const key_type& right) const {
    return comp_(left, right);
  }

  bool operator()(const value_type& left, const value_type& right) const {
    return comp_(left.first, right.first);
  }

  bool operator()(const value_type& left, const key_type& right) const {
    return comp_(left.first, right);
  }

  bool operator()(const key_type& left, const value_type& right) const {
    return comp_(left, right.first);
  }

  Compare comp_;
};

template <typename Key, typename Compare>
struct key_value_compare<Key, Key, Compare> {
  explicit key_value_compare(const Compare& comp = Compare()) : comp_(comp) {}

  using value_type = Key;
  using key_type = Key;

  bool operator()(const key_type& left, const key_type& right) const {
    return comp_(left, right);
  }

  Compare comp_;
};

template <typename Key, typename Value, typename Compare = std::less<Key> >
struct key_value_equal {
  explicit key_value_equal(const Compare& comp = Compare()) : comp_(comp) {}

  using value_type = Value;
  using key_type = Key;

  bool operator()(const key_type& left, const key_type& right) const {
    return !comp_(left, right) && !comp_(right, left);
  }

  bool operator()(const value_type& left, const value_type& right) const {
    return !comp_(left.first, right.first) && !comp_(right.first, left.first);
  }

  bool operator()(const value_type& left, const key_type& right) const {
    return !comp_(left.first, right) && !comp_(right, left.first);
  }

  bool operator()(const key_type& left, const value_type& right) const {
    return !comp_(left, right.first) && !comp_(right.first, left);
  }

  Compare comp_;
};

template <typename Key, typename Compare>
struct key_value_equal<Key, Key, Compare> {
  explicit key_value_equal(const Compare& comp = Compare()) : comp_(comp) {}

  using value_type = Key;
  using key_type = Key;

  bool operator()(const key_type& left, const key_type& right) const {
    return !comp_(left, right) && !comp_(right, left);
  }

  Compare comp_;
};

template <typename Key, bool multiple_allowed, typename Value, typename Compare,
          typename Allocator>
class flat_tree : protected std::vector<Value, Allocator> {
 protected:
  using value_equal = key_value_equal<Key, Value, Compare>;

 public:
  using underlying_container = std::vector<Value, Allocator>;
  using key_type = Key;
  using value_type = Value;
  using typename underlying_container::size_type;
  using typename underlying_container::difference_type;
  using key_compare = Compare;
  using value_compare = key_value_compare<Key, Value, Compare>;

  using typename underlying_container::allocator_type;
  using typename underlying_container::reference;
  using typename underlying_container::const_reference;
  using typename underlying_container::pointer;
  using typename underlying_container::const_pointer;
  using typename underlying_container::iterator;
  using typename underlying_container::const_iterator;
  using typename underlying_container::reverse_iterator;
  using typename underlying_container::const_reverse_iterator;

  using underlying_container::begin;
  using underlying_container::cbegin;
  using underlying_container::end;
  using underlying_container::cend;
  using underlying_container::rbegin;
  using underlying_container::crbegin;
  using underlying_container::rend;
  using underlying_container::crend;

  using underlying_container::get_allocator;

  using underlying_container::empty;
  using underlying_container::size;
  using underlying_container::max_size;
  using underlying_container::reserve;
  using underlying_container::capacity;
  using underlying_container::shrink_to_fit;

  using underlying_container::clear;
  using underlying_container::swap;

  // swap.
  void swap(flat_tree& other) { underlying_container::swap(other); }

  // Assignment.
  flat_tree& operator=(const flat_tree& other) {
    underlying_container::operator=(other);
    return *this;
  }
  flat_tree& operator=(flat_tree&& other) {
    underlying_container::operator=(std::forward<flat_tree>(other));
    return *this;
  }
  flat_tree& operator=(std::initializer_list<value_type> ilist) {
    underlying_container::operator=(ilist);
    std::sort(begin(), end(), comp_);
    if (!multiple_allowed) {
      this->erase(std::unique(begin(), end(), eq_), end());
    }
    return *this;
  }

  // Constructors.
  explicit flat_tree(const Compare& comp, const Allocator& alloc = Allocator())
      : underlying_container(alloc), comp_(comp), eq_(comp) {}
  explicit flat_tree(const Allocator& alloc = Allocator())
      : underlying_container(alloc) {}
  template <typename InputIt>
  flat_tree(InputIt first, InputIt last, const Compare& comp = Compare(),
            const Allocator& alloc = Allocator())
      : underlying_container(first, last, alloc), comp_(comp), eq_(comp) {
    std::sort(begin(), end(), comp_);
    if (!multiple_allowed) {
      this->erase(std::unique(begin(), end(), eq_), end());
    }
  }
  flat_tree(const flat_tree& other)
      : underlying_container(static_cast<const underlying_container&>(other)) {}
  flat_tree(const flat_tree& other, const Allocator& alloc)
      : underlying_container(static_cast<const underlying_container&>(other),
                             alloc) {}
  flat_tree(flat_tree&& other)
      : underlying_container(static_cast<const underlying_container&&>(other)) {
  }
  flat_tree(flat_tree&& other, const Allocator& alloc)
      : underlying_container(static_cast<const underlying_container&&>(other),
                             alloc) {}
  flat_tree(std::initializer_list<value_type> init,
            const Compare& comp = Compare(),
            const Allocator& alloc = Allocator())
      : underlying_container(init, alloc), comp_(comp), eq_(comp) {
    std::sort(begin(), end(), comp_);
    if (!multiple_allowed) {
      this->erase(std::unique(begin(), end(), eq_), end());
    }
  }

  // find/bounds
  iterator upper_bound(const Key& key) {
    return std::upper_bound(begin(), end(), key, comp_);
  }
  const_iterator upper_bound(const Key& key) const {
    return std::upper_bound(begin(), end(), key, comp_);
  }
  iterator lower_bound(const Key& key) {
    return std::lower_bound(begin(), end(), key, comp_);
  }
  const_iterator lower_bound(const Key& key) const {
    return std::lower_bound(begin(), end(), key, comp_);
  }
  std::pair<iterator, iterator> equal_range(const Key& key) {
    return std::equal_range(begin(), end(), key, comp_);
  }
  std::pair<const_iterator, const_iterator> equal_range(const Key& key) const {
    return std::equal_range(begin(), end(), key, comp_);
  }

  // insert
  std::pair<iterator, bool> insert(const value_type& value) {
    auto it = std::lower_bound(begin(), end(), value, comp_);
    if (!multiple_allowed && it != end() && eq_(*it, value)) {
      return {it, false};
    }
    it = underlying_container::insert(it, value);
    return {it, true};
  }
  std::pair<iterator, bool> insert(value_type&& value) {
    auto it = std::lower_bound(begin(), end(), value, comp_);
    if (!multiple_allowed && it != end() && eq_(*it, value)) {
      return {it, false};
    }
    it = underlying_container::insert(it, std::forward<value_type>(value));
    return {it, true};
  }
  iterator insert(const_iterator hint, const value_type& value) {
    return insert(value).first;
  }
  iterator insert(const_iterator hint, value_type&& value) {
    return insert(std::forward<value_type>(value)).first;
  }
  template <typename InputIt>
  void insert(InputIt first, InputIt last) {
    for (auto it = first; it != last; it++) {
      insert(*it);
    }
  }
  void insert(std::initializer_list<value_type> ilist) {
    insert(ilist.begin(), ilist.end());
  }

  // emplace
  template <typename... Args>
  std::pair<iterator, bool> emplace(Args&&... args) {
    return insert(std::move(value_type(std::forward<Args>(args)...)));
  }
  template <class... Args>
  iterator emplace_hint(const_iterator hint, Args&&... args) {
    return insert(std::move(value_type(std::forward<Args>(args)...)));
  }

  // erase
  iterator erase(const_iterator pos) {
    return underlying_container::erase(pos);
  }
  iterator erase(const_iterator first, const_iterator last) {
    return underlying_container::erase(first, last);
  }
  size_type erase(const key_type& key) {
    auto iters = equal_range(key);
    auto result = iters.second - iters.first;
    erase(iters.first, iters.second);
    return result;
  }

  // count.
  size_type count(const Key& key) const {
    auto iters = equal_range(key);
    return iters.second - iters.first;
  }

  // find.
  iterator find(const Key& key) {
    auto it = lower_bound(key);
    if (it != end() && eq_(*it, key)) {
      return it;
    }
    return end();
  }
  const_iterator find(const Key& key) const {
    auto it = lower_bound(key);
    if (it != end() && eq_(*it, key)) {
      return it;
    }
    return end();
  }

  // compare accessors.
  key_compare key_comp() const { return comp_.comp_; }
  value_compare value_comp() const { return comp_; }

 protected:
  value_compare comp_;
  value_equal eq_;
};

}  // namespace detail
}  // namespace lasr

#endif  // LASR_CONTAINER_DETAIL_FLAT_TREE_H_
