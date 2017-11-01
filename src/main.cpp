#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include "ash.h"
#include "ash/iostream_adapters.h"
#include "ash/mpt.h"
#include "ash/serializable.h"
#include "ash/status.h"
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
  ash::status save(S&) const {
    return ash::status::OK;
  }

  template <typename S>
  ash::status load(S&) {
    return ash::status::OK;
  }

  ASH_CUSTOM_SERIALIZATION_VERSION(1);
};
ASH_REGISTER(V2);

struct X : ash::dynamic<X, V> {
  int x = 1, y = 2;
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
}

template <typename T>
void f() {
  std::cerr << "XXX: " << __PRETTY_FUNCTION__ << std::endl;
}

struct kk {
  static constexpr int roro = 1;
};

ASH_MAKE_NESTED_CONSTANT_CHECKER(has_roro, roro);

int main() {
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
  std::cerr << ash::traits::get_custom_serialization_version<signed int>::value
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

  // ASH_CHECK(3 == 4);
  ash::status_or<int> code;
  ash::status_or<int> code1 = ash::status::FAILED_PRECONDITION;
  std::shared_ptr<X> x(new X());
  x->x = 44;
  x->a = 88;
  std::shared_ptr<V> v = x;
  std::weak_ptr<V> w = v;
  std::unique_ptr<Y> y(new Y());
  std::shared_ptr<z::Z> z(new z::Z());
  z->z2 = z;
  z->z = "rosco";

  ash::binary_sizer bs;
  ASH_CHECK_OK(bs(code1, ash::verify_structure{}));
  ASH_CHECK_OK(bs(x));
  ASH_CHECK_OK(bs(v));
  ASH_CHECK_OK(bs(v));
  ASH_CHECK_OK(bs(w));
  ASH_CHECK_OK(bs(y));
  ASH_CHECK_OK(bs(z, ash::verify_structure{}));
  std::cerr << "SIZE: " << bs.size() << std::endl;

  std::ostringstream oss;
  ash::ostream_output_stream osa(oss);
  ash::native_binary_encoder nbe(osa);
  ASH_CHECK_OK(nbe(code1, ash::verify_structure{}));
  ASH_CHECK_OK(nbe(x));
  ASH_CHECK_OK(nbe(v));
  ASH_CHECK_OK(nbe(v));
  ASH_CHECK_OK(nbe(w));
  ASH_CHECK_OK(nbe(y));
  ASH_CHECK_OK(nbe(z, ash::verify_structure{}));

  std::istringstream iss(oss.str());
  ash::istream_input_stream isa(iss);
  ash::native_binary_decoder nbd(isa);

  std::shared_ptr<X> x2;
  std::shared_ptr<V> v2;
  std::weak_ptr<V> w2;
  std::unique_ptr<Y> y2;
  std::shared_ptr<z::Z> z2;

  ASH_CHECK_OK(nbd(code, ash::verify_structure{}));
  ASH_CHECK_OK(nbd(x2));
  ASH_CHECK_OK(nbd(v2));
  ASH_CHECK_OK(nbd(v2));
  ASH_CHECK_OK(nbd(w2));
  ASH_CHECK_OK(nbd(y2));
  ASH_CHECK_OK(nbd(z2, ash::verify_structure{}));

  std::cerr << ash::name(code) << "(" << ash::code(code)
            << "): " << ash::ok(code) << std::endl;
  std::cerr << x2->x << ", " << x2->a << std::endl;
  std::cerr << std::static_pointer_cast<X>(v2)->x << ", "
            << std::static_pointer_cast<X>(v2)->a << std::endl;

  std::cerr << (z2 == z2->z2) << std::endl;
  std::cerr << (z2 == z) << std::endl;

  std::cout << oss.str();

  return 0;
}
