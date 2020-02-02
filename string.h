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

#include <QCoreApplication>
#include <QByteArray>
#include <QString>
#include <QChar>

namespace nsBase
{

////////////////////////////////////////////////////////////////////////////////
/**
    Conversion between various String types.

    C - char *
    Q - QString
    S - ::std::string
    O - OFString
    B - QByteArray
*/




////////////////////////////////////////////////////////////
//  char* -> QString
//
inline QString const
    C2Q(char const * const inValue)
        {
            return QString::fromUtf8(inValue);
        }

inline auto
    operator "" _qs(
            char16_t    const * c_str
        ,   ::std::size_t       len
        )
        -> QString
        {
            return QString::fromUtf16(c_str);
        }


////////////////////////////////////////////////////////////
//  QString -> *
//

// Note: Q2C needs to be a macro because the converted QByteArray returned by toUtf8()
// is temporary and thus the const char* would be dangling after a function return.
#define Q2C(inValue) \
    (inValue).toUtf8().constData()


/**
    Convert a QString to a QByteArray.
    The returned buffer is encoded in UTF-8.
*/
inline QByteArray const
    Q2B( QString const & inValue )
        {
            return inValue.toUtf8();
        }

/**
    Convert a QString to a ::std::string.
    The returned string is encoded in UTF-8.
*/
inline ::std::string
    Q2S(
            QString const & inValue
        )
        {
            return Q2C(inValue);
        }



////////////////////////////////////////////////////////////
//  QByteArray -> *
//
inline ::std::string
    B2S(
            QByteArray const & ba
        )
        {
            if (ba.isEmpty())
                return {};

            return { ba.constData(), static_cast<::std::string::size_type>(ba.size()) };
        }

////////////////////////////////////////////////////////////
//  ::std::string -> *
//
inline char const *
    S2C(::std::string const & inValue)
        {
            return inValue.c_str();
        }

inline QString
    S2Q(::std::string const & inValue)
        {
            return C2Q( inValue.c_str() );
        }

inline QByteArray
    S2B(
            ::std::string const & s
        )
        {
            if (s.empty())
                return {};

            return { s.data(), int(s.size()) };
        }

////////////////////////////////////////////////////////////////////////////////
/**
    Get a string that that results when all matches of a regular expression
    in a source string are replaced by a substitute.

    \param inSource     The source string.
    \param inRegexp     The ECMAScript regular expression.
    \param inSubstitute The substitute that replaces the matches.

    \return The resulting string.
*/

::std::string
    replaced_allRegexp(
            ::std::string const & inSource
        ,   ::std::regex  const & inRegexp
        ,   ::std::string const & inSubstitute
        );


////////////////////////////////////////////////////////////////////////////////
/**
    Get a string that that results when the first occurrence of the
    'what string' in a source string is replaced by a substitute.

    \param inSource     The source string.
    \param inFrom       The what string to be replaced.
    \param inTo         The substitute that replaces the 'what string'.

    \return The resulting string.
*/

::std::string
    replaced_first(
            ::std::string const & inSource
        ,   ::std::string const & inFrom
        ,   ::std::string const & inTo
        );


////////////////////////////////////////////////////////////////////////////////
/**
    Get a string that that results when the all occurrences of the
    'what string' in a source string are replaced by a substitute.

    \param inSource     The source string.
    \param inFrom       The what string to be replaced.
    \param inTo         The substitute that replaces the 'what string'.

    \return The resulting string.
*/

::std::string
    replaced_all(
            ::std::string const & inSource
        ,   ::std::string const & inFrom
        ,   ::std::string const & inTo
        );


////////////////////////////////////////////////////////////////////////////////
/**

*/

::std::string
    unpadded(
            ::std::string const & inSource
        );


////////////////////////////////////////////////////////////////////////////////
/**

*/

::std::vector<::std::string>
    split(
            ::std::string const & inSource
        ,   ::std::string const & inDelimiter
        );


////////////////////////////////////////////////////////////////////////////////
/**
    Concatenate the elements of a collection into a single string using the
    provided delimiter.
    If the elements of the collection are not of type std::string, a matching
    function to_string() is looked up that converts the elements to std::string.
    Be aware that the compiler might select non-explicit constructors to
    the elements to produce instance that match the available to_string() function.
*/
template<
    typename Collection
,   typename ::std::enable_if_t<::std::is_same<typename ::std::remove_cv<typename Collection::value_type>::type,::std::string>::value, int> = 0
>
auto
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
auto
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

            retVal += to_string(line);
        }

        return retVal;
    }


////////////////////////////////////////////////////////////////////////////////

auto
escaped(
        ::std::string const & inSource
    )
    ->  ::std::string;


////////////////////////////////////////////////////////////////////////////////
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


////////////////////////////////////////////////////////////////////////////////
inline ::std::string &
trim_right_in_place(
    ::std::string & str
)
{
    size_t
        i = str.size();

    while(  i > 0
        &&  isspace( (unsigned char)(str[i-1]) )
    )
    {
        --i;
    };

    return str.erase(i, str.size());
}


////////////////////////////////////////////////////////////////////////////////
inline ::std::string &
trim_in_place(
    ::std::string & str
)
{
    return trim_left_in_place( trim_right_in_place(str) );
}


////////////////////////////////////////////////////////////////////////////////
// returns newly created strings
inline ::std::string
trim_right(
    ::std::string str
)
{
    return trim_right_in_place(str);
}


////////////////////////////////////////////////////////////////////////////////
inline ::std::string
trim_left(
    ::std::string str
)
{
    return trim_left_in_place(str);
}


////////////////////////////////////////////////////////////////////////////////
inline ::std::string
trim(
    ::std::string str
)
{
    return trim_left_in_place(trim_right_in_place(str));
}


////////////////////////////////////////////////////////////////////////////////
inline
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


////////////////////////////////////////////////////////////////////////////////
inline
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


////////////////////////////////////////////////////////////////////////////////
template<class T>
QString
QString_number(
    T value
)
{
    return QString::number(value);
}

template<>
inline
QString
QString_number<double>(
    double value
)
{
    return QString::number(value,'g',666);
}

template<>
inline
QString
QString_number<float>(
    float value
)
{
    return QString_number<double>(value);
}





/** Encode a byte-sequence into a string representing
    each byte as two hexadecimal digits [0-9A-F].
*/
void
    dataToHexString(
            unsigned char const * inBufferToEncode
        ,   unsigned int          inBufferToEncodeLen
        ,   ::std::string         & outEncodedBuffer
        ,   bool                  inUseUpperCase
        );


/** Convert a string of hexadecimal a byte-sequence into a byte-array.
The length of the target buffer must be sufficiently big, i.e. at least
half the size of the input string.
The input-strings size must be an even number.
*/
void
    hexStringToData(
            unsigned char      * inBufferToDecodeTo
        ,   ::std::string const  & inEncodedBuffer
        );

}
