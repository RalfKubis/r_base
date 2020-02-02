#pragma once
/* Copyright (C) Ralf Kubis */

#ifdef _WIN32
    #include <filesystem>
    namespace fs = ::std::filesystem;
#else
    #include <experimental/filesystem>
    namespace fs = ::std::experimental::filesystem;
#endif

#include <algorithm>
#include <optional>
#include <string>

#include <QString>

#include <r_base/string.h>


inline auto
    operator "" _path(
            char        const * data
        ,   ::std::size_t       len
        )
        ->  ::fs::path
        {
            return ::fs::u8path(data);
        }

inline ::std::string
    P2S(
            ::fs::path const & path
        )
        {
            return path.generic_u8string();
        }

inline ::fs::path
    S2P(
            ::std::string const & spath
        )
        {
            return ::fs::u8path(spath);
        }

inline QString
    P2Q(
            ::fs::path const & path
        )
        {
            return ::nsBase::S2Q(P2S(path));
        }

inline ::fs::path
    Q2P(
            QString const & qpath
        )
        {
            return S2P(::nsBase::Q2S(qpath));
        }


//KU: keep nested until we drop Qt 5.4 - moc fails on nested namespace definitions
#ifdef _WIN32
namespace std{ namespace filesystem
#else
namespace std{namespace experimental{ namespace filesystem
#endif
{
inline QString
    S2Q(
            ::fs::path const & path
        )
        {
            return ::nsBase::S2Q(path.u8string());
        }

inline ::std::size_t
    size(::fs::path const & p)
        {
            return ::std::distance(p.begin(), p.end());
        }

inline ::std::string
    to_string(::fs::path const & p)
        {
            return p.u8string();
        }

// get the tail of a path if it has the given prefix, otherwise an empty optional
inline ::std::optional<::fs::path>
    tail_if(
            ::fs::path const & prefix
        ,   ::fs::path const & path
        )
        {
            auto it_prefix = prefix.begin();
            auto it_path   = path.begin();

            //TODO: use ::std::mismatch
            for(;;)
            {
                if (it_prefix==prefix.end())
                    break;

                if (it_path==path.end())
                    return {};

                if (*it_path!=*it_prefix)
                    return {};

                ++it_prefix;
                ++it_path;
            }

            ::fs::path
                tail;

            ::std::for_each(
                    it_path
                ,   path.end()
                ,   [&](auto & s){tail /= s;}
                );

            return tail;
        }

#ifdef _WIN32
}}
#else
}}}
#endif
