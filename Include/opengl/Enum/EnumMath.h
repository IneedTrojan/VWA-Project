#pragma once
#include <type_traits> 



template<typename E>
constexpr auto to_underlying(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}


#define ENABLE_ENUM_MATH(EnumClass) \
using UnderlyingType_##EnumClass = std::underlying_type_t<EnumClass>; \
\
constexpr auto operator*(EnumClass e) noexcept -> UnderlyingType_##EnumClass { \
    return static_cast<UnderlyingType_##EnumClass>(e); \
}\
constexpr EnumClass operator|(EnumClass lhs, EnumClass rhs) { \
    return static_cast<EnumClass>(to_underlying(lhs) | to_underlying(rhs)); \
} \
constexpr EnumClass operator&(EnumClass lhs, EnumClass rhs) { \
    return static_cast<EnumClass>(to_underlying(lhs) & to_underlying(rhs)); \
} \
constexpr EnumClass operator~(EnumClass rhs) { \
    return static_cast<EnumClass>(~to_underlying(rhs)); \
} \
constexpr EnumClass operator+(EnumClass lhs, EnumClass rhs) { \
    return static_cast<EnumClass>(to_underlying(lhs) + to_underlying(rhs)); \
} \
constexpr EnumClass operator-(EnumClass lhs, EnumClass rhs) { \
    return static_cast<EnumClass>(to_underlying(lhs) - to_underlying(rhs)); \
} \
constexpr EnumClass operator*(EnumClass lhs, EnumClass rhs) { \
    return static_cast<EnumClass>(to_underlying(lhs) * to_underlying(rhs)); \
} \
constexpr EnumClass operator/(EnumClass lhs, EnumClass rhs) { \
    return static_cast<EnumClass>(to_underlying(lhs) / to_underlying(rhs)); \
} \
constexpr EnumClass operator%(EnumClass lhs, EnumClass rhs) { \
    return static_cast<EnumClass>(to_underlying(lhs) % to_underlying(rhs)); \
} \
constexpr EnumClass operator<<(EnumClass lhs, int shift) { \
    return static_cast<EnumClass>(to_underlying(lhs) << shift); \
} \
constexpr EnumClass operator>>(EnumClass lhs, int shift) { \
    return static_cast<EnumClass>(to_underlying(lhs) >> shift); \
} \
constexpr EnumClass operator+(EnumClass lhs, UnderlyingType_##EnumClass rhs) { \
    return static_cast<EnumClass>(to_underlying(lhs) + rhs); \
} \
constexpr EnumClass operator-(EnumClass lhs, UnderlyingType_##EnumClass rhs) { \
    return static_cast<EnumClass>(to_underlying(lhs) - rhs); \
} \
constexpr EnumClass operator*(EnumClass lhs, UnderlyingType_##EnumClass rhs) { \
    return static_cast<EnumClass>(to_underlying(lhs) * rhs); \
} \
constexpr EnumClass operator/(EnumClass lhs, UnderlyingType_##EnumClass rhs) { \
    return static_cast<EnumClass>(to_underlying(lhs) / rhs); \
} \
constexpr EnumClass operator%(EnumClass lhs, UnderlyingType_##EnumClass rhs) { \
    return static_cast<EnumClass>(to_underlying(lhs) % rhs); \
} \
constexpr EnumClass operator<<(EnumClass lhs, UnderlyingType_##EnumClass shift) { \
    return static_cast<EnumClass>(to_underlying(lhs) << shift); \
} \
constexpr EnumClass operator>>(EnumClass lhs, UnderlyingType_##EnumClass shift) { \
    return static_cast<EnumClass>(to_underlying(lhs) >> shift); \
} \
constexpr EnumClass operator&(EnumClass lhs, UnderlyingType_##EnumClass rhs) { \
    return static_cast<EnumClass>(to_underlying(lhs) & rhs); \
} \
constexpr EnumClass operator|(EnumClass lhs, UnderlyingType_##EnumClass rhs) { \
    return static_cast<EnumClass>(to_underlying(lhs) | rhs); \
} \
constexpr EnumClass operator&(UnderlyingType_##EnumClass lhs, EnumClass rhs) { \
    return static_cast<EnumClass>(lhs & to_underlying(rhs)); \
} \
constexpr EnumClass operator|(UnderlyingType_##EnumClass lhs, EnumClass rhs) { \
    return static_cast<EnumClass>(lhs | to_underlying(rhs)); \
}

