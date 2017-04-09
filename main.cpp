#include <iostream>
#include <sstream>
#include <memory>
#include <string>

#include "ash.h"
#include "ash/mpt.h"
#include "ash/serializable.h"
#include "ash/iostream_adapters.h"
#include "ash/vector_assoc.h"

template<typename R>
struct K: ash::serializable<K<R>> {
	R x = 1, y = 2;
	std::string z = "pasta";

	ASH_OWN_TYPE(K<R>);ASH_FIELDS(x, y, z);
};

struct V: ash::dynamic<V> {
	int a = 64;ASH_FIELDS(a);
};
ASH_REGISTER(V);

struct X: ash::dynamic<X, V> {
	int x = 1, y = 2;
	std::string z = "pasta";

	ASH_FIELDS(x, y, z);
};
ASH_REGISTER(X);

struct Y: ash::dynamic<Y, V> {
};
ASH_REGISTER(Y);

namespace z {
	struct Z : ash::dynamic<Z, X> {
	};
	ASH_REGISTER(z::Z);
}

template<typename T>
void f(T) {
	std::cerr << "X: " << __PRETTY_FUNCTION__ << std::endl;
}

int main() {
	ash::vector_multiset<int> h { 3, 2, 2 };
	h.emplace(5);
	h.insert(13);
	h.insert(13);
	//h[4] = 1;
	//std::cerr << h.at(13) << std::endl;
	ash::vector_multiset<int> l;
	l = h;

	for (auto it = h.begin(); it != h.end(); it++) {
		std::cerr << *it << std::endl;
	}

	h.erase(2);

	for (auto it = h.begin(); it != h.end(); it++) {
		std::cerr << *it << std::endl;
	}

	z::Z z2;
	std::unique_ptr<z::Z> z1(
			ash::registry::dynamic_object_factory::get().create<z::Z>("z::Z"));
	std::unique_ptr<V> v1(
			ash::registry::dynamic_object_factory::get().create<V>("z::Z"));
	std::unique_ptr<Y> y1(
			ash::registry::dynamic_object_factory::get().create<Y>("V"));

	std::cerr << z2.portable_class_name() << std::endl;
	std::cerr << z1->portable_class_name() << std::endl;
	std::cerr << v1->portable_class_name() << std::endl;
	std::cerr << (y1 == nullptr) << std::endl;
	//std::cerr << y1->portable_class_name() << std::endl;

	using pp = ash::mpt::pack<double, int, double>;
	constexpr auto rrrrr = ash::mpt::count_if(pp { }, ash::mpt::is<double> { });
	f(ash::mpt::filter_if(pp { }, ash::mpt::is<double> { }));
	f(ash::mpt::find_if(pp { }, ash::mpt::is<double> { }));
	std::unique_ptr < X > x(new X());
	x->x = 44;
	x->a = 88;
	std::unique_ptr < X > y;

	f(X::base_classes { });

	std::cerr
			<< ash::traits::can_be_saved<decltype(*x),
					ash::native_binary_encoder>::value << std::endl;

	ash::binary_sizer bs;
	bs(x);
	bs(y);
	std::cerr << "SIZE: " << bs.size() << std::endl;

	std::ostringstream oss;
	ash::ostream_output_stream osa(oss);
	ash::native_binary_encoder nbe(osa);
	nbe(x);
	nbe(y);

	std::istringstream iss(oss.str());
	ash::istream_input_stream isa(iss);
	ash::native_binary_decoder nbd(isa);

	std::unique_ptr<X> x2, y2;

	nbd(x2);
	nbd(y2);

	std::cerr << x2->x << ", " << x2->a << std::endl;

	std::cout << oss.str();

	return 0;
}
