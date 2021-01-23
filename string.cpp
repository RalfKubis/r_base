/* Copyright (C) Ralf Kubis */

#include "r_base/string.h"

#include "r_base/language_tools.h"

#include <algorithm>

using namespace std;

namespace nsBase
{

////////////////////////////////////////////////////////////////////////////////

string
replaced_allRegexp(
    string const & inSource
,   regex  const & inRegexp
,   string const & inSubstitute
)
{
    ostringstream
        ss;

    ::std::regex_replace(
            ostreambuf_iterator<char>(ss)  // OutputIt out
        ,   inSource.begin()               // BidirIt first
        ,   inSource.end()                 // BidirIt last
        ,   inRegexp                       // re
        ,   inSubstitute                   // fmt
        // ,   ::std::regex_constants::match_flag_type flags = ::std::regex_constants::match_default
        );

    return ss.str();
}


////////////////////////////////////////////////////////////////////////////////
string
replaced_first(
    string const & inSource
,   string const & inFrom
,   string const & inTo
)
{
    string
        result;

    auto
        pos_next = inSource.find(inFrom,0);

    if ( pos_next==string::npos )
        return inSource;

    auto
        itBegin = inSource.begin();

    result.append( itBegin, itBegin+pos_next );
    result.append( inTo );
    result.append( itBegin+pos_next+inFrom.length(), inSource.end() );

    return result;
}


////////////////////////////////////////////////////////////////////////////////
string
replaced_all(
    string const & inSource
,   string const & inFrom
,   string const & inTo
)
{
    string::size_type
        from = 0;

    string
        result;

    for (;;)
    {
        auto
            to = inSource.find(inFrom, from);

        if ( to==string::npos )
        {
            result.append( inSource.begin() + from, inSource.end() );
            break;
        }

        result.append( inSource.begin() + from, inSource.begin() + to );

        result += inTo;

        from = to + inFrom.size();
    }

    return result;
}


////////////////////////////////////////////////////////////////////////////////
string
unpadded(
    string const & inSource
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

    string
        retVal;

    copy(
            from
        ,   to
        ,   back_inserter(retVal)
        );

    return retVal;
}


////////////////////////////////////////////////////////////////////////////////
vector<string>
split(
    ::std::string const & inSource
,   ::std::string const & inDelimiter
)
{
    vector<string>
        retVal;

    BLOCK
    {
        if ( DBC_FAIL(!inDelimiter.empty()) )
            LEAVE;

        string::size_type
            from = 0;
        string::size_type
            to = inSource.size();

        for(;;)
        {
            if ( from>=to )
                break;

            auto next = inSource.find(
                    inDelimiter
                ,   from
                );


            if ( next==string::npos )
            {
                retVal.push_back( inSource.substr(from) );
                break;
            }

            auto count = next-from;

            retVal.push_back( inSource.substr(from,count) );

            from = next + inDelimiter.size();
        }
    }
    FIN

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////
auto
escaped(
    string const & inSource
)
-> string
{
    return  "\""
        +   replaced_all(
                    replaced_all(inSource,"\\","\\\\")
                ,   "\""
                ,   "\\\""
                )
        +   "\""
        ;
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
    unsigned char      * inBufferToDecodeTo
,   ::std::string const  & inEncodedBuffer
)
{
    DBC_PRE( inBufferToDecodeTo!=nullptr )
    DBC_PRE( inEncodedBuffer.size()%2==0 )

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
