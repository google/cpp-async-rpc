#ifndef ASH_SINGLETON_H_
#define ASH_SINGLETON_H_

namespace ash {

template <typename T>
struct singleton {
	static T& get() {
		static T instance;
		return instance;
	}
};

}  // namespace ash

#endif /* ASH_SINGLETON_H_ */
