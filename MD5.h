#pragma once
/* Copyright (C) Ralf Kubis */

#include "r_base/decl.h"

#include "r_base/language_tools.h"
#include "r_base/file.h"

#include <string>


namespace nsBase
{

class MD5
{
    R_DTOR(MD5) = default;
    R_CTOR(MD5);
    R_CCPY(MD5) = default;
    R_CMOV(MD5) = default;
    R_COPY(MD5) = default;
    R_MOVE(MD5) = default;


    public :
        MD5(
                ::std::string const & str
            );

    public : union
            {
                uint64_t
                    m64[2];

                uint32_t
                    m32[4];

                uint16_t
                    m16[8];

                uint8_t
                    m8[16];
            };

    public : void
        clear()
            {
                m64[0] = 0;
                m64[1] = 0;
            }

    public : bool
        empty() const
            {
                return
                        m64[0]==0ll
                    &&  m64[1]==0ll
                    ;
            }

    public :
        explicit operator bool() const noexcept
            {
                return !empty();
            }

    public : bool
        operator==(
                MD5 const & inOther
            ) const
            {
                return
                        m64[0]==inOther.m64[0]
                    &&  m64[1]==inOther.m64[1]
                    ;
            }

    public : bool
        operator!=(
                MD5 const & inOther
            ) const
            {
                return !(*this==inOther);
            }

    public : bool
        operator<(
                MD5 const & inOther
            ) const
            {
                if (m64[0]==inOther.m64[0])
                    return m64[1]<inOther.m64[1];

                return m64[0]<inOther.m64[0];
            }
};


/**
    Get the 32 character long hex-string-representation of the source MD5
    (128-bit) with upper-case letters.
*/
std::string
    to_string(
            MD5 const & md5
        );

MD5
    hash_from_file(
            ::fs::path const & file_path
        );

MD5
    hash_from_buffer(
            void           const * inBuffer
        ,   ::std::size_t  const   inBufferLen
        );
}
