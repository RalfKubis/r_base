#pragma once

// https://codereview.stackexchange.com/questions/164341/simple-uint128-t-implementation


#include <type_traits>
#include <cstdint>
#include <array>
#include <ostream>

#include "r_base/endianess.h"


namespace nsBase
{

/** special class to represent a 128 bit unsigned number
    some shortcuts were taken to not make it too unnecessary complex
*/
struct uint128_t final
{
    constexpr uint128_t() = default;

    template<typename T, typename = ::std::enable_if_t<::std::is_integral_v<::std::decay_t<T>>>>
    constexpr
        uint128_t(T && num) noexcept
            :   array{ static_cast<::std::uint64_t>(num), 0ull }
            {
            }

// &

    template<typename T>
    friend constexpr uint128_t
        operator&(uint128_t lhs, T && rhs) noexcept
            {
                return lhs &= rhs;
            }

    constexpr uint128_t &
        operator&=(const uint128_t & rhs) noexcept
            {
                array[0] &= rhs.array[0];
                array[1] &= rhs.array[1];
                return *this;
            }

    template<typename T, typename = ::std::enable_if_t<::std::is_integral_v<::std::decay_t<T>>>>
    constexpr uint128_t &
        operator&=(T && rhs) noexcept
            {
                array[0] &= rhs;
                array[1] = 0;
                return *this;
            }


// |

    template<typename T>
    friend constexpr uint128_t
        operator|(uint128_t lhs, T && rhs) noexcept
            {
                return lhs |= rhs;
            }

    constexpr uint128_t &
        operator|=(const uint128_t & rhs) noexcept
            {
                array[0] |= rhs.array[0];
                array[1] |= rhs.array[1];
                return *this;
            }

    template<typename T, typename = ::std::enable_if_t<::std::is_integral_v<::std::decay_t<T>>>>
    constexpr uint128_t &
        operator|=(T && rhs) noexcept
            {
                array[0] |= rhs;
                return *this;
            }


// ^

    template<typename T>
    friend constexpr uint128_t
        operator^(uint128_t lhs, T && rhs) noexcept
            {
                return lhs ^= rhs;
            }

    constexpr uint128_t &
        operator^=(const uint128_t & rhs) noexcept
            {
                array[0] ^= rhs.array[0];
                array[1] ^= rhs.array[1];
                return *this;
            }

    template<typename T, typename = ::std::enable_if_t<::std::is_integral_v<::std::decay_t<T>>>>
    constexpr uint128_t &
        operator^=(T && rhs) noexcept
            {
                array[0] ^= rhs;
                return *this;
            }

// ~

    friend constexpr uint128_t
        operator~(uint128_t value) noexcept
            {
                value.array = { ~value.array[0], ~value.array[1] };
                return value;
            }


// <<

    template<typename T>
    friend constexpr uint128_t
        operator<<(uint128_t lhs, T && rhs) noexcept
            {
                return lhs <<= rhs;
            }

    constexpr uint128_t &
        operator<<=(const uint128_t & rhs) noexcept
            {
                if (rhs.array[1] > 0)
                    return array = {}, *this;
                return *this <<= rhs.array[0];
            }

    template<typename T, typename = ::std::enable_if_t<::std::is_integral_v<::std::decay_t<T>>>>
    constexpr uint128_t &
        operator<<=(T && rhs) noexcept
            {
                if (rhs == 0)
                    return *this;

                const auto backup = array[1];
                array[1] = array[0];
                if (rhs < 64) {
                    array[0] <<= rhs;
                    array[1] >>= 64 - rhs;
                    array[1] |= backup << rhs;
                }
                else {
                    array[0] = 0;
                    array[1] <<= rhs - 64;
                }

                return *this;
            }

// >>

    template<typename T>
    friend constexpr uint128_t
        operator>>(uint128_t lhs, T && rhs) noexcept
            {
                return lhs >>= rhs;
            }

    constexpr uint128_t &
        operator>>=(const uint128_t & rhs) noexcept
            {
                if (rhs.array[1] > 0)
                    return array = {}, *this;
                return *this >>= rhs.array[0];
            }

    template<typename T, typename = ::std::enable_if_t<::std::is_integral_v<::std::decay_t<T>>>>
    constexpr uint128_t &
        operator>>=(T && rhs) noexcept
            {
                if (rhs == 0)
                    return *this;

                const auto backup = array[0];
                array[0] = array[1];
                if (rhs < 64) {
                    array[1] >>= rhs;
                    array[0] <<= 64 - rhs;
                    array[0] |= backup >> rhs;
                }
                else {
                    array[1] = 0;
                    array[0] >>= rhs - 64;
                }

                return *this;
            }

// ==

    template<typename T, typename = ::std::enable_if_t<::std::is_integral_v<::std::decay_t<T>>>>
    constexpr bool
        operator==(T && rhs) const noexcept
            {
                return array[0] == static_cast<::std::uint64_t>(rhs) && array[1] == 0;
            }

    constexpr bool
        operator==(const uint128_t & rhs) const noexcept
            {
                return array[0] == rhs.array[0] && array[1] == rhs.array[1];
            }

// !=

    template<typename T>
    constexpr bool
        operator!=(T && rhs) const noexcept
            {
                return !(*this == rhs);
            }

// <

    template<typename T, typename = ::std::enable_if_t<::std::is_integral_v<::std::decay_t<T>>>>
    constexpr bool
        operator<(T && rhs) const noexcept
            {
                return array[0] < static_cast<::std::uint64_t>(rhs) && array[1] == 0;
            }

    constexpr bool
        operator<(const uint128_t & rhs) const noexcept
            {
                if (array[1] == rhs.array[1])
                    return array[0] < rhs.array[0];
                return array[1] < rhs.array[1];
            }

// >

    template<typename T>
    constexpr bool
        operator>(T && rhs) const noexcept
            {
                return *this >= rhs && *this != rhs;
            }

    template<typename T>
    constexpr bool
        operator<=(T && rhs) const noexcept
            {
                return !(*this > rhs);
            }

    template<typename T>
    constexpr bool
        operator>=(T && rhs) const noexcept
            {
                return !(*this < rhs);
            }


// +

    template<typename T>
    friend constexpr uint128_t
        operator+(uint128_t lhs, T && rhs) noexcept
            {
                return lhs += rhs;
            }

    constexpr uint128_t &
        operator+=(const uint128_t & rhs) noexcept
            {
                *this += rhs.array[0];
                array[1] += rhs.array[1];
                return *this;
            }

    template<typename T, typename = ::std::enable_if_t<::std::is_integral_v<::std::decay_t<T>>>>
    constexpr uint128_t &
        operator+=(T && rhs) noexcept
            {
                // overflow guard
                if (array[0] + rhs < array[0])
                    ++array[1];
                array[0] += rhs;
                return *this;
            }


// -

    template<typename T>
    friend constexpr uint128_t
        operator-(uint128_t lhs, T && rhs) noexcept
            {
                return lhs -= rhs;
            }

    constexpr uint128_t &
        operator-=(const uint128_t & rhs) noexcept
            {
                *this -= rhs.array[0];
                array[1] -= rhs.array[1];
                return *this;
            }

    template<typename T, typename = ::std::enable_if_t<::std::is_integral_v<::std::decay_t<T>>>>
    constexpr uint128_t &
        operator-=(T && rhs) noexcept
            {
                // overflow guard
                if (array[0] - rhs > array[0])
                    --array[1];
                array[0] -= rhs;
                return *this;
            }


//
    friend ::std::ostream &
        operator<<(::std::ostream& stream, const uint128_t & num) noexcept
            {
                return stream << '[' << num.array[0] << " + " << "2**64 * " << num.array[1] << ']';
            }

    private : ::std::array<uint64_t, 2>
        array{};

    // KU
    public : void
        to_uint8_array(::std::array<uint8_t, 16> & dst) const noexcept
            {
                *reinterpret_cast<uint64_t*>(&dst[0]) = hton64(array[0]);
                *reinterpret_cast<uint64_t*>(&dst[8]) = hton64(array[1]);
            }

    public :
        uint128_t(::std::array<uint8_t, 16> const & src) noexcept
            :   array
                    {
                        ntoh64(*reinterpret_cast<::std::uint64_t const*>(&src[0]))
                    ,   ntoh64(*reinterpret_cast<::std::uint64_t const*>(&src[8]))
                    }
            {
            }
};

}
