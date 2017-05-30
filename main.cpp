#include <iostream>
#include <sstream>
#include <memory>
#include <string>

#include "ash.h"
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
	struct Z : ash::dynamic<Z, X> {
	};
	ASH_REGISTER(z::Z);
}

int main() {
	//ASH_CHECK(3 == 4);
	ash::status code;
	std::shared_ptr < X > x(new X());
	x->x = 44;
	x->a = 88;
	std::shared_ptr < V > v = x;
	std::weak_ptr < V > w = v;
	std::unique_ptr < Y > y(new Y());
	std::unique_ptr < z::Z > z(new z::Z());

	ash::binary_sizer bs;
	ASH_CHECK_OK(bs(ash::status::FAILED_PRECONDITION));
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
	ASH_CHECK_OK(nbe(ash::status::FAILED_PRECONDITION));
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
	std::unique_ptr<z::Z> z2;

	ASH_CHECK_OK(nbd(code));
	ASH_CHECK_OK(nbd(x2));
	ASH_CHECK_OK(nbd(v2));
	ASH_CHECK_OK(nbd(v2));
	ASH_CHECK_OK(nbd(w2));
	ASH_CHECK_OK(nbd(y2));
	ASH_CHECK_OK(nbd(z2));

	std::cerr << x2->x << ", " << x2->a << std::endl;
	std::cerr << std::static_pointer_cast<X>(v2)->x << ", " << std::static_pointer_cast<X>(v2)->a << std::endl;

	std::cout << oss.str();

	return 0;
}
