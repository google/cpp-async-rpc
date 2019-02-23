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

#include "address.h"
#include "ash/errors.h"

namespace ash {

address_spec& address_spec::name(const std::string& new_name) {
  name_ = new_name;
  return *this;
}
address_spec& address_spec::service(const std::string& new_service) {
  service_ = new_service;
  return *this;
}
address_spec& address_spec::port(int new_port) {
  service_ = std::to_string(new_port);
  return *this;
}
address_spec& address_spec::passive() {
  passive_ = true;
  return *this;
}
address_spec& address_spec::active() {
  passive_ = false;
  return *this;
}
address_spec& address_spec::stream() {
  sock_type_ = SOCK_STREAM;
  return *this;
}
address_spec& address_spec::datagram() {
  sock_type_ = SOCK_DGRAM;
  return *this;
}
address_spec& address_spec::ipv4() {
  family_ = AF_INET;
  return *this;
}
address_spec& address_spec::ipv6() {
  family_ = AF_INET6;
  return *this;
}
address_spec& address_spec::ip() {
  family_ = AF_UNSPEC;
  return *this;
}

int address::family() const { return ai_family; }
int address::socket_type() const { return ai_socktype; }
int address::protocol() const { return ai_protocol; }
const struct sockaddr* address::address_data() const { return ai_addr; }
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
