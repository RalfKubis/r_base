#pragma once
/* Copyright (C) Ralf Kubis */

#include "r_base/decl.h"

#include <chrono>
#include <string>
#include <cstdint>
#include <ctime>
#include <optional>


namespace nsBase{namespace time
{

using
    time_point_t = ::std::chrono::time_point<::std::chrono::system_clock>;
using
    time_point_optional_t = ::std::optional<time_point_t>;

using
    time_duration_t = ::std::chrono::duration<::std::chrono::system_clock>;
using
    time_duration_optional_t = ::std::optional<time_duration_t>;


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

::std::optional<time_point_t>
    time_from_string_if(
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
}}


namespace std{namespace chrono
{

// convert into a (UTC or Local) time string YYYY-MM-DD HH:mm:ss[.mmm]
::std::string
    to_string(
            time_point<system_clock> const & tp
        ,   bool                             as_utc_time = true
        ,   bool                             with_millis = true
        ,   ::std::string            const & format      = "%Y-%m-%d %H:%M:%S"
        );

// convert into calendar date and time
::std::tm
    to_tm(
            time_point<system_clock> const & tp
        ,   bool                             as_utc_time = true
        );

}}
