#pragma once
/* Copyright (C) Ralf Kubis */

#include "r_base/decl.h"

#include <chrono>
#include <string>
#include <cstdint>
#include <ctime>


namespace nsBase::time
{

using
    time_point_t = ::std::chrono::time_point<::std::chrono::system_clock>;
using
    time_duration_t = ::std::chrono::duration<::std::chrono::system_clock>;

time_point_t
    now();

bool
    is_null(
            time_point_t const & tp
        );



// convert from UTC time string YYYY*MM*DD*HH*mm*ss*mmm (where * is any sequence of non-digits)
time_point_t
    time_from_string(
            ::std::string const & s
        );

time_point_t
    time_from_seconds_and_micros(
            ::std::int64_t seconds
        ,   ::std::int32_t micros
        );


inline int
    year(
            ::std::tm const & t
        )
        {
            return t.tm_year + 1900;
        }
}


namespace std::chrono
{
// convert into a (UTC or Local) time string YYYY-MM-DD HH:mm:ss[.mmm]
::std::string
    to_string(
            time_point<system_clock> const & tp
        ,   bool                             as_utc_time = true
        ,   bool                             with_millis = true
        );

// convert into calendar date and time
::std::tm
    to_tm(
            time_point<system_clock> const & tp
        ,   bool                             as_utc_time = true
        );

}
