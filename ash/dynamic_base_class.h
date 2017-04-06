#ifndef ASH_DYNAMIC_BASE_CLASS_H_
#define ASH_DYNAMIC_BASE_CLASS_H_

#include <type_traits>

#include "ash/mpt.h"

namespace ash {

/// Base class for all run-time polymorphic serializable classes.
class dynamic_base_class {
public:
	const char *portable_class_name() {
		return portable_class_name_internal();
	}
	using field_descriptors = mpt::pack<>;
	using dynamic_base_classes = mpt::pack<>;

private:
	/// Get the string that portably identifies the object's class.
	virtual const char* portable_class_name_internal() const = 0;
};

/// Checker for whether a class `T` is dynamic.
template <typename T>
struct is_dynamic : std::is_base_of<dynamic_base_class, T> {};

namespace detail {
template<typename T>
struct dynamic_class_descriptor {
	static const char* class_name;
};
}  // namespace detail

}  // namespace ash

#endif /* ASH_DYNAMIC_BASE_CLASS_H_ */
