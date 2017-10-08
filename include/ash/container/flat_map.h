#ifndef ASH_CONTAINER_FLAT_MAP_H_
#define ASH_CONTAINER_FLAT_MAP_H_

#include "ash/container/detail/flat_tree.h"

namespace ash {

template<typename Key, typename T, typename Compare = std::less<Key>,
		typename Allocator = std::allocator<std::pair<Key, T> > >
class flat_map: public detail::flat_tree<Key, false, std::pair<Key, T>, Compare,
		Allocator> {
public:
	using typename detail::flat_tree<Key, false, std::pair<Key, T>, Compare,
			Allocator>::underlying_container;
	using mapped_type = T;

	// Inherit constructors.
	using detail::flat_tree<Key, false, std::pair<Key, T>, Compare, Allocator>::flat_tree;

	// operator []
	T& operator[](const Key& key) {
		auto it = this->lower_bound(key);
		if (it != this->end() && this->eq_(*it, key)) {
			return it->second;
		}
		it = underlying_container::emplace(it, key, T());
		return it->second;
	}
	T& operator[](Key&& key) {
		auto it = this->lower_bound(key);
		if (it != this->end() && this->eq_(*it, key)) {
			return it->second;
		}
		it = underlying_container::emplace(it, std::forward<Key>(key), T());
		return it->second;
	}

	// at
	T& at(const Key& key) {
		auto it = this->find(key);
		if (it == this->end()) {
			throw std::out_of_range("key not found.");
		}
		return it->second;
	}
	const T& at(const Key& key) const {
		auto it = this->find(key);
		if (it == this->end()) {
			throw std::out_of_range("key not found.");
		}
		return it->second;
	}
};

template<typename Key, typename T, typename Compare = std::less<Key>,
		typename Allocator = std::allocator<std::pair<Key, T> > >
class flat_multimap: public detail::flat_tree<Key, true, std::pair<Key, T>,
		Compare, Allocator> {
public:
	using mapped_type = T;

	// Inherit constructors.
	using detail::flat_tree<Key, true, std::pair<Key, T>, Compare, Allocator>::flat_tree;
};

}  // namespace ash

#endif /* ASH_CONTAINER_FLAT_MAP_H_ */
