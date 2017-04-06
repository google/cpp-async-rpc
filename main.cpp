#include <iostream>
#include <sstream>
#include <cstring>
#include <memory>
#include <string>
#include <tuple>
//#include <unordered_map>
#include "ash.h"
#include "ash/mpt.h"
#include "ash/registration.h"
#include "ash/iostream_adapters.h"

template <typename R>
struct K : ASH_SERIALIZABLE(K<R>) {
	R x = 1, y = 2;
	std::string z = "pasta";

	// Needed in template classes as the one in ASH_SERIALIZABLE isn't
	// visible without qualification. Bah.
	ASH_OWN_TYPE(K<R>);
	ASH_FIELDS(x, y, z);
};

struct V : ASH_DYNAMIC(V) {
	int a = 64;
	ASH_FIELDS(a);
};
ASH_REGISTER(V);

struct X : ASH_DYNAMIC(X, ASH_PUBLIC(V)) {
	int x = 1, y = 2;
	std::string z = "pasta";

	ASH_FIELDS(x, y, z);
};
ASH_REGISTER(X);

struct Y : ASH_DYNAMIC(Y, ASH_PUBLIC(V)) {
};
ASH_REGISTER(Y);

namespace z {
struct Z : ASH_DYNAMIC(Z, ASH_PUBLIC(X)) {
};
ASH_REGISTER(z::Z);
}

template<typename T>
void f(T) {
	std::cerr << "X: " << __PRETTY_FUNCTION__ << std::endl;
}

int main() {
	z::Z z2;
	std::unique_ptr<z::Z> z1(ash::registry::DynamicObjectFactory::get().create<z::Z>("z::Z"));
	std::unique_ptr<V> v1(ash::registry::DynamicObjectFactory::get().create<V>("z::Z"));
	std::unique_ptr<Y> y1(ash::registry::DynamicObjectFactory::get().create<Y>("V"));

	std::cerr << z2.getPortableClassName() << std::endl;
	std::cerr << z1->getPortableClassName() << std::endl;
	std::cerr << v1->getPortableClassName() << std::endl;
	std::cerr << (y1 == nullptr) << std::endl;
	//std::cerr << y1->getPortableClassName() << std::endl;

	using pp = ash::mpt::pack<double, int, double>;
	constexpr auto rrrrr = ash::mpt::count_if(pp { }, ash::mpt::is<double> { });
	f(ash::mpt::filter_if(pp { }, ash::mpt::is<double> { }));
	f(ash::mpt::find_if(pp { }, ash::mpt::is<double> { }));
	std::unique_ptr < X > x(new X());
	x->x = 44;
	x->a = 88;
	std::unique_ptr < X > y;

	f(X::base_classes { });
	f(X::public_base_classes { });
	f(X::protected_base_classes { });
	f(X::private_base_classes { });

	std::cerr
			<< ash::traits::can_be_saved<decltype(*x), ash::NativeBinaryEncoder>::value
			<< std::endl;

	ash::BinarySizer bs;
	bs(x);
	bs(y);
	std::cerr << "SIZE: " << bs.size() << std::endl;

	std::ostringstream oss;
	ash::OStreamAdapter osa(oss);
	ash::NativeBinaryEncoder nbe(osa);
	nbe(x);
	nbe(y);

	std::istringstream iss(oss.str());
	ash::IStreamAdapter isa(iss);
	ash::NativeBinaryDecoder nbd(isa);

	std::unique_ptr<X> x2, y2;

	nbd(x2);
	nbd(y2);

	std::cerr << x2->x << ", " << x2->a << std::endl;

	std::cout << oss.str();

	return 0;
}
