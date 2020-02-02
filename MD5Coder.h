#pragma once
/* Copyright (C) Ralf Kubis */

#include "r_base/decl.h"
#include "r_base/language_tools.h"
#include "r_base/MD5.h"
#include "r_base/ID.h"
#include "r_base/Range.h"

#include <optional>
#include <memory>
#include <vector>
#include <map>
#include <set>
#include <valarray>
#include <string>


namespace nsBase
{


/** This class provides methods for converting between binary data and
    ASCII strings (uuencode) as well as computing checksums (CRC). */

class MD5Coder
{
    R_DTOR(MD5Coder);
    R_CTOR(MD5Coder);
    R_CCPY(MD5Coder) = delete;
    R_CMOV(MD5Coder);
    R_COPY(MD5Coder) = delete;
    R_MOVE(MD5Coder);

    public : struct
        MD5_CONTEXT;

    private : ::std::unique_ptr<MD5_CONTEXT>
        context;

    /** Append the next buffer to the checksum. */
    public : void
        merge(
                void  const * buffer
            ,   ::std::size_t   buffer_len
            );

    /** Get the resulting hash. No further data can be merged.
    */
    public : MD5
        query();

    /** Get the resulting hash. No further data can be merged.
    */
    public : void
        query(
                unsigned char target[16]
            );

    /** Clear the hash and restart the data merging phase.
    */
    public : void
        clear();
};


MD5Coder & operator<<(MD5Coder &, ::std::string const &);
MD5Coder & operator<<(MD5Coder &, ::nsBase::MD5 const &);
MD5Coder & operator<<(MD5Coder &, ::uuids::uuid const &);
MD5Coder & operator<<(MD5Coder &, ::std::chrono::system_clock::time_point const &);
MD5Coder & operator<<(MD5Coder &, ::std::int8_t);
MD5Coder & operator<<(MD5Coder &, ::std::uint8_t);
MD5Coder & operator<<(MD5Coder &, ::std::int16_t);
MD5Coder & operator<<(MD5Coder &, ::std::uint16_t);
MD5Coder & operator<<(MD5Coder &, ::std::int32_t);
MD5Coder & operator<<(MD5Coder &, ::std::uint32_t);
MD5Coder & operator<<(MD5Coder &, ::std::int64_t);
MD5Coder & operator<<(MD5Coder &, ::std::uint64_t);
MD5Coder & operator<<(MD5Coder &, bool);
MD5Coder & operator<<(MD5Coder &, float);
MD5Coder & operator<<(MD5Coder &, double);

template<typename T>
MD5Coder &
    operator<<(
            MD5Coder & c
        ,   ::std::optional<T> const & d
        )
        {
            using namespace ::std::string_literals;

            if (d)
                return c << *d;

            return c << "empty_optional"s;
        }

template<typename T>
MD5Coder &
    operator<<(
            MD5Coder & c
        ,   ::std::shared_ptr<T> const & d
        )
        {
            using namespace ::std::string_literals;

            if (d)
                return c << *d;

            return c << "empty_shared_pointer"s;
        }

template<typename T>
MD5Coder &
    operator<<(
            MD5Coder & c
        ,   ::std::unique_ptr<T> const & d
        )
        {
            using namespace ::std::string_literals;

            if (d)
                return c << *d;

            return c << "empty_unique_pointer"s;
        }

template<typename T>
MD5Coder &
    operator<<(
            MD5Coder & c
        ,   ::std::vector<T> const & d
        )
        {
            using namespace ::std::string_literals;

            if (d.empty())
                return c << "empty_vector"s;

            for (auto & e : d)
                c << e;

            return c;
        }

template<typename T>
MD5Coder &
    operator<<(
            MD5Coder & c
        ,   ::std::valarray<T> const & d
        )
        {
            using namespace ::std::string_literals;

            if (d.size()==0)
                return c << "empty_valarray"s;

            for (auto & e : d)
                c << e;

            return c;
        }

template<typename K, typename V>
MD5Coder &
    operator<<(
            MD5Coder & c
        ,   ::std::map<K,V> const & d
        )
        {
            using namespace ::std::string_literals;

            if (d.empty())
                return c << "empty_map"s;

            for (auto & p : d)
                c << p.first << p.second;

            return c;
        }

template<typename T>
MD5Coder &
    operator<<(
            MD5Coder & c
        ,   ::std::set<T> const & d
        )
        {
            using namespace ::std::string_literals;

            if (d.empty())
                return c << "empty_map"s;

            for (auto & e : d)
                c << e;

            return c;
        }

template<typename C, typename T>
MD5Coder &
    operator<<(
            MD5Coder & c
        ,   ::nsBase::ID<C,T> const & d
        )
        {
            return c << d.value();
        }

template<typename T>
MD5Coder &
    operator<<(
            MD5Coder & c
        ,   ::nsBase::Range<T> const & d
        )
        {
            return c << d.min() << d.max();
        }

}
