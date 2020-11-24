#ifndef MC2TCPD_ENUMBINARYOPERATORS_H
#define MC2TCPD_ENUMBINARYOPERATORS_H

#include <type_traits>

template<typename Enum>
struct EnableBinaryOperators
{
    constexpr static const bool value = false;
};

template<typename Enum>
typename std::enable_if<EnableBinaryOperators<Enum>::value, Enum>::type
operator |(Enum lhs, Enum rhs)
{
    using underlying = typename std::underlying_type<Enum>::type;
    return static_cast<Enum> (
            static_cast<underlying>(lhs) |
            static_cast<underlying>(rhs)
    );
}
template<typename Enum>
typename std::enable_if<EnableBinaryOperators<Enum>::value, Enum>::type
operator &(Enum lhs, Enum rhs)
{
    using underlying = typename std::underlying_type<Enum>::type;
    return static_cast<Enum> (
            static_cast<underlying>(lhs) &
            static_cast<underlying>(rhs)
    );
}

#endif //MC2TCPD_ENUMBINARYOPERATORS_H
