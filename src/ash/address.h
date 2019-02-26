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

#ifndef ASH_ADDRESS_H_
#define ASH_ADDRESS_H_

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

class endpoint {
 public:
  endpoint& name(const std::string& new_name);
  endpoint& service(const std::string& new_service);
  endpoint& port(int new_port);
  endpoint& passive();
  endpoint& active();
  endpoint& stream();
  endpoint& datagram();
  endpoint& ip();
  endpoint& ipv4();
  endpoint& ipv6();

 private:
  friend class address_resolver;

  std::string name_;
  std::string service_;
  bool passive_ = false;
  int family_ = AF_UNSPEC;
  int sock_type_ = SOCK_STREAM;
};

class address;

class address_list {
 public:
  using value_type = struct address;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;

  class const_iterator;

  class iterator {
   public:
    iterator() noexcept;
    iterator(const iterator&) = default;
    value_type& operator*() const noexcept;
    iterator& operator++() noexcept;
    iterator operator++(int) noexcept;
    void swap(iterator& o) noexcept;
    bool operator==(const iterator& o) const noexcept;
    bool operator!=(const iterator& o) const noexcept;
    value_type* operator->() const noexcept;

   private:
    friend class address_list;
    friend class address_list::const_iterator;
    explicit iterator(value_type* ptr);

    value_type* ptr_;
  };

  class const_iterator {
   public:
    const_iterator() noexcept;
    const_iterator(const const_iterator&) noexcept = default;
    const_iterator(const iterator& o) noexcept;
    const value_type& operator*() const noexcept;
    const_iterator& operator++() noexcept;
    const_iterator operator++(int) noexcept;
    void swap(const_iterator& o) noexcept;
    bool operator==(const const_iterator& o) const noexcept;
    bool operator!=(const const_iterator& o) const noexcept;
    const value_type* operator->() const noexcept;

   private:
    friend class address_list;
    explicit const_iterator(const value_type* ptr);

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

  address_list(address_list&&) = default;
  address_list& operator=(address_list&&) = default;

  ~address_list() = default;

 private:
  friend class address_resolver;
  explicit address_list(struct addrinfo* result);
  static void free_result(address* result);
  std::unique_ptr<address, void (*)(address*)> result_;
};

void swap(address_list::iterator& a, address_list::iterator& b) noexcept;

void swap(address_list::const_iterator& a,
          address_list::const_iterator& b) noexcept;

class address : private addrinfo {
 public:
  address();
  address(const address& other);
  address(address&& other);
  ~address();

  int& family();
  int family() const;
  int& socket_type();
  int socket_type() const;
  int& protocol();
  int protocol() const;
  struct sockaddr* address_data();
  const struct sockaddr* address_data() const;
  socklen_t& address_size();
  socklen_t address_size() const;
  std::string as_string() const;

 private:
  friend class address_list;
  friend class address_list::iterator;
  friend class address_list::const_iterator;
};

static_assert(
    sizeof(address) == sizeof(struct addrinfo),
    "ash::address has to remain data-compatible with struct addrinfo");

}  // namespace ash

namespace std {
template <>
struct iterator_traits<::ash::address_list::iterator> {
  using value_type = ::ash::address;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type*;
  using reference = value_type&;
  using iterator_category = std::forward_iterator_tag;
};

template <>
struct iterator_traits<::ash::address_list::const_iterator> {
  using value_type = const ::ash::address;
  using difference_type = std::ptrdiff_t;
  using pointer = const value_type*;
  using reference = const value_type&;
  using iterator_category = std::forward_iterator_tag;
};

}  // namespace std
#endif  // ASH_ADDRESS_H_
