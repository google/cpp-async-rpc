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

ASH_STRUCT(V) {
	int a = 64;

	//virtual void f(){};

	ASH_FIELDS(V, ASH_FIELD(a));
};

ASH_REGISTER_CLASS("V", V);

ASH_STRUCT(X, ash::metadata::PublicVirtual<V>) {
	int x = 1, y = 2;
	std::string z = "pasta";

	ASH_FIELDS(X,
			ASH_FIELD(x),
			ASH_FIELD(y),
			ASH_FIELD(z)
	);
};

ASH_REGISTER_CLASS("X", X);

ASH_STRUCT(Y, ash::metadata::PublicVirtual<V>) {
};

ASH_REGISTER_CLASS("Y", Y);

ASH_STRUCT(Z, ash::metadata::Public<X>, ash::metadata::Public<Y>) {
};

ASH_REGISTER_CLASS("Z", Z);

template<typename T>
void f(T) {
	std::cerr << "X: " << __PRETTY_FUNCTION__ << std::endl;
}

int main() {
	using pp = ash::mpt::pack<double, int, double>;
	f(ash::mpt::find_if(pp{}, ash::mpt::is<double>{}));

	f(ash::metadata::DynamicBaseClasses<V>{});
	f(ash::metadata::DynamicBaseClasses<X>{});
	f(ash::metadata::DynamicBaseClasses<Z>{});
	std::unique_ptr<X> x(new X());
	x->x = 44;
	x->a = 88;
	std::unique_ptr<X> y;

	f(X::base_classes{});
	f(X::public_base_classes{});
	f(X::protected_base_classes{});
	f(X::private_base_classes{});

	std::cerr << ash::traits::can_be_saved<decltype(*x), ash::NativeBinaryEncoder>::value << std::endl;

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
