#pragma once

#include <span>

#define UUID_SYSTEM_GENERATOR

#include <stduuid/uuid.h>


consteval auto
    operator "" _uuid(
            char        const * data
        ,   ::std::size_t       len
        )
        ->  ::uuids::uuid
        {
            auto is_digit = [](char const c){return (c>='0' && c<='9') || (c>='a' && c<='f');};
            auto is_minus = [](char const c){return  c=='-';};

            // if this fails, the UUID string is wrong sized
            len /= static_cast<::std::size_t>(len==36);
            len /= static_cast<::std::size_t>(
                    is_digit(data[ 0])
                &&  is_digit(data[ 1])
                &&  is_digit(data[ 2])
                &&  is_digit(data[ 3])
                &&  is_digit(data[ 4])
                &&  is_digit(data[ 5])
                &&  is_digit(data[ 6])
                &&  is_digit(data[ 7])

                &&  is_minus(data[ 8])

                &&  is_digit(data[ 9])
                &&  is_digit(data[10])
                &&  is_digit(data[11])
                &&  is_digit(data[12])

                &&  is_minus(data[13])

                &&  is_digit(data[14])
                &&  is_digit(data[15])
                &&  is_digit(data[16])
                &&  is_digit(data[17])

                &&  is_minus(data[18])

                &&  is_digit(data[19])
                &&  is_digit(data[20])
                &&  is_digit(data[21])
                &&  is_digit(data[22])

                &&  is_minus(data[23])

                &&  is_digit(data[24])
                &&  is_digit(data[25])
                &&  is_digit(data[26])
                &&  is_digit(data[27])
                &&  is_digit(data[28])
                &&  is_digit(data[29])
                &&  is_digit(data[30])
                &&  is_digit(data[31])
                &&  is_digit(data[32])
                &&  is_digit(data[33])
                &&  is_digit(data[34])
                &&  is_digit(data[35])
                );

            auto u = ::uuids::uuid::from_string(::std::string_view{data, len});

            // if this fails, You might have given an invalid UUID string argument
            len /= static_cast<::std::size_t>(u.has_value());

            return u.value();
        }


namespace nsBase::uuids
{

/** Parse UUID from a given string.
    \return NULLOPT from an empty string, otherwise a UUID.
    \throws If the given string is non-empty and doesn't parse as a UUID.
*/
::std::optional<::uuids::uuid>
    from_string(
            ::std::string_view const & s
        );


/** Parse UUID from a given string.
    \return NULLOPT from an empty string or a NUL-uuid, otherwise a UUID.
    \throws If the given string is non-empty and doesn't parse as a UUID.
*/
::std::optional<::uuids::uuid>
    from_string_with_empty_and_NIL_to_nullopt(
            ::std::string_view const & s
        );


/** Parse UUID from a given string.
    \return The parsed UUID while an empty string produces a NIL-UUID.
    \throws If the given string is non-empty and doesn't parse as a UUID.
*/
::uuids::uuid
    from_string_with_empty_to_NIL(
            ::std::string_view const & s
        );
}


namespace uuids
{
/** Convert an optional UUID to a string.
    If the optional is empty, a NIL-UUID is returned.
*/
::std::string
    to_string_with_empty_to_NIL(
            ::std::optional<::uuids::uuid> const &
        );


::std::string
    to_string8(
            ::uuids::uuid const &
        );

::std::string
    to_string8_with_empty_to_NIL(
            ::std::optional<::uuids::uuid> const &
        );

}
