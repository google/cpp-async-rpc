#ifndef ASH_ENUM_H_
#define ASH_ENUM_H_

#include <algorithm>
#include <type_traits>
#include <utility>

#include "ash/preprocessor.h"

namespace ash {

template <typename T> struct enum_names {
	static const char* name(const T& value) {
		const base_type v = static_cast<base_type>(value);
		auto it = std::find_if(entries, entries + size, [&v](const std::pair<base_type, const char*>& x) {
			return x.first == v;
		});
		if (it == entries + size) {
			return unknown_name;
		}
		else {
			return it->second;
		}
	}

private:
	using base_type = typename std::underlying_type<T>::type;
	static const std::size_t size;
	static const char* unknown_name;
	static const std::pair<base_type, const char*> entries[];
};

#define ASH_ENUM_ENTRY_IN_BODY_IMPL(NAME, VALUE) NAME = VALUE
#define ASH_ENUM_ENTRY_IN_BODY(...) ASH_NO_PARENS(ASH_ENUM_ENTRY_IN_BODY_IMPL, __VA_ARGS__ )
#define ASH_ENUM_ENTRY_IN_ARRAY_IMPL(NAME, VALUE) { VALUE, #NAME }
#define ASH_ENUM_ENTRY_IN_ARRAY(...) ASH_NO_PARENS(ASH_ENUM_ENTRY_IN_ARRAY_IMPL, __VA_ARGS__ )
#define ASH_ENUM_ENTRY_SEP() ,

#define ASH_ENUM(NAME, BASE_TYPE, UNKNOWN_NAME, ...) \
enum class NAME : BASE_TYPE { \
	ASH_FOREACH(ASH_ENUM_ENTRY_IN_BODY, ASH_ENUM_ENTRY_SEP, __VA_ARGS__) \
}; \
template <> const std::size_t ::ash::enum_names<NAME>::size = ASH_ARG_COUNT(__VA_ARGS__); \
template <> const char* ::ash::enum_names<NAME>::unknown_name = UNKNOWN_NAME; \
template <> const std::pair<typename ::ash::enum_names<NAME>::base_type, const char*> ash::enum_names<NAME>::entries[] = { \
	ASH_FOREACH(ASH_ENUM_ENTRY_IN_ARRAY, ASH_ENUM_ENTRY_SEP, __VA_ARGS__) \
}

}  // namespace ash

#endif /* ASH_ENUM_H_ */
