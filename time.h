﻿#pragma once
/* Copyright (C) Ralf Kubis */

#include "r_base/decl.h"

#include <chrono>
#include <string>
#include <cstdint>
#include <ctime>
#include <optional>
#include <array>
#include <compare>

#include <time.h>


namespace nsBase::time
{

using time_point_t              = ::std::chrono::system_clock::time_point;
using time_point_optional_t     = ::std::optional<time_point_t>;

using time_duration_t           = ::std::chrono::system_clock::duration;
using time_duration_optional_t  = ::std::optional<time_duration_t>;

using day_time_t                = ::std::chrono::hh_mm_ss<time_duration_t>;
using day_time_optional_t       = ::std::optional<day_time_t>;

using date_t                    = ::std::chrono::year_month_day;
using date_optional_t           = ::std::optional<date_t>;


time_point_t
    now();


bool
is_null(
        time_point_t const & tp
    );


inline ::std::tm *
safe_localtime(
    ::std::tm     * to
,   ::std::time_t * from
)
{
    #ifdef WIN32
        if (::localtime_s(to, from))
            return {};
        return to;
    #else
        return localtime_r(from, to);
    #endif
}


inline ::std::tm *
safe_gmtime(
    ::std::tm     * to
,   ::std::time_t * from
)
{
    #ifdef WIN32
        if (::gmtime_s(to, from))
            return {};
        return to;
    #else
        return gmtime_r(from, to);
    #endif
}


time_point_optional_t
    time_from_string(
            ::std::string_view  const & s
        ,   bool                        utc
        ,   ::std::array<int,7> const & mapping // index of [year,month,mday,hour,min,sec,usec] in string
        ,   bool                        fix_short_year = {} // offset two-digit-years by 2000
        );

// convert from string conform with rfc3339 "Date and Time on the Internet"
//       e.g. 1985-04-12T23:20:50.52Z or 1990-12-31T15:59:60-08:00
time_point_optional_t
    time_from_string_rfc3339(
            ::std::string_view const & s
        );

// convert from UTC time string YYYY*MM*DD*HH*mm*ss*mmm (where * is any sequence of non-digits)
inline time_point_optional_t
    time_from_string_utc_YYYY_MM_DD_HH_mm_ss_mmm(
            ::std::string_view const & s
        )
        {
            return time_from_string(s, true, {0,1,2,3,4,5,6});
        }

inline time_point_optional_t
    time_from_string_local_YYYY_MM_DD_HH_mm_ss_mmm(
            ::std::string_view const & s
        )
        {
            return time_from_string(s, false, {0,1,2,3,4,5,6});
        }

inline time_point_optional_t
    time_from_string_utc_DD_MM_YYYY_HH_mm_ss_mmm(
            ::std::string_view const & s
        )
        {
            return time_from_string(s, true, {2,1,0,3,4,5,6});
        }

inline time_point_optional_t
    time_from_string_local_DD_MM_YYYY_HH_mm_ss_mmm(
            ::std::string_view const & s
        )
        {
            return time_from_string(s, false, {2,1,0,3,4,5,6});
        }

inline time_point_optional_t
    time_from_string_local_DD_MM_YY_HH_mm_ss_mmm(
            ::std::string_view const & s
        )
        {
            return time_from_string(s, false, {2,1,0,3,4,5,6}, true);
        }



time_point_t
    time_from_seconds_and_micros(
            ::std::int64_t seconds
        ,   ::std::int32_t micros
        );

time_point_t
    time_from_calendar(
            bool                         utc
        ,   ::std::optional<int> const & year   = {} // years since year 0
        ,   ::std::optional<int> const & month  = {} // month - [1, 12]
        ,   ::std::optional<int> const & mday   = {} // day of the month - [1, 31]
        ,   ::std::optional<int> const & hour   = {} // hours since midnight - [0, 23]
        ,   ::std::optional<int> const & min    = {} // minutes after the hour - [0, 59]
        ,   ::std::optional<int> const & sec    = {} // seconds after the minute - [0, 60] including leap second
        ,   ::std::optional<int> const & usecs  = {} // micro-seconds [0, 1000000]
        );

inline time_point_t
    time_from_calendar_utc(
            ::std::optional<int> const & year   = {} // years since year 0
        ,   ::std::optional<int> const & month  = {} // month - [1, 12]
        ,   ::std::optional<int> const & mday   = {} // day of the month - [1, 31]
        ,   ::std::optional<int> const & hour   = {} // hours since midnight - [0, 23]
        ,   ::std::optional<int> const & min    = {} // minutes after the hour - [0, 59]
        ,   ::std::optional<int> const & sec    = {} // seconds after the minute - [0, 60] including leap second
        ,   ::std::optional<int> const & usecs  = {} // micro-seconds [0, 1000000]
        )
        {
            return time_from_calendar(true, year, month, mday, hour, min, sec, usecs);
        }

inline time_point_t
    time_from_calendar_local(
            ::std::optional<int> const & year   = {} // years since year 0
        ,   ::std::optional<int> const & month  = {} // month - [1, 12]
        ,   ::std::optional<int> const & mday   = {} // day of the month - [1, 31]
        ,   ::std::optional<int> const & hour   = {} // hours since midnight - [0, 23]
        ,   ::std::optional<int> const & min    = {} // minutes after the hour - [0, 59]
        ,   ::std::optional<int> const & sec    = {} // seconds after the minute - [0, 60] including leap second
        ,   ::std::optional<int> const & usecs  = {} // micro-seconds [0, 1000000]
        )
        {
            return time_from_calendar(false, year, month, mday, hour, min, sec, usecs);
        }

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
////////////////////////////////////////////////////////////////////////////////
//  Time Point
//

/** Convert into a (UTC or Local) time string.
    If with_millis id set, [.mmm] is appended.
    If with_millis id set, [.uuuuuu] is appended.
*/
::std::string
    to_string(
            ::nsBase::time::time_point_optional_t   const & tp
        ,   bool                                            as_utc      = true
        ,   bool                                            with_millis = true
        ,   ::std::string                           const & format      = "%Y-%m-%d %H:%M:%S"
        ,   bool                                            with_micros = false
        );

// convert into a Local time string 'DD.MM.YYYY HH:mm'
inline ::std::string
    to_string_local_DD_MM_YYYY_HH_mm(
            ::nsBase::time::time_point_optional_t const & tp
        )
        {
            return to_string(
                    tp
                ,   false
                ,   false
                ,   "%d.%m.%Y %H:%M"
                );
        }

// convert into a UTC Date string 'YYYY-MM-DD'
inline ::std::string
    to_string_utc_YYYY_MM_DD(
            ::nsBase::time::time_point_optional_t const & tp
        )
        {
            return to_string(
                    tp
                ,   true
                ,   false
                ,   "%Y-%m-%d"
                );
        }

inline ::std::string
    to_string_iso_utc(
            ::nsBase::time::time_point_optional_t const & tp
        ,   bool                                          with_subseconds = true
        ,   bool                                          with_micros = false
        )
        {
            if (!tp)
                return {};

            return to_string(
                    tp
                ,   true // as_utc
                ,   with_subseconds
                ,   "%Y-%m-%dT%H:%M:%S"
                ,   with_micros
                ) + 'Z';
        }


// convert into calendar date time
::std::tm
    to_calendar(
            ::nsBase::time::time_point_optional_t const & tp
        ,   bool                                          utc
        );

inline ::std::tm
    to_calendar_utc(
            ::nsBase::time::time_point_optional_t const & tp
        )
        {
            return to_calendar(tp, true);
        }

inline ::std::tm
    to_calendar_local(
            ::nsBase::time::time_point_optional_t const & tp
        )
        {
            return to_calendar(tp, false);
        }


inline ::nsBase::time::date_t
    to_date(
            ::nsBase::time::time_point_t const & tp
        ,   bool                                 utc
        )
        {
            auto tm = to_calendar(tp, utc);

            return ::nsBase::time::date_t
                {   ::std::chrono:: year{tm.tm_year + 1900}
                ,   ::std::chrono::month{static_cast<unsigned int>(tm.tm_mon + 1)}
                ,   ::std::chrono::  day{static_cast<unsigned int>(tm.tm_mday)}
                };
        }

inline ::nsBase::time::date_t
    to_utc_date(
            ::nsBase::time::time_point_t const & tp
        )
        {
            return to_date(tp, true);
        }

inline ::nsBase::time::date_t
    to_local_date(
            ::nsBase::time::time_point_t const & tp
        )
        {
            return to_date(tp, false);
        }

}


////////////////////////////////////////////////////////////////////////////////
//  Duration
//
namespace std::chrono
{
// convert into a number of seconds with fractionals as string 'sss.sss'
::std::string
    to_string_ss(
            ::nsBase::time::time_duration_optional_t const &
        );

::std::string
    to_string_HH_mm_ss(
            ::nsBase::time::time_duration_optional_t const &
        ,   bool                                             with_micros = {}
        );

[[nodiscard]] inline constexpr ::std::strong_ordering
    operator<=>(
            ::nsBase::time::day_time_optional_t const & a
        ,   ::nsBase::time::day_time_optional_t const & b
        ) noexcept
        {
            if (a.has_value() && b.has_value())
                return a->to_duration() <=> b->to_duration();

            return a.has_value() <=> b.has_value();
        }
}

namespace nsBase::time
{
::nsBase::time::time_duration_optional_t
    duration_from_HH_mm_ss(
            ::std::string_view const & hhmmsss
        );
}


////////////////////////////////////////////////////////////////////////////////
//  Date
//
namespace std::chrono
{

inline ::std::string
to_string(
    ::nsBase::time::date_t const & d
)
{
    return      ::std::to_string(static_cast<int>(         d. year() ))
        + "_" + ::std::to_string(static_cast<int>(unsigned(d.month())))
        + "_" + ::std::to_string(static_cast<int>(unsigned(d.  day())))
        ;
}

}
