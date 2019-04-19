/// \file
/// \brief Kitchen sink test binary.
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

#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include "ash.h"
#include "ash/address_resolver.h"
#include "ash/client.h"
#include "ash/connection.h"
#include "ash/context.h"
#include "ash/errors.h"
#include "ash/executor.h"
#include "ash/future.h"
#include "ash/highway_hash.h"
#include "ash/interface.h"
#include "ash/iostream_adapters.h"
#include "ash/mpt.h"
#include "ash/packet_codecs.h"
#include "ash/packet_protocols.h"
#include "ash/queue.h"
#include "ash/select.h"
#include "ash/serializable.h"
#include "ash/server.h"
#include "ash/socket.h"
#include "ash/string_adapters.h"
#include "ash/thread.h"
#include "ash/type_hash.h"

// clang-format off
ASH_INTERFACE(
    Reader, (),
    (
        // Get the value of a variable.
        ((std::string), get, (((const std::string&), name)))
    )
);

ASH_INTERFACE(
    Writer, ((Reader)),
    (
        // Set the value of a variable.
        ((void), set,
            (((const std::string&), name),
            ((const std::string&), value))),

        // Reset the server.
        ((void), clear, ())

    )
);
// clang-format on

template <typename R>
struct K : ash::serializable<K<R>> {
  R x = 1, y = 2;
  std::string z = "pasta";

  ASH_OWN_TYPE(K<R>);

  ASH_FIELDS(x, y, z);
};

struct V : ash::dynamic<V> {
  int a = 64;

  ASH_FIELDS(a);
};
ASH_REGISTER(V);

struct V2 : ash::dynamic<V2> {
  int a = 64;

  ASH_FIELDS(a);

  template <typename S>
  void save(S&) const {}

  template <typename S>
  void load(S&) {}

  ASH_CUSTOM_SERIALIZATION_VERSION(1);
};
ASH_REGISTER(V2);

struct X : ash::dynamic<X, V> {
  int x[10] = {1}, y = 2;
  std::string z = "pasta";

  ASH_FIELDS(x, y, z);
};
ASH_REGISTER(X);

struct Y : ash::serializable<Y> {
  int u = 32;

  ASH_FIELDS(u);
};
// ASH_REGISTER(Y);

namespace z {
struct Z : ash::dynamic<Z, X> {
  std::shared_ptr<Z> z2;
  std::shared_ptr<int> z3;

  ASH_FIELDS(z2, z3);
};
}  // namespace z
ASH_REGISTER(z::Z);

template <typename T>
void f() {
  std::cerr << "XXX: " << __PRETTY_FUNCTION__ << std::endl;
}

template <auto v>
void f() {
  std::cerr << "XXX: " << __PRETTY_FUNCTION__ << std::endl;
}

struct kk {
  static constexpr int roro = 1;
};

ASH_MAKE_NESTED_CONSTANT_CHECKER(has_roro, roro);

void xxd(const std::string& data) {
  std::size_t i = 0;
  while (i < data.size()) {
    std::cerr << std::hex << std::setfill('0') << std::setw(8) << i << ": ";

    for (std::size_t j = 0; j < 16; j++) {
      if (i + j < data.size()) {
        std::cerr << std::hex << std::setfill('0') << std::setw(2)
                  << static_cast<std::uint16_t>(
                         static_cast<std::uint8_t>(data[i + j]))
                  << ' ';

      } else {
        std::cerr << "   ";
      }
    }
    for (std::size_t j = 0; j < 16; j++) {
      if (i + j < data.size()) {
        auto c = data[i + j];
        if (c < ' ' || static_cast<int>(c) >= 0x80) {
          c = '.';
        }
        std::cerr << c;
      } else {
        std::cerr << ' ';
      }
    }
    std::cerr << std::dec << std::endl;
    i += 16;
  }
  std::cerr << std::endl;
}

struct bad_connection {
  template <auto mfp, typename... Args>
  auto call(Args&&... args) ->
      typename ash::traits::member_function_pointer_traits<mfp>::return_type {
    return (static_cast<typename ash::traits::member_function_pointer_traits<
                mfp>::class_type*>(nullptr)
                ->*mfp)(std::forward<Args>(args)...);
  }
};

struct WriterImpl : public Writer {
  std::string get(const std::string& name) override {
    std::string res = name + "_poo";
    std::cerr << "returning " << res << " for " << name << std::endl;
    return res;
  }

  void set(const std::string& name, const std::string& value) {
    std::cerr << "Set " << name << " = " << value << std::endl;
  }

  void clear() {
    throw ash::errors::invalid_argument("I don't like you!");
    std::cerr << "Clearing..." << std::endl;
  }
};

int main() {
  try {
    using namespace std::literals;

    ash::server_object<WriterImpl> writer;
    ash::server server({}, ash::endpoint().port(12121));
    server.register_object("test", writer);
    server.start();

    ash::client_connection conn(ash::endpoint().name("localhost").port(12121));
    auto obj = conn.get_proxy<Writer>("test");
    auto result = obj.get("patata");
    std::cerr << "RESULT: " << result << std::endl;
    obj.set("pollo", "vaca");
    obj.clear();

    server.stop();
    return 0;
  } catch (const ash::errors::base_error& e) {
    std::cerr << e.portable_error_class_name() << ": " << e.what() << std::endl;
    return 0;
  } catch (int x) {
    std::cerr << x << std::endl;
    return 0;
  }
  {
    std::cerr
        << "ms: "
        << ash::traits::type_hash_v<std::chrono::milliseconds> << std::endl;
    std::cerr << "m: "
              << ash::traits::type_hash_v<std::chrono::minutes> << std::endl;
    std::cerr << "sys: "
              << ash::traits::type_hash_v<
                     std::chrono::system_clock::time_point> << std::endl;
    std::cerr << "steady: "
              << ash::traits::type_hash_v<
                     std::chrono::steady_clock::time_point> << std::endl;
    std::cerr
        << "high-res: "
        << ash::traits::type_hash_v<
               std::chrono::high_resolution_clock::time_point> << std::endl;
  }
  {
    f<&Writer::get>();
    f<decltype(&Writer::get)>();
    f<&Writer::set>();
    f<decltype(&Writer::set)>();
  }
  {
    f<&Writer::async::get>();
    f<decltype(&Writer::async::get)>();
    f<&Writer::async::set>();
    f<decltype(&Writer::async::set)>();
  }
  {
    ash::promise<void> p1;
    auto f1 =
        p1.get_future().then([]() { std::cerr << "Was done!" << std::endl; });
    p1.set_value();
    f1.get();
  }
  {
    ash::promise<int> p1;
    auto f1 = p1.get_future().then([](int i) {
      std::cerr << "Was done!" << std::endl;
      return i + 1;
    });
    p1.set_value(3);
    std::cerr << "GOT: " << f1.get() << std::endl;
  }
  {
    ash::synchronous_executor x;
    for (int i = 0; i < 10; i++) {
      x.run([]() {
        std::cerr << "Hello "
                  << "world "
                  << "1" << std::endl;
      });
    }
  }
  {
    ash::thread_pool x;
    for (int i = 0; i < 10; i++) {
      x.run([]() {
        std::cerr << "Hello "
                  << "world "
                  << "2/" << ash::thread::hardware_concurrency() << " "
                  << std::this_thread::get_id() << std::endl;
      });
    }

    ash::select(ash::timeout(std::chrono::seconds(2)));
  }
  {
    ash::context ctx;

    V v;
    v.a = 33;

    V2 v2;
    v2.a = 65;

    ctx.set(std::move(v), std::move(v2));
    ctx.set_timeout(std::chrono::seconds(10));

    std::cerr << "V: " << ctx.get<V>().a << std::endl;
    std::cerr << "V2: " << ctx.get<V2>().a << std::endl;
    std::cerr << "X: " << ctx.get<X>().y << ", " << ctx.get<X>().z << std::endl;

    std::ostringstream oss;
    ash::ostream_output_stream osa(oss);
    ash::native_binary_encoder nbe(osa);
    nbe(ctx);
    xxd(oss.str());

    std::istringstream iss(oss.str());
    ash::istream_input_stream isa(iss);
    ash::native_binary_decoder nbd(isa);
    nbd(ctx);

    std::cerr << "V: " << ctx.get<V>().a << std::endl;
    std::cerr << "V2: " << ctx.get<V2>().a << std::endl;
    std::cerr << "X: " << ctx.get<X>().y << ", " << ctx.get<X>().z << std::endl;

    ctx.reset<V>();

    std::cerr << "V: " << ctx.get<V>().a << std::endl;
    std::cerr << "V2: " << ctx.get<V2>().a << std::endl;
    std::cerr << "X: " << ctx.get<X>().y << ", " << ctx.get<X>().z << std::endl;
  }
  {
    ash::promise<void> p1, p2;
    auto f1 = p1.get_future();
    auto f2 = p2.get_future();
    p2.set_value();
    std::vector<ash::awaitable<void>> conds;
    conds.push_back(f1.can_get());
    conds.push_back(f2.can_get());
    auto [got_fut, timeout] = ash::select(
        std::move(conds), ash::timeout(std::chrono::milliseconds(2000)));

    std::cerr << !!timeout << ',' << !!got_fut[0] << ',' << !!got_fut[1]
              << std::endl;
  }
  {
    ash::promise<void> p1, p2;
    auto f1 = p1.get_future();
    auto f2 = p2.get_future();
    std::vector<ash::awaitable<void>> conds;
    conds.push_back(f1.can_get());
    conds.push_back(f2.can_get());
    auto timeout_a = ash::timeout(std::chrono::milliseconds(2000));
    auto [got_fut, timeout] = ash::select(conds, timeout_a);

    std::cerr << !!timeout << ',' << !!got_fut[0] << ',' << !!got_fut[1]
              << std::endl;
  }
  {
    ash::promise<void> p;
    auto f = p.get_future();
    p.set_value();
    f.get();
  }
  {
    ash::context ctx;
    ctx.set_timeout(std::chrono::milliseconds(100));
    auto s = dial(ash::endpoint().name("www.google.com").service("https"));
  }
  {
    ash::listener l(ash::endpoint().port(13133));
    for (int i = 0; i < 4; i++) {
      auto s = l.accept();
      std::cerr << "Got connection from " << s.peer_addr().as_string()
                << " into " << s.own_addr().as_string() << std::endl;
    }
  }
  {
    ash::thread t1([]() {
      std::cerr << "CI " << &ash::context::current() << std::endl;
      ash::select(ash::context::current().wait_cancelled());
      std::cerr << "DONE!" << std::endl;
    });
    std::cerr << "CO " << &t1.get_context() << std::endl;
    t1.get_context().cancel();
    t1.join();
  }

  {
    std::cerr << *ash::context::current().deadline_left() /
                     std::chrono::milliseconds(1)
              << std::endl;

    {
      ash::context ctx2;
      ctx2.set_timeout(std::chrono::milliseconds(44));
      std::cerr << *ash::context::current().deadline_left() /
                       std::chrono::milliseconds(1)
                << std::endl;

      try {
        ash::select();
      } catch (const ash::errors::cancelled&) {
        std::cerr << "CANCELLED!" << std::endl;
      } catch (const ash::errors::deadline_exceeded&) {
        std::cerr << "DEADLINE!" << std::endl;
      }
    }

    std::cerr << *ash::context::current().deadline_left() /
                     std::chrono::milliseconds(1)
              << std::endl;
  }

  {
    ash::future<std::unique_ptr<int>> fi;
    ash::promise<std::unique_ptr<int>> pi;
    fi = pi.get_future();
    ash::thread th1([&]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(2000));
      pi.set_value(std::make_unique<int>(33));
    });

    auto [val, to] = ash::select(fi.async_get(),
                                 ash::timeout(std::chrono::milliseconds(3000)));
    if (val) {
      std::cerr << "VAL: " << **val << std::endl;
    }
    if (to) {
      std::cerr << "TIMED OUT" << std::endl;
    }

    th1.join();
  }

  ash::queue<std::unique_ptr<int>> q(10);

  auto tl = [&q]() {
    do {
      auto [get, timeout] = ash::select(
          q.async_get(), ash::timeout(std::chrono::milliseconds(3000)));
      if (get) {
        std::cerr << "Got! Here! " << **get << std::endl;
        q.put(std::make_unique<int>(**get + 1));
        return;
      } else if (timeout) {
        std::cerr << "1 Timed out!" << std::endl;
        return;
      }
    } while (true);
  };

  ash::thread th1(tl);
  ash::thread th2(tl);

  auto [put] = ash::select(q.async_put(std::make_unique<int>(32)));
  std::cerr << "DID " << !!put << " DID" << std::endl;

  th1.join();
  th2.join();

  std::cerr << *(q.get()) << " DONE!" << std::endl;

  ash::channel in(0);
  auto [read, timeout] =
      ash::select(in.can_read(), ash::timeout(std::chrono::milliseconds(3000)));
  std::cerr << !!read << !!timeout << std::endl;
  in.release();

  f<decltype(ash::mpt::as_tuple(ash::mpt::value_pack<33, 'c'>{}))>();

  std::cerr
      << "X"
      << ash::traits::member_function_pointer_traits<&Reader::get>::is_const
      << "X" << std::endl;

  try {
    ash::error_factory::get().throw_error("out_of_range", "It's way off!");
  } catch (const ash::errors::out_of_range& oor) {
    std::cerr << "CAUGHT! " << oor.portable_error_class_name() << ": "
              << oor.what() << std::endl;
  }

  ash::char_dev_connection cdc("/dev/tty");

  ash::thread t1([&cdc] {
    std::this_thread::sleep_for(std::chrono::seconds(10));
    cdc.disconnect();
  });

  try {
    std::string buf("hello\n");
    cdc.write(buf.data(), buf.size());
    cdc.read(&buf[0], 3);
    std::cerr << buf << std::endl;
  } catch (const ash::errors::base_error& sd) {
    std::cerr << "CAUGHT! " << sd.portable_error_class_name() << ": "
              << sd.what() << std::endl;
  }
  cdc.disconnect();

  t1.join();

  /*
  ash::packet_connection_impl<ash::char_dev_connection,
                              ash::serial_line_packet_protocol<>>
      slpci("/dev/tty");
  slpci.send("hello");
  slpci.disconnect();

  ash::reconnectable_connection<ash::char_dev_connection> cdc2("/dev/tty");
  std::string buf2("hello\n");
  cdc2.connect();
  cdc2.write(buf2.data(), buf2.size());
  cdc2.disconnect();
  cdc2.connect();
  cdc2.write(buf2.data(), buf2.size());
  cdc2.disconnect();

  ash::packet_connection_impl<
      ash::reconnectable_connection<ash::char_dev_connection>,
      ash::protected_stream_packet_protocol<>>
      slpci2("/dev/tty");
  slpci2.connect();
  slpci2.send("hello");
  slpci2.disconnect();
  slpci2.connect();
  slpci2.send("hello");
  slpci2.disconnect();
  */

  try {
    using namespace std::literals;
    ash::client_connection conn(ash::endpoint().name("localhost").port(9999));
    auto obj = conn.get_proxy<Reader::async>("default"sv);
    ash::future<std::string> res;
    {
      ash::context ctx;
      ctx.set_timeout(std::chrono::seconds(10));
      ctx.set(X());
      auto [fut, req_id] = obj.get("variable");
      conn.cancel_request(req_id);
      res = std::move(fut);
    }
    std::cerr << "XX" << res.get() << "XX" << std::endl;
  } catch (const std::runtime_error& e) {
    std::cerr << "CAUGHT: " << e.what() << std::endl;
  }

  std::uint64_t key[4] = {1, 2, 3, 4};

  std::string data =
      std::string("Hello") + std::string(2, '\0') + std::string("World");

  std::stringstream xs;
  ash::istream_input_stream xsi(xs);
  ash::ostream_output_stream xso(xs);
  ash::protected_stream_packet_protocol<ash::big_endian_binary_encoder,
                                        ash::big_endian_binary_decoder>
      pspp;
  xxd(data);
  pspp.send(xso, data);
  xxd(xs.str());
  data = pspp.receive(xsi);
  xxd(data);

  xs.str("");
  xs.seekg(0);
  xs.seekp(0);

  ash::serial_line_packet_protocol<> slpp;
  xxd(data);
  slpp.send(xso, data);
  xxd(xs.str());
  data = slpp.receive(xsi);
  xxd(data);

  ash::mac_codec mac(key);
  xxd(data);
  mac.encode(data);
  xxd(data);
  mac.decode(data);
  xxd(data);

  ash::mac_codec mac2;
  xxd(data);
  mac2.encode(data);
  xxd(data);
  mac2.decode(data);
  xxd(data);

  int index = 0;
  for (std::string s :
       {std::string(""), std::string(1, '\0'), std::string(2, '\0'),
        std::string(254, 'x'), std::string(1, '\0') + std::string(254, 'x'),
        std::string("Hello")}) {
    std::cerr << "TEST " << ++index << std::endl;
    ash::cobs_codec cobs;
    xxd(s);
    cobs.encode(s);
    xxd(s);
    cobs.decode(s);
    xxd(s);
  }

  std::cerr << ash::mpt::element_type_t<
                   0, ash::traits::get_field_descriptors_t<z::Z>>::type::name()
            << std::endl;
  std::cerr << ash::mpt::element_type_t<
                   1, ash::traits::get_field_descriptors_t<z::Z>>::type::name()
            << std::endl;

  std::cerr
      << ash::mpt::element_type_t<0, Writer::method_descriptors>::type::name()
      << std::endl;
  std::cerr
      << ash::mpt::element_type_t<1, Writer::method_descriptors>::type::name()
      << std::endl;

  try {
    std::string data = "Hello";
    std::cerr << std::hex
              << ash::highway_hash::hash64(
                     reinterpret_cast<const uint8_t*>(data.data()), data.size(),
                     key)
              << std::dec << std::endl;
    data = "Hell!";
    std::cerr << std::hex
              << ash::highway_hash::hash64(
                     reinterpret_cast<const uint8_t*>(data.data()), data.size(),
                     key)
              << std::dec << std::endl;
    f<decltype(kk::roro)>();
    std::cerr << has_roro<kk, int>::value << std::endl;
    std::cerr << has_roro<Y, int>::value << std::endl;

    using A = ash::mpt::pack<>;
    using B = ash::mpt::insert_type_into_t<int, A>;
    using C = ash::mpt::insert_type_into_t<int, B>;
    using D = ash::mpt::insert_type_into_t<double, C>;
    using E = ash::mpt::insert_type_into_t<int, D>;
    f<E>();

    std::cerr << std::hex;
    std::cerr
        << ash::traits::get_custom_serialization_version<signed int>::value
        << std::endl;

    std::cerr << ash::traits::type_hash<signed int>::value << std::endl;
    std::cerr << ash::traits::type_hash<std::tuple<int>>::value << std::endl;

    std::cerr << ash::traits::type_hash<V>::value << std::endl;
    std::cerr << ash::traits::type_hash<V2>::value << std::endl;
    std::cerr << ash::traits::type_hash<X>::value << std::endl;
    std::cerr << ash::traits::type_hash<Y>::value << std::endl;
    std::cerr << ash::traits::type_hash<z::Z>::value << std::endl;
    std::cerr << ash::traits::type_hash<std::shared_ptr<z::Z>>::value
              << std::endl;
    std::cerr << std::dec;

    int h1 = 333;
    double h2 = 444;
    std::tuple<const int&, const double&> tup(h1, h2);
    std::shared_ptr<X> x(new X());
    x->x[0] = 44;
    x->a = 88;
    std::shared_ptr<V> v = x;
    std::weak_ptr<V> w = v;
    std::unique_ptr<Y> y(new Y());
    std::shared_ptr<z::Z> z(new z::Z());
    z->z2 = z;
    z->z = "rosco";

    ash::binary_sizer bs;
    bs(tup, ash::verify_structure{});
    bs(x);
    bs(v);
    bs(v);
    bs(w);
    bs(y);
    bs(z, ash::verify_structure{});
    std::cerr << "SIZE: " << bs.size() << std::endl;

    std::ostringstream oss;
    ash::ostream_output_stream osa(oss);
    ash::native_binary_encoder nbe(osa);
    nbe(tup, ash::verify_structure{});
    nbe(x);
    nbe(v);
    nbe(v);
    nbe(w);
    nbe(y);
    nbe(z, ash::verify_structure{});

    std::istringstream iss(oss.str());
    ash::istream_input_stream isa(iss);
    ash::native_binary_decoder nbd(isa);

    std::tuple<int, double> tup2;
    std::shared_ptr<X> x2;
    std::shared_ptr<V> v2;
    std::weak_ptr<V> w2;
    std::unique_ptr<Y> y2;
    std::shared_ptr<z::Z> z2;

    nbd(tup2, ash::verify_structure{});
    nbd(x2);
    nbd(v2);
    nbd(v2);
    nbd(w2);
    nbd(y2);
    nbd(z2, ash::verify_structure{});

    std::cerr << std::get<0>(tup2) << ", " << std::get<1>(tup2) << std::endl;

    std::cerr << x2->x << ", " << x2->a << std::endl;
    std::cerr << std::static_pointer_cast<X>(v2)->x << ", "
              << std::static_pointer_cast<X>(v2)->a << std::endl;

    std::cerr << (z2 == z2->z2) << std::endl;
    std::cerr << (z2 == z) << std::endl;

    std::cout << oss.str();

    throw ash::errors::io_error("hello");
  } catch (const std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
