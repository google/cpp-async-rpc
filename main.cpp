#include <iostream>
#include <sstream>
#include <memory>
#include <string>

#include "ash.h"
#include "ash/type_hash.h"
#include "ash/status.h"
#include "ash/mpt.h"
#include "ash/serializable.h"
#include "ash/iostream_adapters.h"
#include "ash/vector_assoc.h"

template<typename R>
struct K: ash::serializable<K<R>> {
	R x = 1, y = 2;
	std::string z = "pasta";

	ASH_OWN_TYPE(K<R>);

	ASH_FIELDS(x, y, z);
};

struct V: ash::dynamic<V> {
	int a = 64;

	ASH_FIELDS(a);
};
ASH_REGISTER(V);

struct X: ash::dynamic<X, V> {
	int x = 1, y = 2;
	std::string z = "pasta";

	ASH_FIELDS(x, y, z);
};
ASH_REGISTER(X);

struct Y: ash::serializable<Y> {
	int u = 32;

	ASH_FIELDS(u);
};
//ASH_REGISTER(Y);

namespace z {
struct Z: ash::dynamic<Z, X> {
	std::shared_ptr<Z> z2;

	ASH_FIELDS(z2);
};
ASH_REGISTER(z::Z);}

int main() {
	std::cerr << std::hex;
	std::cerr << ash::traits::type_hash<signed int>::value << std::endl;
	std::cerr << ash::traits::type_hash<unsigned int>::value << std::endl;
	std::cerr << ash::traits::type_hash<long double>::value << std::endl;
	std::cerr << ash::traits::type_hash<ash::status>::value << std::endl;
	std::cerr << ash::traits::type_hash<unsigned int[1]>::value << std::endl;
	std::cerr << ash::traits::type_hash<unsigned int[2]>::value << std::endl;

	std::cerr << ash::traits::type_hash<signed int[1]>::value << std::endl;
	std::cerr << ash::traits::type_hash<signed int[2]>::value << std::endl;

	std::cerr << ash::traits::type_hash<std::array<signed int, 2>>::value
			<< std::endl;

	std::cerr << ash::traits::type_hash<signed int[2][3]>::value << std::endl;

	std::cerr << ash::traits::type_hash<std::pair<int, double>>::value
			<< std::endl;
	std::cerr << ash::traits::type_hash<std::tuple<int, double>>::value
			<< std::endl;

	std::cerr << ash::traits::type_hash<std::vector<std::pair<int, double>>>::value
			<< std::endl;
	std::cerr << ash::traits::type_hash<std::set<std::pair<int, double>>>::value
			<< std::endl;
	std::cerr << ash::traits::type_hash<std::multiset<std::pair<int, double>>>::value
			<< std::endl;
	std::cerr << ash::traits::type_hash<std::map<int, double>>::value
			<< std::endl;
	std::cerr << ash::traits::type_hash<ash::vector_map<int, double>>::value
			<< std::endl;

	std::cerr << ash::traits::type_hash<std::unique_ptr<char>>::value
			<< std::endl;
	std::cerr << ash::traits::type_hash<std::shared_ptr<char>>::value
			<< std::endl;
	std::cerr << ash::traits::type_hash<std::weak_ptr<char>>::value
			<< std::endl;

	std::cerr << ash::traits::type_hash<std::vector<char>>::value
			<< std::endl;
	std::cerr << ash::traits::type_hash<std::string>::value
			<< std::endl;

	/*
	 std::cerr << ash::traits::type_hash<ash::status_or<int>>::value
	 << std::endl;
	 */

	//ASH_CHECK(3 == 4);
	ash::status_or<int> code;
	ash::status_or<int> code1 = ash::status::FAILED_PRECONDITION;
	std::shared_ptr < X > x(new X());
	x->x = 44;
	x->a = 88;
	std::shared_ptr<V> v = x;
	std::weak_ptr < V > w = v;
	std::unique_ptr < Y > y(new Y());
	std::shared_ptr<z::Z> z(new z::Z());
	z->z2 = z;
	z->z = "rosco";

	ash::binary_sizer bs;
	ASH_CHECK_OK(bs(code1));
	ASH_CHECK_OK(bs(x));
	ASH_CHECK_OK(bs(v));
	ASH_CHECK_OK(bs(v));
	ASH_CHECK_OK(bs(w));
	ASH_CHECK_OK(bs(y));
	ASH_CHECK_OK(bs(z));
	std::cerr << "SIZE: " << bs.size() << std::endl;

	std::ostringstream oss;
	ash::ostream_output_stream osa(oss);
	ash::native_binary_encoder nbe(osa);
	ASH_CHECK_OK(nbe(code1));
	ASH_CHECK_OK(nbe(x));
	ASH_CHECK_OK(nbe(v));
	ASH_CHECK_OK(nbe(v));
	ASH_CHECK_OK(nbe(w));
	ASH_CHECK_OK(nbe(y));
	ASH_CHECK_OK(nbe(z));

	std::istringstream iss(oss.str());
	ash::istream_input_stream isa(iss);
	ash::native_binary_decoder nbd(isa);

	std::shared_ptr<X> x2;
	std::shared_ptr<V> v2;
	std::weak_ptr<V> w2;
	std::unique_ptr<Y> y2;
	std::shared_ptr<z::Z> z2;

	ASH_CHECK_OK(nbd(code));
	ASH_CHECK_OK(nbd(x2));
	ASH_CHECK_OK(nbd(v2));
	ASH_CHECK_OK(nbd(v2));
	ASH_CHECK_OK(nbd(w2));
	ASH_CHECK_OK(nbd(y2));
	ASH_CHECK_OK(nbd(z2));

	std::cerr << ash::name(code) << "(" << ash::code(code) << "): "
			<< ash::ok(code) << std::endl;
	std::cerr << x2->x << ", " << x2->a << std::endl;
	std::cerr << std::static_pointer_cast < X
			> (v2)->x << ", " << std::static_pointer_cast < X
			> (v2)->a << std::endl;

	std::cerr << (z2 == z2->z2) << std::endl;
	std::cerr << (z2 == z) << std::endl;

	std::cout << oss.str();

	return 0;
}
