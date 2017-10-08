#ifndef ASH_CONTAINER_FLAT_SET_H_
#define ASH_CONTAINER_FLAT_SET_H_

#include "ash/container/detail/flat_tree.h"

namespace ash {

template<typename Key, typename Compare = std::less<Key>,
		typename Allocator = std::allocator<Key> >
class flat_set: public detail::flat_tree<Key, false, Key, Compare, Allocator> {
public:
	// Inherit constructors.
	using detail::flat_tree<Key, false, Key, Compare, Allocator>::flat_tree;
};

template<typename Key, typename Compare = std::less<Key>,
		typename Allocator = std::allocator<Key> >
class flat_multiset: public detail::flat_tree<Key, true, Key, Compare,
		Allocator> {
public:
	// Inherit constructors.
	using detail::flat_tree<Key, true, Key, Compare, Allocator>::flat_tree;
};

}  // namespace ash

#endif /* ASH_CONTAINER_FLAT_SET_H_ */
