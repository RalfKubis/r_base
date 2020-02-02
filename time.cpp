/* Copyright (C) Ralf Kubis */
#include "r_base/time.h"

#include "r_base/string.h"

#include <algorithm>
#include <ctime>
#include <vector>
#include <chrono>


using namespace ::std::literals;


namespace std::chrono
{

::std::tm
to_tm(
    time_point<system_clock> const & tp
,   bool                             as_utc_time
)
{
    auto
        t = ::std::chrono::system_clock::to_time_t(tp);

    return as_utc_time ? *::std::gmtime(&t) : *::std::localtime(&t);
}


::std::string
to_string(
    time_point<system_clock> const & tp
,   bool                             as_utc_time
,   bool                             with_millis
)
{
    auto
        tm = to_tm(tp, as_utc_time);
    auto
        duration = tp.time_since_epoch();
    auto
        format = "%Y-%m-%d %H:%M:%S";

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
        ,   format
        ,   &tm
        ));

    if (with_millis)
    {
        auto
            millis = ::std::to_string(::std::chrono::duration_cast<::std::chrono::milliseconds>(duration).count() % 1000);
            millis = ::std::string(::std::max<int>(0,3-millis.size()),'0') + millis;

        buf += '.' + millis;
    }

    return buf;

#endif
}

}


namespace nsBase{namespace time
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
// https://stackoverflow.com/questions/25393683/stdstring-to-stdchrono-time-point/25396781
struct Tm
:   ::std::tm
{
    int
        tm_usecs; // [0, 999999] micros after the sec

    Tm(
            int year
        ,   int month
        ,   int mday
        ,   int hour
        ,   int min
        ,   int sec
        ,   int usecs
        ,   int isDST = -1
        )
        :   tm_usecs { usecs }
        {
            tm_year  = year - 1900; // [0, 60] since 1900
            tm_mon   = month - 1;   // [0, 11] since Jan
            tm_mday  = mday;        // [1, 31]
            tm_hour  = hour;        // [0, 23] since midnight
            tm_min   = min;         // [0, 59] after the hour
            tm_sec   = sec;         // [0, 60] after the min
                                    //         allows for 1 positive leap second
            tm_isdst = isDST;       // [-1...] -1 for unknown, 0 for not DST,
                                    //         any positive value if DST.
        }

    // HACK
    // https://stackoverflow.com/questions/38298261/why-there-is-no-inverse-function-for-gmtime-in-libc
    // UTC calendar time -> UTC timestamp
    time_t
        timegm(
                tm * tm_utc
            )
            {
                auto
                    timestamp_local = mktime(tm_utc); // pretend tm_utc to be tm_local

                decltype(timestamp_local)
                    offset = 0;

                if (timestamp_local<0)
                {
                    tm_utc->tm_year++;
                    timestamp_local = mktime(tm_utc);
                    tm_utc->tm_year--;

                    if (timestamp_local!=-1)
                        offset = -31536000; // number of seconds in 1970
                }

                if (timestamp_local<0)
                    return -1;

                auto
                    timestamp_utc = mktime(gmtime(&timestamp_local));
                auto
                    seconds_utc_to_local = timestamp_local - timestamp_utc; // this may still be wrong

                return timestamp_local + seconds_utc_to_local + offset;
            }

    time_point_t to_time_point()
                {
                    auto timestamp_utc = timegm(this);

                    if (timestamp_utc<0)
                        return time_point_t();

                    return ::std::chrono::system_clock::from_time_t(timestamp_utc) + ::std::chrono::microseconds{ tm_usecs };
                }
};
} // ns


time_point_t
time_from_string(
    ::std::string const & s
)
{
    // %Y-%m-%d %H:%M:%S.U
    auto
        ss = s;

    for (auto & c : ss)
    {
        if (c<'0' || c>'9')
            c=' ';
    }

    auto
        sss = nsBase::split(ss,u8" "s);
    ::std::vector<int>
        nnn;

    ::std::transform(
            sss.begin()
        ,   sss.end()
        ,   ::std::back_inserter(nnn)
        ,   [](::std::string const & s)
            {
                try{return ::std::stoi(s);}catch(...){return 0;}
            }
        );

    nnn.resize(7,0);

    auto
        t = Tm(
                nnn[0]      // year
            ,   nnn[1]      // month
            ,   nnn[2]      // mday
            ,   nnn[3]      // hour
            ,   nnn[4]      // min
            ,   nnn[5]      // sec
            ,   nnn[6]*1000 // usecs
            ,   0
            ).to_time_point();

    return t;
}


time_point_t
time_from_seconds_and_micros(
    ::std::int64_t seconds
,   ::std::int32_t micros
)
{
    return ::std::chrono::system_clock::from_time_t(::std::time_t(seconds)) + micros * 1us;
}


}}
