/// \file
/// \brief Wrapper for name resolution results.
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

#include "ash/address.h"
#include <cassert>
#include <cstring>
#include "ash/errors.h"

namespace ash {

endpoint& endpoint::name(const std::string& new_name) {
  name_ = new_name;
  return *this;
}
endpoint& endpoint::service(const std::string& new_service) {
  service_ = new_service;
  return *this;
}
endpoint& endpoint::port(int new_port) {
  service_ = std::to_string(new_port);
  return *this;
}
endpoint& endpoint::passive() {
  passive_ = true;
  return *this;
}
endpoint& endpoint::active() {
  passive_ = false;
  return *this;
}
endpoint& endpoint::stream() {
  sock_type_ = SOCK_STREAM;
  return *this;
}
endpoint& endpoint::datagram() {
  sock_type_ = SOCK_DGRAM;
  return *this;
}
endpoint& endpoint::ipv4() {
  family_ = AF_INET;
  return *this;
}
endpoint& endpoint::ipv6() {
  family_ = AF_INET6;
  return *this;
}
endpoint& endpoint::ip() {
  family_ = AF_UNSPEC;
  return *this;
}

address::address()
    : addrinfo{
          0,
          0,
          0,
          0,
          sizeof(struct sockaddr_storage),
          reinterpret_cast<struct sockaddr*>(new struct sockaddr_storage()),
          nullptr,
          this} {}

address::address(const address& other)
    : addrinfo{
          other.ai_flags,
          other.ai_family,
          other.ai_socktype,
          other.ai_protocol,
          other.ai_addrlen,
          reinterpret_cast<struct sockaddr*>(new struct sockaddr_storage()),
          nullptr,
          this} {
  memcpy(ai_addr, other.ai_addr, ai_addrlen);
}

address::address(address&& other)
    : addrinfo{other.ai_flags,
               other.ai_family,
               other.ai_socktype,
               other.ai_protocol,
               0,
               nullptr,
               nullptr,
               this} {
  assert(other.ai_next == &other);
  std::swap(ai_addr, other.ai_addr);
  std::swap(ai_addrlen, other.ai_addrlen);
}

address::~address() {
  assert(ai_next == this);
  delete ai_addr;
}

int& address::family() { return ai_family; }
int address::family() const { return ai_family; }
int& address::socket_type() { return ai_socktype; }
int address::socket_type() const { return ai_socktype; }
int& address::protocol() { return ai_protocol; }
int address::protocol() const { return ai_protocol; }
struct sockaddr* address::address_data() {
  return ai_addr;
}
const struct sockaddr* address::address_data() const { return ai_addr; }
socklen_t& address::address_size() { return ai_addrlen; }
socklen_t address::address_size() const { return ai_addrlen; }

void swap(address_list::iterator& a, address_list::iterator& b) noexcept {
  a.swap(b);
}

void swap(address_list::const_iterator& a,
          address_list::const_iterator& b) noexcept {
  a.swap(b);
}

bool address_list::empty() const noexcept { return begin() == end(); }
address_list::size_type address_list::size() const noexcept {
  return std::distance(begin(), end());
}

address_list::iterator address_list::begin() noexcept {
  return iterator(result_.get());
}
address_list::const_iterator address_list::begin() const noexcept {
  return const_iterator(result_.get());
}
address_list::const_iterator address_list::cbegin() const noexcept {
  return const_iterator(result_.get());
}

address_list::iterator address_list::end() noexcept {
  return iterator(nullptr);
}
address_list::const_iterator address_list::end() const noexcept {
  return const_iterator(nullptr);
}
address_list::const_iterator address_list::cend() const noexcept {
  return const_iterator(nullptr);
}

address_list::address_list(struct addrinfo* result)
    : result_(static_cast<address*>(result), &free_result) {}

void address_list::free_result(address* result) { freeaddrinfo(result); }

std::string address::as_string() const {
  char hostbuf[65];
  char portbuf[6];
  int res = getnameinfo(ai_addr, ai_addrlen, hostbuf, sizeof(hostbuf), portbuf,
                        sizeof(portbuf), NI_NUMERICHOST | NI_NUMERICSERV);
  if (res) throw_io_error("Can't print address as string", res);
  std::string host(hostbuf);
  if (host.find(':') != std::string::npos) {
    return "[" + std::string(hostbuf) + "]:" + std::string(portbuf);
  } else {
    return std::string(hostbuf) + ":" + std::string(portbuf);
  }
}

address_list::iterator::iterator() noexcept : ptr_(nullptr) {}
address_list::value_type& address_list::iterator::operator*() const noexcept {
  return *ptr_;
}
address_list::iterator& address_list::iterator::operator++() noexcept {
  ptr_ = static_cast<address*>(ptr_->ai_next);
  return *this;
}
address_list::iterator address_list::iterator::operator++(int) noexcept {
  iterator res(*this);
  ptr_ = static_cast<address*>(ptr_->ai_next);
  return res;
}
void address_list::iterator::swap(address_list::iterator& o) noexcept {
  std::swap(ptr_, o.ptr_);
}
bool address_list::iterator::operator==(const address_list::iterator& o) const
    noexcept {
  return ptr_ == o.ptr_;
}
bool address_list::iterator::operator!=(const address_list::iterator& o) const
    noexcept {
  return ptr_ != o.ptr_;
}
address_list::value_type* address_list::iterator::operator->() const noexcept {
  return ptr_;
}
address_list::iterator::iterator(address_list::value_type* ptr) : ptr_(ptr) {}

address_list::const_iterator::const_iterator() noexcept : ptr_(nullptr) {}
address_list::const_iterator::const_iterator(const iterator& o) noexcept
    : ptr_(o.ptr_) {}
const address_list::value_type& address_list::const_iterator::operator*() const
    noexcept {
  return *ptr_;
}
address_list::const_iterator& address_list::const_iterator::
operator++() noexcept {
  ptr_ = static_cast<const address*>(ptr_->ai_next);
  return *this;
}
address_list::const_iterator address_list::const_iterator::operator++(
    int) noexcept {
  const_iterator res(*this);
  ptr_ = static_cast<const address*>(ptr_->ai_next);
  return res;
}
void address_list::const_iterator::swap(
    address_list::const_iterator& o) noexcept {
  std::swap(ptr_, o.ptr_);
}
bool address_list::const_iterator::operator==(
    const address_list::const_iterator& o) const noexcept {
  return ptr_ == o.ptr_;
}
bool address_list::const_iterator::operator!=(
    const address_list::const_iterator& o) const noexcept {
  return ptr_ != o.ptr_;
}
const address_list::value_type* address_list::const_iterator::operator->() const
    noexcept {
  return ptr_;
}
address_list::const_iterator::const_iterator(
    const address_list::value_type* ptr)
    : ptr_(ptr) {}
}  // namespace ash
