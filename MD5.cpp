/* Copyright (C) Ralf Kubis */

#include "r_base/MD5.h"
#include "r_base/MD5Coder.h"
#include "r_base/language_tools.h"
#include "r_base/Log.h"
#include "r_base/string.h"

#include <memory>
#include <fstream>


namespace nsBase
{

::std::string
to_string(
    MD5 const & md5
)
{
    ::std::string
        hex;

    dataToHexString(
            reinterpret_cast<unsigned char const *>(md5.m8) // inBufferToEncode
        ,   16                                          // inBufferToEncodeLen
        ,   hex                                         // outEncodedBuffer
        ,   true                                        // inUseUpperCase
        );

    DBC_POST(hex.length()==32)

    return hex;
}


MD5::MD5()
{
    clear();
}


MD5::MD5(
    ::std::string const & str
)
{
    if (DBC_FAIL(str.empty() || str.size()==32))
        throw Error(Log(u8"c7ee1650-7b9c-4cb0-9183-65ffbb703412"_uuid));

    clear();

    if (str.length()==32)
    {
        hexStringToData(
                m8 // inBufferToDecodeTo
            ,   str // inEncodedBuffer
            );
    }
}


MD5
hash_from_file(
    ::fs::path const & file_path
)
{
    Status
        retVal;

    MD5
        md5;

    MD5Coder        coder;
    ::std::string     buf;
    size_t const    bufLen = 1024 * 1024 * 10;

    BLOCK
    {
        // open file
        ::std::ifstream
            file(
                    file_path
                ,   ::std::ios::binary
                );

        if (!CHECK2(
                    "0298c09f-ba55-4cce-9a4e-84b533d2b6db"_uuid
                ,   file.is_open()
                ,   file_path.u8string()
                )
        )
            FailBreak(u8"0b04c80e-3425-4c41-9764-a5ce2d0f2739"_uuid);

        // alloc buffer
        buf.resize(bufLen);

        // file: read data and compute md5
        while(true)
        {
            try
            {
                file.read(
                        &buf[0]
                     ,  bufLen
                     );
            }
            catch(...)
            {
                FailBreak("1041c781-188b-4ef3-a410-81decd8ad56d"_uuid);
            }

            auto
                numBytesRead = file.gcount();

            DBC_ASSERT(numBytesRead<=bufLen);

            // -1 is an error
            if (!CHECK("5ea22fa9-97bb-4bb8-a06c-918cfee66bbe"_uuid, numBytesRead>=0))
                FailBreak2("2254ea8e-67d0-4670-a087-50b6e1c1800b"_uuid, retVal);

            // more data read -> update md5
            if (numBytesRead>0)
            {
                coder.merge(
                        (unsigned char*)(&buf[0])
                    ,   numBytesRead
                    );
            }

            // eof reached -> finish md5
            if (file.eof())
            {
                md5 = coder.query();
                break;
            }
        }

        BreakOnFail;
    }
    FIN

    if (!retVal)
        md5.clear();

    return md5;
}


MD5
hash_from_buffer(
    void         const * inBuffer
,   ::std::size_t  const   inBufferLen
)
{
    DBC_PRE(inBufferLen==0 || inBuffer)
    DBC_PRE(inBufferLen>=0 )

    MD5Coder
        coder;

    coder.merge(
            inBuffer
        ,   inBufferLen
        );

    return coder.query();
}


}
