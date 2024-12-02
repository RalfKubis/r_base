/* Copyright (C) Ralf Kubis */

#include "r_base/string.h"

#include "r_base/language_tools.h"

#include <algorithm>
#include <sstream>


namespace nsBase
{

::std::string
replaced_allRegexp(
    ::std::string const & inSource
,   ::std::regex  const & inRegexp
,   ::std::string const & inSubstitute
)
{
    ::std::ostringstream
        ss;

    ::std::regex_replace(
            ::std::ostreambuf_iterator<char>(ss)  // OutputIt out
        ,   inSource.begin()               // BidirIt first
        ,   inSource.end()                 // BidirIt last
        ,   inRegexp                       // re
        ,   inSubstitute                   // fmt
        // ,   ::std::regex_constants::match_flag_type flags = ::std::regex_constants::match_default
        );

    return ss.str();
}


::std::string
replaced_first(
    ::std::string const & inSource
,   ::std::string const & inFrom
,   ::std::string const & inTo
)
{
    ::std::string
        result;

    auto
        pos_next = inSource.find(inFrom,0);

    if (pos_next==::std::string::npos)
        return inSource;

    auto
        itBegin = inSource.begin();

    result.append(itBegin, itBegin+pos_next);
    result.append(inTo);
    result.append(itBegin+pos_next+inFrom.length(), inSource.end());

    return result;
}


::std::string
replaced_all(
    ::std::string const & inSource
,   ::std::string const & inFrom
,   ::std::string const & inTo
)
{
    ::std::string::size_type
        from = 0;

    ::std::string
        result;

    for (;;)
    {
        auto
            to = inSource.find(inFrom, from);

        if (to==::std::string::npos)
        {
            result.append(inSource.begin() + from, inSource.end());
            break;
        }

        result.append(inSource.begin() + from, inSource.begin() + to);

        result += inTo;

        from = to + inFrom.size();
    }

    return result;
}


::std::string
unpadded(
    ::std::string const & inSource
)
{
    auto
        from = inSource.begin();
    auto
        to = inSource.end();

    // skip leading blanks
    for( ; from!=to ; ++from )
    {
        if (    *from != ' '
            &&  *from != '\t'
        )
            break;
    }

    // skip trailing blanks
    for( ; to!=from ; --to )
    {
        auto
            last = --to;

        if (    *last != ' '
            &&  *last != '\t'
        )
        {
            ++to;
            break;
        }
    }

    ::std::string
        retVal;

    copy(
            from
        ,   to
        ,   back_inserter(retVal)
        );

    return retVal;
}



::std::vector<::std::string>
split(
    ::std::string_view const & source
,   ::std::string_view const & delimiter
,   bool                       keep_delimiter
)
{
    ::std::vector<::std::string>
        retVal;

    BLOCK
    {
        if (DBC_FAIL(!delimiter.empty()))
            LEAVE;

        if (source.empty())
            LEAVE;

        ::std::string::size_type from = 0;
        ::std::string::size_type to   = source.size();

        for (;;)
        {
            auto next = source.find(delimiter, from);

            if (next==::std::string_view::npos)
            {
                retVal.push_back(::std::string{source.substr(from)}); // the rest
                break;
            }

            auto count = next-from;

            if (keep_delimiter)
                count += delimiter.size();

            retVal.push_back(::std::string{source.substr(from,count)});

            from = next + delimiter.size();
        }
    }
    FIN

    return retVal;
}


::std::pair<::std::string, ::std::string>
split_at_first_occurrence_of(
    ::std::string_view const & source
,   ::std::string_view const & delimiter
,   bool                       keep_delimiter
)
{
    ::std::pair<::std::string, ::std::string>
        ss;

    BLOCK
    {
        if (DBC_FAIL(!delimiter.empty()))
        {
            ss.first = source;
            LEAVE;
        }

        if (source.empty())
            LEAVE;

        auto index = source.find(delimiter);

        if (index==::std::string_view::npos)
        {
            ss.first = source;
            LEAVE;
        }

        auto count = index;

        if (keep_delimiter)
            count += delimiter.size();

        ss.first  = source.substr(0,count);
        ss.second = source.substr(index + delimiter.size());
    }
    FIN

    return ss;
}


auto
escaped(
    ::std::string const & source
)
-> ::std::string
{
    return  "\""
        +   replaced_all(
                    replaced_all(source,"\\","\\\\")
                ,   "\""
                ,   "\\\""
                )
        +   "\""
        ;
}


auto
quoted(
    ::std::string const & inSource
)
-> ::std::string
{
    ::std::stringstream
        ss;
        ss << ::std::quoted(inSource);

    return ss.str();
}


void
dataToHexString(
    unsigned char const * inBufferToEncode
,   unsigned int          inBufferToEncodeLen
,   ::std::string         & outEncodedBuffer
,   bool                  inUseUpperCase
)
{
    DBC_PRE( inBufferToEncode!=nullptr || inBufferToEncodeLen==0 )

    outEncodedBuffer.resize(2*inBufferToEncodeLen);

    static char const * dictU = "0123456789ABCDEF";
    static char const * dictL = "0123456789abcdef";

    char const * dict = inUseUpperCase ? dictU : dictL;

    unsigned char const *
        src = inBufferToEncode;
    char *
        dst = &outEncodedBuffer[0];

    for ( unsigned int i=inBufferToEncodeLen ; i-- ; )
    {
        *dst++ = dict[(*src >>  4)];
        *dst++ = dict[(*src & 0xf)];
        src++;
    }
}


void
hexStringToData(
    unsigned char             * inBufferToDecodeTo
,   ::std::string_view const  & inEncodedBuffer
)
{
    DBC_PRE(inBufferToDecodeTo)
    DBC_PRE(inEncodedBuffer.size()%2==0)

    if ( inEncodedBuffer.empty() )
        return;

    char const *
        src = &inEncodedBuffer[0];
    unsigned char *
        dst = inBufferToDecodeTo;

    while ( *src != 0 )
    {
        char h = *src++;
        char l = *src++;

        if ( h>='a' )
            h = h-'a' + 10;
        else if ( h>='A' )
            h = h-'A' + 10;
        else
            h = h-'0';

        if ( l>='a' )
            l = l-'a' + 10;
        else if ( l>='A' )
            l = l-'A' + 10;
        else
            l = l-'0';

        DBC_ASSERT( h>=0 );
        DBC_ASSERT( l>=0 );

        DBC_ASSERT( h<16 );
        DBC_ASSERT( l<16 );

        *dst++ = ((unsigned char)h<<4) | (unsigned char)l;
    }
}

}
