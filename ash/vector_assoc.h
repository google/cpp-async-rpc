#ifndef ASH_VECTOR_ASSOC_H_
#define ASH_VECTOR_ASSOC_H_

#include <algorithm>
#include <functional>
#include <initializer_list>
#include <memory>
#include <utility>
#include <vector>

namespace ash {

template<typename Key, typename Compare = std::less<Key>,
		typename Allocator = std::allocator<Key> > class vector_set: protected std::vector<
		Key, Allocator> {
public:
	using underlying_container = std::vector<Key, Allocator>;
	using key_type = Key;
	using typename underlying_container::value_type;
	using typename underlying_container::size_type;
	using typename underlying_container::difference_type;
	using key_compare = Compare;
	using value_compare = Compare;
	using typename underlying_container::allocator_type;
	using typename underlying_container::reference;
	using typename underlying_container::const_reference;
	using typename underlying_container::pointer;
	using typename underlying_container::const_pointer;
	using typename underlying_container::iterator;
	using typename underlying_container::const_iterator;
	using typename underlying_container::reverse_iterator;
	using typename underlying_container::const_reverse_iterator;

	using underlying_container::begin;
	using underlying_container::cbegin;
	using underlying_container::end;
	using underlying_container::cend;
	using underlying_container::rbegin;
	using underlying_container::crbegin;
	using underlying_container::rend;
	using underlying_container::crend;

	using underlying_container::get_allocator;

	using underlying_container::empty;
	using underlying_container::size;
	using underlying_container::max_size;
	using underlying_container::reserve;
	using underlying_container::capacity;
	using underlying_container::shrink_to_fit;

	using underlying_container::clear;
	using underlying_container::swap;

	// swap.
	void swap(vector_set& other) {
		underlying_container::swap(other);
	}

	// Assignment.
	vector_set& operator=(const vector_set& other) {
		underlying_container::operator =(other);
		return *this;
	}
	vector_set& operator=(vector_set&& other) {
		underlying_container::operator =(std::forward < vector_set > (other));
		return *this;
	}
	vector_set& operator=(std::initializer_list<value_type> ilist) {
		underlying_container::operator =(ilist);
		std::sort(begin(), end(), comp_);
		this->erase(std::unique(begin(), end()), end());
		return *this;
	}

	// Constructors.
	explicit vector_set(const Compare& comp, const Allocator& alloc =
			Allocator()) :
			underlying_container(alloc), comp_(comp) {
	}
	explicit vector_set(const Allocator& alloc = Allocator()) :
			underlying_container(alloc) {
	}
	template<typename InputIt>
	vector_set(InputIt first, InputIt last, const Compare& comp = Compare(),
			const Allocator& alloc = Allocator()) :
			underlying_container(first, last, alloc), comp_(comp) {
		std::sort(begin(), end(), comp_);
		this->erase(std::unique(begin(), end()), end());
	}
	vector_set(const vector_set& other) :
			underlying_container(
					static_cast<const underlying_container&>(other)) {
	}
	vector_set(const vector_set& other, const Allocator& alloc) :
			underlying_container(
					static_cast<const underlying_container&>(other), alloc) {
	}
	vector_set(vector_set&& other) :
			underlying_container(
					static_cast<const underlying_container&&>(other)) {
	}
	vector_set(vector_set&& other, const Allocator& alloc) :
			underlying_container(
					static_cast<const underlying_container&&>(other), alloc) {
	}
	vector_set(std::initializer_list<value_type> init, const Compare& comp =
			Compare(), const Allocator& alloc = Allocator()) :
			underlying_container(init, alloc), comp_(comp) {
		std::sort(begin(), end(), comp_);
		this->erase(std::unique(begin(), end()), end());
	}

	// find/bounds
	iterator upper_bound(const Key& key) {
		return std::upper_bound(begin(), end(), key, comp_);
	}
	const_iterator upper_bound(const Key& key) const {
		return std::upper_bound(begin(), end(), key, comp_);
	}
	iterator lower_bound(const Key& key) {
		return std::lower_bound(begin(), end(), key, comp_);
	}
	const_iterator lower_bound(const Key& key) const {
		return std::lower_bound(begin(), end(), key, comp_);
	}
	std::pair<iterator, iterator> equal_range(const Key& key) {
		return std::equal_range(begin(), end(), key, comp_);
	}
	std::pair<const_iterator, const_iterator> equal_range(
			const Key& key) const {
		return std::equal_range(begin(), end(), key, comp_);
	}

	// insert
	std::pair<iterator, bool> insert(const value_type& value) {
		auto it = lower_bound(value);
		if (it != end() && *it == value) {
			return {it, false};
		}
		underlying_container::insert(it, value);
		return {it, true};
	}
	std::pair<iterator, bool> insert(value_type&& value) {
		auto it = lower_bound(value);
		if (it != end() && *it == value) {
			return {it, false};
		}
		underlying_container::insert(it, std::forward < value_type > (value));
		return {it, true};
	}
	iterator insert(const_iterator hint, const value_type& value) {
		return insert(value).first;
	}
	iterator insert(const_iterator hint, value_type&& value) {
		return insert(std::forward < value_type > (value)).first;
	}
	template<typename InputIt>
	void insert(InputIt first, InputIt last) {
		for (auto it = first; it != last; it++) {
			insert(*it);
		}
	}
	void insert(std::initializer_list<value_type> ilist) {
		insert(ilist.begin(), ilist.end());
	}

	// emplace
	template<typename ... Args>
	std::pair<iterator, bool> emplace(Args&&... args) {
		return insert(std::move(value_type(std::forward<Args>(args)...)));
	}
	template<class ... Args>
	iterator emplace_hint(const_iterator hint, Args&&... args) {
		return insert(std::move(value_type(std::forward<Args>(args)...)));
	}

	// erase
	iterator erase(const_iterator pos) {
		return underlying_container::erase(pos);
	}
	iterator erase(const_iterator first, const_iterator last) {
		return underlying_container::erase(first, last);
	}
	size_type erase(const key_type& key) {
		auto iters = equal_range(key);
		auto result = iters.second - iters.first;
		erase(iters.first, iters.second);
		return result;
	}

	// count.
	size_type count(const Key& key) const {
		auto iters = equal_range(key);
		return iters.second - iters.first;
	}

	// find.
	iterator find(const Key& key) {
		auto it = lower_bound(key);
		if (it != end() && *it == key) {
			return it;
		}
		return end();
	}
	const_iterator find(const Key& key) const {
		auto it = lower_bound(key);
		if (it != end() && *it == key) {
			return it;
		}
		return end();
	}

	// compare accessors.
	key_compare key_comp() const {
		return comp_;
	}
	value_compare value_comp() const {
		return comp_;
	}

private:
	Compare comp_;
};



template<typename Key, typename T, typename Compare = std::less<Key>,
		typename Allocator = std::allocator<Key> > class vector_map: protected std::vector<
		std::pair<Key, T>, Allocator> {
public:
	using underlying_container = std::vector<std::pair<Key, T>, Allocator>;
	using key_type = Key;
	using mapped_type = T;
	using value_type = std::pair<const Key, T>;
	using typename underlying_container::size_type;
	using typename underlying_container::difference_type;
	using key_compare = Compare;

	struct value_compare {
		value_compare(const Compare& comp = Compare()) : comp_(comp) {}
		using result_type = bool;
		using first_argument_type = value_type;
		using second_argument_type = value_type;

		bool operator()(const value_type& left, const value_type& right) {
			return comp_(left.first, right.first);
		}

		bool operator()(const value_type& left, const key_type& right) {
			return comp_(left.first, right);
		}

		Compare comp_;
	};
	using typename underlying_container::allocator_type;
	using typename underlying_container::reference;
	using typename underlying_container::const_reference;
	using typename underlying_container::pointer;
	using typename underlying_container::const_pointer;
	using typename underlying_container::iterator;
	using typename underlying_container::const_iterator;
	using typename underlying_container::reverse_iterator;
	using typename underlying_container::const_reverse_iterator;

	using underlying_container::begin;
	using underlying_container::cbegin;
	using underlying_container::end;
	using underlying_container::cend;
	using underlying_container::rbegin;
	using underlying_container::crbegin;
	using underlying_container::rend;
	using underlying_container::crend;

	using underlying_container::get_allocator;

	using underlying_container::empty;
	using underlying_container::size;
	using underlying_container::max_size;
	using underlying_container::reserve;
	using underlying_container::capacity;
	using underlying_container::shrink_to_fit;

	using underlying_container::clear;
	using underlying_container::swap;

	// swap.
	void swap(vector_map& other) {
		underlying_container::swap(other);
	}

	// Assignment.
	vector_map& operator=(const vector_map& other) {
		underlying_container::operator =(other);
		return *this;
	}
	vector_map& operator=(vector_map&& other) {
		underlying_container::operator =(std::forward < vector_map > (other));
		return *this;
	}
	vector_map& operator=(std::initializer_list<value_type> ilist) {
		underlying_container::operator =(ilist);
		std::sort(begin(), end(), comp_);
		this->erase(std::unique(begin(), end()), end());
		return *this;
	}

	// Constructors.
	explicit vector_map(const Compare& comp, const Allocator& alloc =
			Allocator()) :
			underlying_container(alloc), comp_(comp) {
	}
	explicit vector_map(const Allocator& alloc = Allocator()) :
			underlying_container(alloc) {
	}
	template<typename InputIt>
	vector_map(InputIt first, InputIt last, const Compare& comp = Compare(),
			const Allocator& alloc = Allocator()) :
			underlying_container(first, last, alloc), comp_(comp) {
		std::sort(begin(), end(), comp_);
		this->erase(std::unique(begin(), end()), end());
	}
	vector_map(const vector_map& other) :
			underlying_container(
					static_cast<const underlying_container&>(other)) {
	}
	vector_map(const vector_map& other, const Allocator& alloc) :
			underlying_container(
					static_cast<const underlying_container&>(other), alloc) {
	}
	vector_map(vector_map&& other) :
			underlying_container(
					static_cast<const underlying_container&&>(other)) {
	}
	vector_map(vector_map&& other, const Allocator& alloc) :
			underlying_container(
					static_cast<const underlying_container&&>(other), alloc) {
	}
	vector_map(std::initializer_list<value_type> init, const Compare& comp =
			Compare(), const Allocator& alloc = Allocator()) :
			underlying_container(init, alloc), comp_(comp) {
		std::sort(begin(), end(), comp_);
		this->erase(std::unique(begin(), end()), end());
	}

	// find/bounds
	iterator upper_bound(const Key& key) {
		return std::upper_bound(begin(), end(), key, comp_);
	}
	const_iterator upper_bound(const Key& key) const {
		return std::upper_bound(begin(), end(), key, comp_);
	}
	iterator lower_bound(const Key& key) {
		return std::lower_bound(begin(), end(), key, comp_);
	}
	const_iterator lower_bound(const Key& key) const {
		return std::lower_bound(begin(), end(), key, comp_);
	}
	std::pair<iterator, iterator> equal_range(const Key& key) {
		return std::equal_range(begin(), end(), key, comp_);
	}
	std::pair<const_iterator, const_iterator> equal_range(
			const Key& key) const {
		return std::equal_range(begin(), end(), key, comp_);
	}

	// insert
	std::pair<iterator, bool> insert(const value_type& value) {
		auto it = lower_bound(value.first);
		if (it != end() && it->first == value.first) {
			return {it, false};
		}
		underlying_container::insert(it, value);
		return {it, true};
	}
	std::pair<iterator, bool> insert(value_type&& value) {
		auto it = lower_bound(value.first);
		if (it != end() && it->first == value.first) {
			return {it, false};
		}
		underlying_container::insert(it, std::forward < value_type > (value));
		return {it, true};
	}
	iterator insert(const_iterator hint, const value_type& value) {
		return insert(value).first;
	}
	iterator insert(const_iterator hint, value_type&& value) {
		return insert(std::forward < value_type > (value)).first;
	}
	template<typename InputIt>
	void insert(InputIt first, InputIt last) {
		for (auto it = first; it != last; it++) {
			insert(*it);
		}
	}
	void insert(std::initializer_list<value_type> ilist) {
		insert(ilist.begin(), ilist.end());
	}

	// emplace
	template<typename ... Args>
	std::pair<iterator, bool> emplace(Args&&... args) {
		return insert(std::move(value_type(std::forward<Args>(args)...)));
	}
	template<class ... Args>
	iterator emplace_hint(const_iterator hint, Args&&... args) {
		return insert(std::move(value_type(std::forward<Args>(args)...)));
	}

	// erase
	iterator erase(const_iterator pos) {
		return underlying_container::erase(pos);
	}
	iterator erase(const_iterator first, const_iterator last) {
		return underlying_container::erase(first, last);
	}
	size_type erase(const key_type& key) {
		auto iters = equal_range(key);
		auto result = iters.second - iters.first;
		erase(iters.first, iters.second);
		return result;
	}

	// count.
	size_type count(const Key& key) const {
		auto iters = equal_range(key);
		return iters.second - iters.first;
	}

	// find.
	iterator find(const Key& key) {
		auto it = lower_bound(key);
		if (it != end() && it->first == key) {
			return it;
		}
		return end();
	}
	const_iterator find(const Key& key) const {
		auto it = lower_bound(key);
		if (it != end() && it->first == key) {
			return it;
		}
		return end();
	}

	// compare accessors.
	key_compare key_comp() const {
		return comp_.comp_;
	}
	value_compare value_comp() const {
		return comp_;
	}

private:
	value_compare comp_;
};

}  // namespace ash

#endif /* ASH_VECTOR_ASSOC_H_ */
