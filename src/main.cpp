#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include "ash.h"
// #include "ash/client.h"
#include "ash/connection.h"
#include "ash/errors.h"
#include "ash/highway_hash.h"
#include "ash/interface.h"
#include "ash/iostream_adapters.h"
#include "ash/linux/connection.h"
#include "ash/mpt.h"
#include "ash/packet_codecs.h"
#include "ash/packet_protocols.h"
#include "ash/serializable.h"
#include "ash/string_adapters.h"
#include "ash/type_hash.h"

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
ASH_REGISTER(z::Z);
}  // namespace z

template <typename T>
void f() {
  std::cerr << "XXX: " << __PRETTY_FUNCTION__ << std::endl;
}

struct kk {
  static constexpr int roro = 1;
};

ASH_MAKE_NESTED_CONSTANT_CHECKER(has_roro, roro);

struct MyInterface : ash::interface<MyInterface> {
  virtual void Method1(int x) = 0;
  virtual int Method2(int x, int y) = 0;
  ASH_METHODS(Method1, Method2);
};

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

int main() {
  try {
    ash::error_factory::get().throw_error("out_of_range", "It's way off!");
  } catch (const ash::errors::out_of_range& oor) {
    std::cerr << "CAUGHT! " << oor.portable_error_class_name() << ": "
              << oor.what() << std::endl;
  }

  ash::char_dev_connection cdc("/dev/tty");

  std::thread t1([&cdc] {
    std::this_thread::sleep_for(std::chrono::seconds(10));
    cdc.disconnect();
  });

  try {
    cdc.connect();

    std::string buf("hello\n");
    cdc.write(buf.data(), buf.size());
    cdc.read(&buf[0], 3);
    std::cerr << buf << std::endl;
  } catch (const ash::errors::shutting_down& sd) {
    std::cerr << "CAUGHT! " << sd.portable_error_class_name() << ": "
              << sd.what() << std::endl;
  }
  cdc.disconnect();

  t1.join();

  ash::packet_connection_impl<ash::char_dev_connection,
                              ash::serial_line_packet_protocol<>>
      slpci("/dev/tty");
  slpci.connect();
  slpci.send("hello");
  slpci.disconnect();

  /*
  std::string req, res;
  ash::string_output_stream req_os(req);
  ash::string_input_stream res_is(res);
  ash::serial_line_packet_protocol<> slpprpc(res_is, req_os);
  // xxd(slpprpc.receive());
  // return 0;
  ash::client_connection<> conn(slpprpc);
  auto obj = conn.object("default");
  obj.ASH_CALL(MyInterface::Method2)(133, 22);

  xxd(req);
  */

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

  std::cerr << decltype(
                   ash::mpt::at<0>(z::Z::field_descriptors{}))::type::name()
            << std::endl;
  std::cerr << decltype(
                   ash::mpt::at<1>(z::Z::field_descriptors{}))::type::name()
            << std::endl;

  std::cerr << decltype(ash::mpt::at<0>(
                   MyInterface::method_descriptors{}))::type::name()
            << std::endl;
  std::cerr << decltype(ash::mpt::at<1>(
                   MyInterface::method_descriptors{}))::type::name()
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
    using B = ash::mpt::insert_into_t<int, A>;
    using C = ash::mpt::insert_into_t<int, B>;
    using D = ash::mpt::insert_into_t<double, C>;
    using E = ash::mpt::insert_into_t<int, D>;
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
