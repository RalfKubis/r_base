/* Copyright (C) Ralf Kubis */

#include "r_base/filesystem.h"

#include <fmt/format.h>

#include <numeric>
#include <thread>
#include <chrono>


#ifdef _WIN32
namespace std{ namespace filesystem
#else
namespace std{namespace experimental{ namespace filesystem
#endif
{

void
safe_rename(
    ::fs::path const & from
,   ::fs::path const & to
)
{
    auto start = ::std::chrono::system_clock::now();

    while(::std::chrono::system_clock::now() - start < 20s)
    {
        try
        {
            if (!::fs::exists(from))
                return;

            if (::fs::exists(to))
                return;

            ::fs::rename(from,to);

            return;
        }
        catch(::std::exception & e)
        {
            auto msg = e.what();
            ::fmt::print("exception: {}\n", msg);

            ::std::this_thread::sleep_for(100ms);
        }
    }

    ::fmt::print("failed to move: {} -> {}\n", P2S(from), P2S(to));
}


void
safe_remove(
    ::fs::path const & path
)
{
    auto start = ::std::chrono::system_clock::now();

    while(::std::chrono::system_clock::now() - start < 20s)
    {
        try
        {
            if (!::fs::exists(path))
                return;

            ::fs::remove(path);

            return;
        }
        catch(::std::exception & e)
        {
            auto msg = e.what();
            ::fmt::print("exception: {}\n", msg);

            ::std::this_thread::sleep_for(100ms);
        }
    }

    ::fmt::print("failed to remove: {}\n", P2S(path));
}


void
safe_remove_all(
    ::fs::path const & path
)
{
    auto start = ::std::chrono::system_clock::now();

    while(::std::chrono::system_clock::now() - start < 20s)
    {
        try
        {
            if (!::fs::exists(path))
                return;

            ::fs::remove_all(path);

            return;
        }
        catch(::std::exception & e)
        {
            auto msg = e.what();
            ::fmt::print("exception: {}\n", msg);

            ::std::this_thread::sleep_for(100ms);
        }
    }

    ::fmt::print("failed to remove: {}\n", P2S(path));
}


::fs::path
slash_format(
    ::fs::path const & path
)
{
    auto p = ::std::accumulate(
            path.begin()
        ,   path.end()
        ,   ::std::string{}
        ,   [&](auto & acc, auto & p)
            {
                auto s = P2S(p);
                auto sep = (acc.empty() || *acc.rbegin()=='/') ? "" : "/";
                return acc + sep + s;
            }
        );

    return ::fs::u8path(p);
}


#ifdef _WIN32
}}
#else
}}}
#endif
