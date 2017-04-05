#ifndef ASH_DYNAMIC_CLASS_H_
#define ASH_DYNAMIC_CLASS_H_

#include "ash/mpt.h"

namespace ash {

/// Base class for all run-time polymorphic serializable classes.
class DynamicClass {
public:
	const char *getPortableClassName() {
		return portableClassName();
	}
	using field_descriptors = mpt::pack<>;
	using dynamic_base_classes = mpt::pack<>;

private:
	/// Get the string that portably identifies the object's class.
	virtual const char* portableClassName() const = 0;
};

namespace metadata {
namespace detail {

template<typename T>
struct DynamicClassDescriptor {
	static const char* class_name;
};

}  // namespace detail
}  // namespace metadata

}  // namespace ash

#endif /* ASH_DYNAMIC_CLASS_H_ */
