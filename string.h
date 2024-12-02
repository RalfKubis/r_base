#pragma once
/* Copyright (C) Ralf Kubis */

#include "r_base/decl.h"
#include "r_base/dbc.h"

#include <string>
#include <sstream>
#include <regex>
#include <vector>
#include <algorithm>
#include <cstddef>
#include <type_traits>


namespace nsBase // make it known for dingdong
{
}


namespace dingdong
{
template<typename T>
::std::string
    dingdong_to_string(
            T const & t
        )
        {
            using namespace ::nsBase;
            using namespace ::std;

            return to_string(t);
        }

template<>
inline ::std::string
    dingdong_to_string<::std::string>(
            ::std::string const & t
        )
        {
            return t;
        }
}


namespace nsBase
{
/**
    Conversion between various String types.

    C - char *
    Q - QString
    S - ::std::string
    O - OFString
    B - QByteArray
*/


////////////////////////////////////////////////////////////
//  ::std::string -> *
//
[[nodiscard]] inline char const *
    S2C(::std::string const & inValue)
        {
            return inValue.c_str();
        }


/**
    Get a string that that results when all matches of a regular expression
    in a source string are replaced by a substitute.

    \param inSource     The source string.
    \param inRegexp     The ECMAScript regular expression.
    \param inSubstitute The substitute that replaces the matches.

    \return The resulting string.
*/
[[nodiscard]] ::std::string
    replaced_allRegexp(
            ::std::string const & inSource
        ,   ::std::regex  const & inRegexp
        ,   ::std::string const & inSubstitute
        );


/**
    Get a string that that results when the first occurrence of the
    'what string' in a source string is replaced by a substitute.

    \param inSource     The source string.
    \param inFrom       The what string to be replaced.
    \param inTo         The substitute that replaces the 'what string'.

    \return The resulting string.
*/
[[nodiscard]] ::std::string
    replaced_first(
            ::std::string const & inSource
        ,   ::std::string const & inFrom
        ,   ::std::string const & inTo
        );


/**
    Get a string that that results when the all occurrences of the
    'what string' in a source string are replaced by a substitute.

    \param inSource     The source string.
    \param inFrom       The what string to be replaced.
    \param inTo         The substitute that replaces the 'what string'.

    \return The resulting string.
*/
[[nodiscard]] ::std::string
    replaced_all(
            ::std::string const & inSource
        ,   ::std::string const & inFrom
        ,   ::std::string const & inTo
        );


/**

*/
[[nodiscard]] ::std::string
    unpadded(
            ::std::string const & inSource
        );


/**

*/
[[nodiscard]] ::std::vector<::std::string>
    split(
            ::std::string_view const & source
        ,   ::std::string_view const & delimiter
        ,   bool                       keep_delimiter = {}
        );

[[nodiscard]] ::std::pair<::std::string, ::std::string>
    split_at_first_occurrence_of(
            ::std::string_view const & source
        ,   ::std::string_view const & delimiter
        ,   bool                       keep_delimiter = {}
        );


#if 0
/**
    Concatenate the elements of a collection into a single string using the
    provided delimiter.
    If the elements of the collection are not of type ::std::string, a matching
    function to_string() is looked up that converts the elements to ::std::string.
    Be aware that the compiler might select non-explicit constructors to
    the elements to produce instance that match the available to_string() function.
*/
template<
    typename Collection
,   typename ::std::enable_if_t<::std::is_same<typename ::std::remove_cv<typename Collection::value_type>::type,::std::string>::value, int> = 0
>
[[nodiscard]] auto
joined(
        Collection    const & lines
    ,   ::std::string const & delimiter
    )
    ->  ::std::string
    {
        ::std::string
            retVal;

        auto
            count = 0;

        for (auto const & line : lines)
        {
            if (count)
                retVal += delimiter;
            count++;

            retVal += line;
        }

        return retVal;
    }


template<
    typename Collection
,   typename ::std::enable_if_t<!::std::is_same<typename ::std::remove_cv<typename Collection::value_type>::type,::std::string>::value, int> = 0
>
[[nodiscard]] auto
joined(
        Collection    const & lines
    ,   ::std::string const & delimiter
    )
    ->  ::std::string
    {
        ::std::string
            retVal;

        auto
            count = 0;

        for (auto & line : lines)
        {
            if (count)
                retVal += delimiter;
            count++;

            retVal += ::dingdong::dingdong_to_string(line);
        }

        return retVal;
    }
#endif


template<
    typename T
,   typename ::std::enable_if_t<!::std::is_same<typename ::std::remove_cv<T>::type,::std::string>::value, int> = 0
>
::std::string
to_joinable_string_helper(
    T const & v
)
{
    return ::dingdong::dingdong_to_string(v);
}


template<
    typename T
,   typename ::std::enable_if_t<::std::is_same<typename ::std::remove_cv<T>::type,::std::string>::value, int> = 0
>
::std::string_view
to_joinable_string_helper(
    T const & v
)
{
    return v;
}


template<
    typename Collection
>
[[nodiscard]] auto
joined(
        Collection    const & lines
    ,   ::std::string const & delimiter
    )
    ->  ::std::string
    {
        ::std::string
            retVal;

        auto
            count = 0;

        for (auto const & line : lines)
        {
            if (count)
                retVal += delimiter;
            count++;

            retVal += to_joinable_string_helper(line);
        }

        return retVal;
    }


[[nodiscard]] auto
escaped(
        ::std::string const & inSource
    )
    ->  ::std::string;


[[nodiscard]] auto
quoted(
        ::std::string const & inSource
    )
    ->  ::std::string;


// modifies input string, returns input
inline ::std::string &
trim_left_in_place(
    ::std::string & str
)
{
    size_t
        i = 0;

    while(  i < str.size()
        &&  isspace( (unsigned char)(str[i]) )
    )
    {
        ++i;
    };

    return str.erase(0, i);
}


inline ::std::string &
trim_right_in_place(
    ::std::string & str
)
{
    size_t
        i = str.size();

    while(  i > 0
        &&  isspace((unsigned char)(str[i-1]))
    )
    {
        --i;
    };

    return str.erase(i, str.size());
}


inline ::std::string &
trim_in_place(
    ::std::string & str
)
{
    return trim_left_in_place(trim_right_in_place(str));
}


// returns newly created strings
inline ::std::string
trim_right(
    ::std::string str
)
{
    return trim_right_in_place(str);
}


[[nodiscard]] inline ::std::string
trim_left(
    ::std::string str
)
{
    return trim_left_in_place(str);
}


[[nodiscard]] inline ::std::string
trim(
    ::std::string str
)
{
    return trim_left_in_place(trim_right_in_place(str));
}


[[nodiscard]] inline
::std::string
to_lower(
    ::std::string const & s
)
{
    auto
        l = s;

    ::std::transform(
            l.begin()
        ,   l.end()
        ,   l.begin()
        ,   ::tolower
        );

    return l;
}


[[nodiscard]] inline
::std::string
to_upper(
    ::std::string_view const & s
)
{
    ::std::string
        l {s};

    ::std::transform(
            l.begin()
        ,   l.end()
        ,   l.begin()
        ,   ::toupper
        );

    return l;
}


/** Encode a byte-sequence into a string representing
    each byte as two hexadecimal digits [0-9A-F].
*/
void
    dataToHexString(
            unsigned char const * inBufferToEncode
        ,   unsigned int          inBufferToEncodeLen
        ,   ::std::string       & outEncodedBuffer
        ,   bool                  inUseUpperCase
        );


/** Convert a string of hexadecimal a byte-sequence into a byte-array.
The length of the target buffer must be sufficiently big, i.e. at least
half the size of the input string.
The input-strings size must be an even number.
*/
void
    hexStringToData(
            unsigned char             * inBufferToDecodeTo
        ,   ::std::string_view const  & inEncodedBuffer
        );

[[nodiscard]]
inline ::std::string
    append_with_separator(
            ::std::string const & pre
        ,   ::std::string const & separator
        ,   ::std::string const & post
        )
        {
            return pre + (pre.empty() ? ::std::string{} : separator) + post;
        }


// as with C++20 this gets replaced by string::starts_with()
template<typename Collection>
[[nodiscard]] bool
starts_with(
        Collection         const & subject
    ,   ::std::string_view const & prefix
    )
    {
        return ::std::mismatch(prefix.begin(), prefix.end(), subject.begin(), subject.end()).first==prefix.end();
    }

}
