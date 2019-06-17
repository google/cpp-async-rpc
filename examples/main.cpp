/// \file
/// \brief Kitchen sink test binary.
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

#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include "arpc/address_resolver.h"
#include "arpc/client.h"
#include "arpc/connection.h"
#include "arpc/context.h"
#include "arpc/errors.h"
#include "arpc/executor.h"
#include "arpc/future.h"
#include "arpc/highway_hash.h"
#include "arpc/interface.h"
#include "arpc/iostream_adapters.h"
#include "arpc/mpt.h"
#include "arpc/packet_codecs.h"
#include "arpc/packet_protocols.h"
#include "arpc/queue.h"
#include "arpc/select.h"
#include "arpc/serializable.h"
#include "arpc/server.h"
#include "arpc/socket.h"
#include "arpc/string_adapters.h"
#include "arpc/thread.h"
#include "arpc/type_hash.h"

// clang-format off
ARPC_INTERFACE(
    Reader, (),
    (
        // Get the value of a variable.
        ((std::string), get, (((const std::string&), name)))
    )
);

ARPC_INTERFACE(
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
struct K : arpc::serializable<K<R>> {
  R x = 1, y = 2;
  std::string z = "pasta";

  ARPC_OWN_TYPE(K<R>);

  ARPC_FIELDS(x, y, z);
};

struct V : arpc::dynamic<V> {
  int a = 64;

  ARPC_FIELDS(a);
};
ARPC_REGISTER(V);

struct V2 : arpc::dynamic<V2> {
  int a = 64;

  ARPC_FIELDS(a);

  template <typename S>
  void save(S&) const {}

  template <typename S>
  void load(S&) {}

  ARPC_CUSTOM_SERIALIZATION_VERSION(1);
};
ARPC_REGISTER(V2);

struct X : arpc::dynamic<X, V> {
  int x[10] = {1}, y = 2;
  std::string z = "pasta";

  ARPC_FIELDS(x, y, z);
};
ARPC_REGISTER(X);

struct Y : arpc::serializable<Y> {
  int u = 32;

  ARPC_FIELDS(u);
};
// ARPC_REGISTER(Y);

namespace z {
struct Z : arpc::dynamic<Z, X> {
  std::shared_ptr<Z> z2;
  std::shared_ptr<int> z3;

  ARPC_FIELDS(z2, z3);
};
}  // namespace z
ARPC_REGISTER(z::Z);

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

ARPC_MAKE_NESTED_CONSTANT_CHECKER(has_roro, roro);

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
      typename arpc::traits::member_function_pointer_traits<mfp>::return_type {
    return (static_cast<typename arpc::traits::member_function_pointer_traits<
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
    throw arpc::errors::invalid_argument("I don't like you!");
    std::cerr << "Clearing..." << std::endl;
  }
};

int main() {
  {
    arpc::listener l(arpc::endpoint().port(13133));
    for (int i = 0; i < 4; i++) {
      auto s = l.accept();
      std::cerr << "Got connection from " << s.peer_addr().as_string()
                << " into " << s.own_addr().as_string() << std::endl;
    }
  }
  try {
    using namespace std::literals;

    arpc::server_object<WriterImpl> writer;
    arpc::server server({}, arpc::endpoint().port(12121));
    server.register_object("test", writer);
    server.start();

    arpc::client_connection conn(
        arpc::endpoint().name("localhost").port(12121));
    auto obj = conn.get_proxy<Writer>("test");
    auto result = obj.get("patata");
    std::cerr << "RESULT: " << result << std::endl;
    obj.set("pollo", "vaca");
    obj.clear();

    server.stop();
    return 0;
  } catch (const arpc::errors::base_error& e) {
    std::cerr << e.portable_error_class_name() << ": " << e.what() << std::endl;
    return 0;
  } catch (int x) {
    std::cerr << x << std::endl;
    return 0;
  }
  {
    std::cerr
        << "ms: "
        << arpc::traits::type_hash_v<std::chrono::milliseconds> << std::endl;
    std::cerr << "m: "
              << arpc::traits::type_hash_v<std::chrono::minutes> << std::endl;
    std::cerr << "sys: "
              << arpc::traits::type_hash_v<
                     std::chrono::system_clock::time_point> << std::endl;
    std::cerr << "steady: "
              << arpc::traits::type_hash_v<
                     std::chrono::steady_clock::time_point> << std::endl;
    std::cerr
        << "high-res: "
        << arpc::traits::type_hash_v<
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
    arpc::promise<void> p1;
    auto f1 =
        p1.get_future().then([]() { std::cerr << "Was done!" << std::endl; });
    p1.set_value();
    f1.get();
  }
  {
    arpc::promise<int> p1;
    auto f1 = p1.get_future().then([](int i) {
      std::cerr << "Was done!" << std::endl;
      return i + 1;
    });
    p1.set_value(3);
    std::cerr << "GOT: " << f1.get() << std::endl;
  }
  {
    arpc::synchronous_executor x;
    for (int i = 0; i < 10; i++) {
      x.run([]() {
        std::cerr << "Hello "
                  << "world "
                  << "1" << std::endl;
      });
    }
  }
  {
    arpc::thread_pool x;
    for (int i = 0; i < 10; i++) {
      x.run([]() {
        std::cerr << "Hello "
                  << "world "
                  << "2/" << arpc::thread::hardware_concurrency() << " "
                  << std::this_thread::get_id() << std::endl;
      });
    }

    auto [res] = arpc::select(arpc::timeout(std::chrono::seconds(2)));
    *res;
  }
  {
    arpc::context ctx;

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
    arpc::ostream_output_stream osa(oss);
    arpc::native_binary_encoder nbe(osa);
    nbe(ctx);
    xxd(oss.str());

    std::istringstream iss(oss.str());
    arpc::istream_input_stream isa(iss);
    arpc::native_binary_decoder nbd(isa);
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
    arpc::promise<void> p1, p2;
    auto f1 = p1.get_future();
    auto f2 = p2.get_future();
    p2.set_value();
    std::vector<arpc::awaitable<void>> conds;
    conds.push_back(f1.can_get());
    conds.push_back(f2.can_get());
    auto [got_fut, timeout] = arpc::select(
        std::move(conds), arpc::timeout(std::chrono::milliseconds(2000)));

    std::cerr << !!timeout << ',' << !!got_fut[0] << ',' << !!got_fut[1]
              << std::endl;
  }
  {
    arpc::promise<void> p1, p2;
    auto f1 = p1.get_future();
    auto f2 = p2.get_future();
    std::vector<arpc::awaitable<void>> conds;
    conds.push_back(f1.can_get());
    conds.push_back(f2.can_get());
    auto timeout_a = arpc::timeout(std::chrono::milliseconds(2000));
    auto [got_fut, timeout] = arpc::select(conds, timeout_a);

    std::cerr << !!timeout << ',' << !!got_fut[0] << ',' << !!got_fut[1]
              << std::endl;
  }
  {
    arpc::promise<void> p;
    auto f = p.get_future();
    p.set_value();
    f.get();
  }
  {
    arpc::context ctx;
    ctx.set_timeout(std::chrono::milliseconds(100));
    auto s = dial(arpc::endpoint().name("www.google.com").service("https"));
  }
  {
    arpc::thread t1([]() {
      std::cerr << "CI " << &arpc::context::current() << std::endl;
      auto [res] = arpc::select(arpc::context::current().wait_cancelled());
      *res;
      std::cerr << "DONE!" << std::endl;
    });
    std::cerr << "CO " << &t1.get_context() << std::endl;
    t1.get_context().cancel();
    t1.join();
  }

  {
    std::cerr << *arpc::context::current().deadline_left() /
                     std::chrono::milliseconds(1)
              << std::endl;

    {
      arpc::context ctx2;
      ctx2.set_timeout(std::chrono::milliseconds(44));
      std::cerr << *arpc::context::current().deadline_left() /
                       std::chrono::milliseconds(1)
                << std::endl;

      try {
        (void)(arpc::select());
      } catch (const arpc::errors::cancelled&) {
        std::cerr << "CANCELLED!" << std::endl;
      } catch (const arpc::errors::deadline_exceeded&) {
        std::cerr << "DEADLINE!" << std::endl;
      }
    }

    std::cerr << *arpc::context::current().deadline_left() /
                     std::chrono::milliseconds(1)
              << std::endl;
  }

  {
    arpc::future<std::unique_ptr<int>> fi;
    arpc::promise<std::unique_ptr<int>> pi;
    fi = pi.get_future();
    arpc::thread th1([&]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(2000));
      pi.set_value(std::make_unique<int>(33));
    });

    auto [val, to] = arpc::select(
        fi.async_get(), arpc::timeout(std::chrono::milliseconds(3000)));
    if (val) {
      std::cerr << "VAL: " << **val << std::endl;
    }
    if (to) {
      std::cerr << "TIMED OUT" << std::endl;
    }

    th1.join();
  }

  arpc::queue<std::unique_ptr<int>> q(10);

  auto tl = [&q]() {
    do {
      auto [get, timeout] = arpc::select(
          q.async_get(), arpc::timeout(std::chrono::milliseconds(3000)));
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

  arpc::thread th1(tl);
  arpc::thread th2(tl);

  auto [put] = arpc::select(q.async_put(std::make_unique<int>(32)));
  std::cerr << "DID " << !!put << " DID" << std::endl;

  th1.join();
  th2.join();

  std::cerr << *(q.get()) << " DONE!" << std::endl;

  arpc::channel in(0);
  auto [read, timeout] = arpc::select(
      in.can_read(), arpc::timeout(std::chrono::milliseconds(3000)));
  std::cerr << !!read << !!timeout << std::endl;
  in.release();

  f<decltype(arpc::mpt::as_tuple(arpc::mpt::value_pack<33, 'c'>{}))>();

  std::cerr
      << "X"
      << arpc::traits::member_function_pointer_traits<&Reader::get>::is_const
      << "X" << std::endl;

  try {
    arpc::error_factory::get().throw_error("out_of_range", "It's way off!");
  } catch (const arpc::errors::out_of_range& oor) {
    std::cerr << "CAUGHT! " << oor.portable_error_class_name() << ": "
              << oor.what() << std::endl;
  }

  arpc::char_dev_connection cdc("/dev/tty");

  arpc::thread t1([&cdc] {
    std::this_thread::sleep_for(std::chrono::seconds(10));
    cdc.disconnect();
  });

  try {
    std::string buf("hello\n");
    cdc.write(buf.data(), buf.size());
    cdc.read(&buf[0], 3);
    std::cerr << buf << std::endl;
  } catch (const arpc::errors::base_error& sd) {
    std::cerr << "CAUGHT! " << sd.portable_error_class_name() << ": "
              << sd.what() << std::endl;
  }
  cdc.disconnect();

  t1.join();

  /*
  arpc::packet_connection_impl<arpc::char_dev_connection,
                              arpc::serial_line_packet_protocol<>>
      slpci("/dev/tty");
  slpci.send("hello");
  slpci.disconnect();

  arpc::reconnectable_connection<arpc::char_dev_connection> cdc2("/dev/tty");
  std::string buf2("hello\n");
  cdc2.connect();
  cdc2.write(buf2.data(), buf2.size());
  cdc2.disconnect();
  cdc2.connect();
  cdc2.write(buf2.data(), buf2.size());
  cdc2.disconnect();

  arpc::packet_connection_impl<
      arpc::reconnectable_connection<arpc::char_dev_connection>,
      arpc::protected_stream_packet_protocol<>>
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
    arpc::client_connection conn(arpc::endpoint().name("localhost").port(9999));
    auto obj = conn.get_proxy<Reader::async>("default"sv);
    arpc::future<std::string> res;
    {
      arpc::context ctx;
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
  arpc::istream_input_stream xsi(xs);
  arpc::ostream_output_stream xso(xs);
  arpc::protected_stream_packet_protocol<arpc::big_endian_binary_encoder,
                                         arpc::big_endian_binary_decoder>
      pspp;
  xxd(data);
  pspp.send(xso, data);
  xxd(xs.str());
  data = pspp.receive(xsi);
  xxd(data);

  xs.str("");
  xs.seekg(0);
  xs.seekp(0);

  arpc::serial_line_packet_protocol<> slpp;
  xxd(data);
  slpp.send(xso, data);
  xxd(xs.str());
  data = slpp.receive(xsi);
  xxd(data);

  arpc::mac_codec mac(key);
  xxd(data);
  mac.encode(data);
  xxd(data);
  mac.decode(data);
  xxd(data);

  arpc::mac_codec mac2;
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
    arpc::cobs_codec cobs;
    xxd(s);
    cobs.encode(s);
    xxd(s);
    cobs.decode(s);
    xxd(s);
  }

  std::cerr << arpc::mpt::element_type_t<
                   0, arpc::traits::get_field_descriptors_t<z::Z>>::type::name()
            << std::endl;
  std::cerr << arpc::mpt::element_type_t<
                   1, arpc::traits::get_field_descriptors_t<z::Z>>::type::name()
            << std::endl;

  std::cerr
      << arpc::mpt::element_type_t<0, Writer::method_descriptors>::type::name()
      << std::endl;
  std::cerr
      << arpc::mpt::element_type_t<1, Writer::method_descriptors>::type::name()
      << std::endl;

  try {
    std::string data = "Hello";
    std::cerr << std::hex
              << arpc::highway_hash::hash64(
                     reinterpret_cast<const uint8_t*>(data.data()), data.size(),
                     key)
              << std::dec << std::endl;
    data = "Hell!";
    std::cerr << std::hex
              << arpc::highway_hash::hash64(
                     reinterpret_cast<const uint8_t*>(data.data()), data.size(),
                     key)
              << std::dec << std::endl;
    f<decltype(kk::roro)>();
    std::cerr << has_roro<kk, int>::value << std::endl;
    std::cerr << has_roro<Y, int>::value << std::endl;

    using A = arpc::mpt::pack<>;
    using B = arpc::mpt::insert_type_into_t<int, A>;
    using C = arpc::mpt::insert_type_into_t<int, B>;
    using D = arpc::mpt::insert_type_into_t<double, C>;
    using E = arpc::mpt::insert_type_into_t<int, D>;
    f<E>();

    std::cerr << std::hex;
    std::cerr
        << arpc::traits::get_custom_serialization_version<signed int>::value
        << std::endl;

    std::cerr << arpc::traits::type_hash<signed int>::value << std::endl;
    std::cerr << arpc::traits::type_hash<std::tuple<int>>::value << std::endl;

    std::cerr << arpc::traits::type_hash<V>::value << std::endl;
    std::cerr << arpc::traits::type_hash<V2>::value << std::endl;
    std::cerr << arpc::traits::type_hash<X>::value << std::endl;
    std::cerr << arpc::traits::type_hash<Y>::value << std::endl;
    std::cerr << arpc::traits::type_hash<z::Z>::value << std::endl;
    std::cerr << arpc::traits::type_hash<std::shared_ptr<z::Z>>::value
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

    std::ostringstream oss;
    arpc::ostream_output_stream osa(oss);
    arpc::native_binary_encoder nbe(osa);
    nbe(tup, arpc::verify_structure{});
    nbe(x);
    nbe(v);
    nbe(v);
    nbe(w);
    nbe(y);
    nbe(z, arpc::verify_structure{});

    std::istringstream iss(oss.str());
    arpc::istream_input_stream isa(iss);
    arpc::native_binary_decoder nbd(isa);

    std::tuple<int, double> tup2;
    std::shared_ptr<X> x2;
    std::shared_ptr<V> v2;
    std::weak_ptr<V> w2;
    std::unique_ptr<Y> y2;
    std::shared_ptr<z::Z> z2;

    nbd(tup2, arpc::verify_structure{});
    nbd(x2);
    nbd(v2);
    nbd(v2);
    nbd(w2);
    nbd(y2);
    nbd(z2, arpc::verify_structure{});

    std::cerr << std::get<0>(tup2) << ", " << std::get<1>(tup2) << std::endl;

    std::cerr << x2->x << ", " << x2->a << std::endl;
    std::cerr << std::static_pointer_cast<X>(v2)->x << ", "
              << std::static_pointer_cast<X>(v2)->a << std::endl;

    std::cerr << (z2 == z2->z2) << std::endl;
    std::cerr << (z2 == z) << std::endl;

    std::cout << oss.str();

    throw arpc::errors::io_error("hello");
  } catch (const std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
