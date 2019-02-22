/// \file
/// \brief Wrapper for name resolution results.
///
/// \copyright
///   Copyright 2018 by Google Inc. All Rights Reserved.
///
/// \copyright
///   Licensed under the Apache License, Version 2.0 (the "License"); you may
///   not use this channel except in compliance with the License. You may obtain
///   a copy of the License at
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

#ifndef ASH_ADDRESS_INFO_H_
#define ASH_ADDRESS_INFO_H_

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstddef>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ash {

class address_info {
 public:
  using value_type = struct addrinfo;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;

  class const_iterator;

  class iterator {
   public:
    iterator() noexcept : ptr_(nullptr) {}
    iterator(const iterator&) = default;
    value_type& operator*() const noexcept { return *ptr_; }
    iterator& operator++() noexcept {
      ptr_ = ptr_->ai_next;
      return *this;
    }
    iterator operator++(int) noexcept {
      iterator res(*this);
      ptr_ = ptr_->ai_next;
      return res;
    }
    void swap(iterator& o) noexcept { std::swap(ptr_, o.ptr_); }
    bool operator==(const iterator& o) const noexcept { return ptr_ == o.ptr_; }
    bool operator!=(const iterator& o) const noexcept { return ptr_ != o.ptr_; }
    value_type* operator->() const noexcept { return ptr_; }

   private:
    friend class address_info;
    friend class address_info::const_iterator;
    explicit iterator(value_type* ptr) : ptr_(ptr) {}

    value_type* ptr_;
  };

  class const_iterator {
   public:
    const_iterator() noexcept : ptr_(nullptr) {}
    const_iterator(const const_iterator&) noexcept = default;
    const_iterator(const iterator& o) noexcept : ptr_(o.ptr_) {}
    const value_type& operator*() const noexcept { return *ptr_; }
    const_iterator& operator++() noexcept {
      ptr_ = ptr_->ai_next;
      return *this;
    }
    const_iterator operator++(int) noexcept {
      const_iterator res(*this);
      ptr_ = ptr_->ai_next;
      return res;
    }
    void swap(const_iterator& o) noexcept { std::swap(ptr_, o.ptr_); }
    bool operator==(const const_iterator& o) const noexcept {
      return ptr_ == o.ptr_;
    }
    bool operator!=(const const_iterator& o) const noexcept {
      return ptr_ != o.ptr_;
    }
    const value_type* operator->() const noexcept { return ptr_; }

   private:
    friend class address_info;
    explicit const_iterator(const value_type* ptr) : ptr_(ptr) {}

    const value_type* ptr_;
  };

  iterator begin() noexcept;
  const_iterator begin() const noexcept;
  const_iterator cbegin() const noexcept;
  iterator end() noexcept;
  const_iterator end() const noexcept;
  const_iterator cend() const noexcept;

  bool empty() const noexcept;
  size_type size() const noexcept;

  address_info(address_info&&) = default;
  address_info& operator=(address_info&&) = default;

  ~address_info() = default;

  static std::string to_string(const struct addrinfo& addr);

 private:
  friend class address_resolver;
  explicit address_info(struct addrinfo* result);
  static void free_result(struct addrinfo* result);
  std::unique_ptr<struct addrinfo, void (*)(struct addrinfo*)> result_;
};

void swap(address_info::iterator& a, address_info::iterator& b) noexcept;

void swap(address_info::const_iterator& a,
          address_info::const_iterator& b) noexcept;

}  // namespace ash

namespace std {
template <>
struct iterator_traits<::ash::address_info::iterator> {
  using value_type = struct addrinfo;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type*;
  using reference = value_type&;
  using iterator_category = std::forward_iterator_tag;
};

template <>
struct iterator_traits<::ash::address_info::const_iterator> {
  using value_type = const struct addrinfo;
  using difference_type = std::ptrdiff_t;
  using pointer = const value_type*;
  using reference = const value_type&;
  using iterator_category = std::forward_iterator_tag;
};

}  // namespace std
#endif  // ASH_ADDRESS_INFO_H_
