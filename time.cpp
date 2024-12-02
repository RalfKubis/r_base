/* Copyright (C) Ralf Kubis */

#include "r_base/time.h"
#include "r_base/string.h"
#include "r_base/Log.h"

#include <algorithm>
#include <ctime>
#include <vector>
#include <chrono>
#include <regex>

#include <fmt/format.h>


using namespace ::std::literals;


////////////////////////////////////////////////////////////////////////////////
//
//  time point
//
////////////////////////////////////////////////////////////////////////////////
namespace std::chrono
{

::std::tm
to_calendar(
    ::nsBase::time::time_point_optional_t const & tp
,   bool                                          utc
)
{
    ::std::tm
        tm;

    int err {};

    auto tt = tp ? ::std::chrono::system_clock::to_time_t(*tp) : 0;

    if (tt<0)
        tt = 0;

    if (utc)
    {
        if (!::nsBase::time::safe_gmtime(&tm, &tt))
            "b58e2c17-ae1c-4bb1-b2b4-32a7a26b705c"_log["time"]("to_calender()").throw_error();
    }
    else
    {
        if (!::nsBase::time::safe_localtime(&tm, &tt))
            "e66094de-0a3b-43cd-bcfd-fbf93f084594"_log["time"]("to_calender()").throw_error();
    }

    return tm;
}


::std::string
to_string(
    ::nsBase::time::time_point_optional_t const & tp
,   bool                                          utc
,   bool                                          with_millis
,   ::std::string                         const & format
,   bool                                          with_micros
)
{
    if (!tp)
        return {};

    auto
        tm = to_calendar(tp, utc);
    auto
        duration = tp->time_since_epoch();

#if 0 // ::std::put_time() is not supported by gcc < 5

    ::std::ostringstream
        os;
        os << ::std::put_time(tm, format) << millis;

    return os.str();

#else

    ::std::string
        buf(64,'\0');
        buf.resize(::std::strftime(
                const_cast<char*>(buf.data())
            ,   buf.size()
            ,   format.c_str()
            ,   &tm
            ));

    if (with_micros)
    {
        auto
            micros = ::std::to_string(::std::chrono::duration_cast<::std::chrono::microseconds>(duration).count() % 1'000'000);
            micros = ::std::string(::std::max<int>(0,6-micros.size()),'0') + micros;

        buf += '.' + micros;
    }
    else if (with_millis)
    {
        auto
            millis = ::std::to_string(::std::chrono::duration_cast<::std::chrono::milliseconds>(duration).count() % 1'000);
            millis = ::std::string(::std::max<int>(0,3-millis.size()),'0') + millis;

        buf += '.' + millis;
    }

    return buf;

#endif
}

}


namespace nsBase::time
{

time_point_t
now()
{
    return ::std::chrono::system_clock::now();
}


bool
is_null(
    time_point_t const & tp
)
{
    return tp.time_since_epoch().count()==0;
}


namespace
{
::std::time_t
time_from_local_calendar(
    tm * tm_local
)
{
    return ::std::mktime(tm_local);
}


// UTC calendar time -> timestamp
::std::time_t
time_from_utc_calendar(
    tm * tm_utc
)
{
/* HACK
https://stackoverflow.com/questions/38298261/why-there-is-no-inverse-function-for-gmtime-in-libc

Due to the lack of timegm() as an "inverse" of gmtime() I apply the following hack.

(1) Pretend the parameter calernder time to be local calendet, instead of UTC
and compute its timestamp 'ts_0' via mktime().
ts_0 is no off by the 'seconds_utc_to_local'.

Example:
    calling time_from_utc_calendar('2021-04-29 13:10') // meant to be utc-calender

    ts_0 would represent '2021-04-29 11:10 UTC' - 2 Hours off, 7200 sec

(2) Determine 'seconds_utc_to_local'.
    - set cal_0_utc to represent ts_0 via gmtime(ts_0) - (still 2 hours off in the example)
    - set ts_1 to the timestamp tepresenting cal_0_utc as if it would be a local calender
        in the example, ts_1 would be 09:10 UTC, now 4 hours off

    - seconds_utc_to_local = ts_0 - ts1

(3)
ts = ts_0 + seconds_utc_to_local
*/


    auto
        ts_0 = time_from_local_calendar(tm_utc);

    decltype(ts_0)
        offset = 0;

    if (ts_0<0)
    {
        tm_utc->tm_year++;
        ts_0 = time_from_local_calendar(tm_utc);
        tm_utc->tm_year--;

        if (ts_0!=-1)
            offset = -31536000; // number of seconds in 1970
    }

    if (ts_0<0)
        return -1;

    time_t
        seconds_utc_to_local {};

        {
            ::std::tm
                cal_0_utc;
                safe_gmtime(&cal_0_utc, &ts_0);
                cal_0_utc.tm_isdst = -1;

            auto
                ts_1 = time_from_local_calendar(&cal_0_utc);

            seconds_utc_to_local = ts_0 - ts_1; // this may still be wrong
        }

    return ts_0 + seconds_utc_to_local + offset;
}



// https://stackoverflow.com/questions/25393683/stdstring-to-stdchrono-time-point/25396781
struct CalendarTime
:   ::std::tm
{
    int
        tm_usecs; // [0, 999999] micros after the sec

    bool
        is_utc;

    CalendarTime(
            int     year    // 2021
        ,   int     month   // 1..12
        ,   int     mday    // 1..
        ,   int     hour    // 0..
        ,   int     min     // 0..
        ,   int     sec     // 0..
        ,   int     usecs   // 0..
        ,   bool    utc
        )
        :   tm_usecs {usecs}
        ,   is_utc   {utc}
        {
            tm_year  = year - 1900; // [0, ..] since 1900
            tm_mon   = month - 1;   // [0, 11] since Jan
            tm_mday  = mday;        // [1, 31]
            tm_hour  = hour;        // [0, 23] since midnight
            tm_min   = min;         // [0, 59] after the hour
            tm_sec   = sec;         // [0, 60] after the min
                                    //         allows for 1 positive leap second
            tm_isdst = -1;          // [-1...] -1 for unknown, 0 for not DST,
                                    //         any positive value if DST.
        }

    time_point_t
        time_point()
                {
                    auto
                        timestamp = is_utc ? time_from_utc_calendar(this) : time_from_local_calendar(this);

                    if (timestamp<0)
                        return {};

                    return ::std::chrono::system_clock::from_time_t(timestamp) + ::std::chrono::microseconds{ tm_usecs };
                }
};
}



time_point_t
time_from_seconds_and_micros(
    ::std::int64_t seconds
,   ::std::int32_t micros
)
{
    return ::std::chrono::system_clock::from_time_t(::std::time_t(seconds)) + micros * 1us;
}


time_point_t
time_from_calendar(
    bool                         utc
,   ::std::optional<int> const & year
,   ::std::optional<int> const & month
,   ::std::optional<int> const & mday
,   ::std::optional<int> const & hour
,   ::std::optional<int> const & min
,   ::std::optional<int> const & sec
,   ::std::optional<int> const & usecs
)
{
    auto
        t = CalendarTime(
                year    .value_or(0)
            ,   month   .value_or(1)
            ,   mday    .value_or(1)
            ,   hour    .value_or(0)
            ,   min     .value_or(0)
            ,   sec     .value_or(0)
            ,   usecs   .value_or(0)
            ,   utc
            ).time_point();

    return t;
}


time_point_optional_t
time_from_string(
    ::std::string_view  const & s
,   bool                        utc
,   ::std::array<int,7> const & mapping
,   bool                        fix_short_year
)
{
    if (s.empty())
        return {};

    // %Y-%m-%d %H:%M:%S.U
    // %Y-%m-%dT%H:%M:%S.UZ
    auto
        ss = ::std::string{s};

    for (auto & c : ss)
    {
        if (c<'0' || c>'9')
            c=' ';
    }

    auto
        sss = ::nsBase::split(ss," "s);
        sss.resize(7,"0"s);

    ::std::vector<int>
        nnn;

    ::std::transform(
            sss.begin()
        ,   sss.end()
        ,   ::std::back_inserter(nnn)
        ,   [](::std::string const & s)
            {
                if (s.empty()) return 0;
                try{return ::std::stoi(s);}catch(...){return 0;}
            }
        );

    // fix_short_year
    if (fix_short_year && nnn[mapping[0]]<100)
        nnn[mapping[0]] += 2000;

    // extract fraction of a second
    auto fraction = [&]() -> double {try{return ::std::stod("0." + sss[mapping[6]]);}catch(...){return .0;}}();

    auto
        t = CalendarTime(
                nnn[mapping[0]]      // year
            ,   nnn[mapping[1]]      // month
            ,   nnn[mapping[2]]      // mday
            ,   nnn[mapping[3]]      // hour
            ,   nnn[mapping[4]]      // min
            ,   nnn[mapping[5]]      // sec
            ,   fraction * 1'000'000 // usecs
            ,   utc
            ).time_point();

    return t;
}


time_point_optional_t
time_from_string_rfc3339(
    ::std::string_view const & s
)
{
    // Regular Expression for Datetime compliant with rfc3339 "Date and Time on the Internet"
    ::std::regex tp_rgx("^((?:((\\d{4})-(\\d{2})-(\\d{2}))T((\\d{2}):(\\d{2}):(\\d{2})(?:\\.(\\d+))?))(Z|([\\+-])(\\d{2}):(\\d{2}))?)$");
    ::std::cmatch tp_match;

    if (!::std::regex_match(s.data(), s.data() + s.size(), tp_match, tp_rgx))
        return {};

    auto year    = ::std::stoi(tp_match[3].str());
    auto month   = ::std::stoi(tp_match[4].str());
    auto mday    = ::std::stoi(tp_match[5].str());
    auto hour    = ::std::stoi(tp_match[7].str());
    auto min     = ::std::stoi(tp_match[8].str());
    auto sec     = ::std::stoi(tp_match[9].str());

    // calculate usecs from fraction string
    auto fraction = tp_match[10].matched ? ::std::stod("0."s + tp_match[10].str()) : 0.0;
    auto usecs    = ::std::lround(1'000'000 * fraction);

    int timeoffset_minutes = 0;

    if (tp_match[11].str() != "Z"s) // tp_match[4] is something like -01:00 or Z
    {
        timeoffset_minutes  = ::std::stoi(tp_match[13].str()) * 60;
        timeoffset_minutes += ::std::stoi(tp_match[14].str());

        if (tp_match[12] == "+"s) // tp_match[5] is + or -
            timeoffset_minutes *= -1;
    }

    auto
        t = CalendarTime(
                year       // year
            ,   month      // month
            ,   mday       // mday
            ,   hour       // hour
            ,   min        // min
            ,   sec        // sec
            ,   usecs      // usecs
            ,   true
            ).time_point()
          + ::std::chrono::minutes(timeoffset_minutes);

    return t;
}

}


////////////////////////////////////////////////////////////////////////////////
//
//  duration
//
////////////////////////////////////////////////////////////////////////////////
namespace std::chrono
{

::std::string
to_string_ss(
    ::nsBase::time::time_duration_optional_t const & duration
)
{
    if (!duration)
        return {};

    auto micros_all = ::std::chrono::duration_cast<::std::chrono::microseconds>(*duration).count();
    auto seconds    = micros_all / 1'000'000;
    auto micros     = micros_all % 1'000'000;

    return ::fmt::format("{}.{:06}", seconds, micros);
}


::std::string
to_string_HH_mm_ss(
    ::nsBase::time::time_duration_optional_t const & duration
,   bool                                             with_micros
)
{
    if (!duration)
        return {};

    auto micros_all     = ::std::chrono::duration_cast<::std::chrono::microseconds>(*duration).count();
    auto micros         = micros_all % 1'000'000;
    auto seconds_remain = micros_all / 1'000'000;
    auto hh             = seconds_remain / 3600; seconds_remain = seconds_remain % 3600;
    auto mm             = seconds_remain /   60; seconds_remain = seconds_remain %   60;

    return  ::fmt::format("{:02}:{:02}:{:02}", hh, mm, seconds_remain)
        +   (with_micros ? ::fmt::format(".{:06}", micros) : ""s)
        ;
}
}


namespace nsBase::time
{

time_duration_optional_t
duration_from_HH_mm_ss(
    ::std::string_view const & hhmmsss
)
{
    using namespace ::std::chrono_literals;

    auto xxx = split(hhmmsss, ":");

    auto it = xxx.begin();

    time_duration_t
        d = 0s;

        if (it!=xxx.end()) d += ::std::atoi(it++->c_str()) * 1h;
        if (it!=xxx.end()) d += ::std::atoi(it++->c_str()) * 1min;
        if (it!=xxx.end()) d += ::std::atoi(it++->c_str()) * 1s;

    return d;
}
}
