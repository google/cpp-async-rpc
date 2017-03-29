#ifndef ASH_STATIC_OBJECT_H_
#define ASH_STATIC_OBJECT_H_

namespace ash {

// Brought in from the Cereal Serialization Library.
#ifdef _MSC_VER
#   define ASH_DLL_EXPORT __declspec(dllexport)
#   define ASH_USED
#else // clang or gcc
#   define ASH_DLL_EXPORT
#   define ASH_USED __attribute__ ((__used__))
#endif

template <typename T>
class ASH_DLL_EXPORT StaticObject {
public:
	static T& get() ASH_USED {
		static T t;
		instance;  // Force a cyclic reference?
		return t;
	}
private:
	template <T&()> struct InstantiateFunction{};
	using unused = InstantiateFunction<get>;
	static T& instance;
};

template<typename T> T& StaticObject<T>::instance = get();

}  // namespace ash

#endif /* ASH_STATIC_OBJECT_H_ */
