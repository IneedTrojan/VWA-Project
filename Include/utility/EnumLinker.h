#pragma once
#include <vector>
#include <initializer_list>
#include <map>
#include <exception>
#include <algorithm>
#include <magic_enum/magic_enum_all.hpp>
#include <type_traits>


namespace utility
{
	



	template<typename Enum, typename T>
	class EnumLinker
	{
		std::vector<T> map;
		int64_t maxIndex = std::numeric_limits<int64_t>::min();
		int64_t minIndex = std::numeric_limits<int64_t>::max();
	public:
	

		EnumLinker(std::initializer_list<std::pair<Enum, T>> values) {

			map.reserve(values.size());
			for (const auto& pair : values)
			{
				maxIndex = std::max(maxIndex, static_cast<int64_t>(pair.first) + 1);
				minIndex = std::min(minIndex, static_cast<int64_t>(pair.first));
			}

			map.resize(maxIndex);
			for (const auto& pair : values)
			{
				map[static_cast<int64_t>(pair.first)-minIndex] = pair.second;
			}
		}
		
		T operator[](Enum index)
		{
			int64_t idx = static_cast<uint64_t>(static_cast<int64_t>(index));
			return idx < maxIndex ?
				map[idx - minIndex] :
				static_cast<T>(map[maxIndex- minIndex]);
		}
		Enum findKey(T value)
		{
			auto it = std::find(map.begin(), map.end(), value);
			return it == map.end()?static_cast<Enum>(maxIndex + 1) :
			static_cast<Enum>(std::distance(map.begin(), it) + minIndex);
		}
	};
	

	template<typename Enum, typename T>
	class EnumMap
	{
		std::unordered_map<Enum, T> map;
	public:
		EnumMap(std::initializer_list<std::pair<Enum, T>> values) {

			map.reserve(values.size());
			for (const auto& pair : values)
			{
				map.emplace(pair.first, pair.second);
			}
		}

		T operator[](Enum index)
		{
			return map[static_cast<Enum>(index)];
		}
		Enum findKey(T value)
		{
			for(auto& pair: map)
			{
				if(pair.second == value)
				{
					return pair.first;
				}
			}
			return static_cast<Enum>(0);
		}
	};

}
template<typename T>
inline constexpr bool is_allowed_bitwise_integral_v = std::is_integral_v<T> && !std::is_same_v<T, bool>;

#define ENABLE_BITWISE_OPERATORS(EnumType)                                      \
constexpr EnumType operator << (EnumType lhs,  std::underlying_type_t<EnumType> rhs) {                   \
    using T = std::underlying_type_t<EnumType>;                                 \
    return static_cast<EnumType>(static_cast<T>(lhs) << static_cast<T>(rhs));   \
}																				\
constexpr EnumType operator >> (EnumType lhs,  std::underlying_type_t<EnumType> rhs) {                   \
    using T = std::underlying_type_t<EnumType>;                                 \
    return static_cast<EnumType>(static_cast<T>(lhs) >> static_cast<T>(rhs));   \
}																				\
constexpr EnumType operator << (EnumType lhs, EnumType rhs) {                   \
    using T = std::underlying_type_t<EnumType>;                                 \
    return static_cast<EnumType>(static_cast<T>(lhs) << static_cast<T>(rhs));   \
}																				\
constexpr EnumType operator >> (EnumType lhs, EnumType rhs) {                   \
    using T = std::underlying_type_t<EnumType>;                                 \
    return static_cast<EnumType>(static_cast<T>(lhs) >> static_cast<T>(rhs));   \
}																				\
constexpr EnumType operator | (EnumType lhs, EnumType rhs) {                    \
    using T = std::underlying_type_t<EnumType>;                                 \
    return static_cast<EnumType>(static_cast<T>(lhs) | static_cast<T>(rhs));    \
}                                                                               \
template<typename T, std::enable_if_t<is_allowed_bitwise_integral_v<T>, bool> = true> \
constexpr EnumType operator | (EnumType lhs, T rhs) {                           \
    return lhs | static_cast<EnumType>(rhs);                                    \
}                                                                               \
constexpr EnumType operator & (EnumType lhs, EnumType rhs) {                    \
    using T = std::underlying_type_t<EnumType>;                                 \
    return static_cast<EnumType>(static_cast<T>(lhs) & static_cast<T>(rhs));    \
}                                                                               \
template<typename T, std::enable_if_t<is_allowed_bitwise_integral_v<T>, bool> = true> \
constexpr EnumType operator & (EnumType lhs, T rhs) {                           \
    return lhs & static_cast<EnumType>(rhs);                                    \
}                                                                               \
constexpr EnumType operator ^ (EnumType lhs, EnumType rhs) {                    \
    using T = std::underlying_type_t<EnumType>;                                 \
    return static_cast<EnumType>(static_cast<T>(lhs) ^ static_cast<T>(rhs));    \
}                                                                               \
template<typename T, std::enable_if_t<is_allowed_bitwise_integral_v<T>, bool> = true> \
constexpr EnumType operator ^ (EnumType lhs, T rhs) {                           \
    return lhs ^ static_cast<EnumType>(rhs);                                    \
}                                                                               \
constexpr EnumType operator ~ (EnumType rhs) {                                  \
    using T = std::underlying_type_t<EnumType>;                                 \
    return static_cast<EnumType>(~static_cast<T>(rhs));                         \
}



#define ENABLE_NUMERIC_CONVERSION(EnumType)                                         \
template<typename NumericType>                                                      \
constexpr std::enable_if_t<std::is_arithmetic_v<NumericType>, NumericType>          \




template<typename EnumType, typename T>
using is_allowed_integral = std::disjunction<
	std::is_same<T, int>,
	std::is_same<T, int>,
	std::is_same<T, unsigned int>,
	std::is_same<T, uint64_t>, // Add other types as needed
	std::is_same<EnumType, std::underlying_type_t<T>> 
>;



#define ENABLE_ARITHMETIC_OPERATORS(EnumType)                                   \
template<typename T,                                                           \
         typename = std::enable_if_t<is_allowed_integral<EnumType, T>::value>>  \
constexpr EnumType operator+(EnumType lhs, T rhs) {                             \
    return static_cast<EnumType>(static_cast<std::underlying_type_t<EnumType>>(lhs) + rhs); \
}                                                                               \
template<typename T,                                                           \
         typename = std::enable_if_t<is_allowed_integral<EnumType, T>::value>>  \
constexpr EnumType operator-(EnumType lhs, T rhs) {                             \
    return static_cast<EnumType>(static_cast<std::underlying_type_t<EnumType>>(lhs) - rhs); \
}


