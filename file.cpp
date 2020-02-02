/* Copyright (C) Ralf Kubis */

#include "r_base/file.h"
#include "r_base/Log.h"
#include "r_base/Error.h"
#include "r_base/expected.h"

#include <fstream>


namespace nsBase
{

void
foreachSubdirectory(
    ::fs::path                        const & dir
,   ::std::function<void(::fs::path const &)> action
)
{
    if (!fs::is_directory(dir))
        return;

    for (auto const & entry : ::fs::directory_iterator(dir))
    {
        if (::fs::is_directory(entry.status()))
            action(entry.path());
    }
}


void
foreachFileInDir(
    ::fs::path                          const & dir
,   ::std::function<void(::fs::path const &)>   action
)
{
    for (auto const & entry : ::fs::directory_iterator(dir))
    {
        auto
            status = entry.status();

        if (::fs::is_regular_file(status))
            action(entry.path());
    }
}


::std::string
file_read_all(
    ::fs::path const & path
)
{
    auto const
        c_file_size_max_accepted = 100000000_sz;

    ::std::ifstream
        stream(path.u8string(), ::std::ios::binary | ::std::ios::ate);

    if (!stream)
    {
        throw Error(Log(u8"e5c20901-f77e-4c5a-bdaa-cdd36fbe2828"_uuid)
            .error()
            .message("unable to open '${path}'")
            .path(path)
            );
    }

    ::std::size_t
        size = stream.tellg();

    if (size > c_file_size_max_accepted)
    {
        throw Error(Log(u8"0436ac75-9bed-4dc0-834f-ed4e1b618246"_uuid)
            .error()
            .message("file too large; accepting '${count}' bytes or less ")
            .count(c_file_size_max_accepted)
            );
    }

    ::std::string
        buffer(size,'\0');

    stream.seekg(0);

    if (!stream.read(&buffer[0], size))
    {
        throw Error(Log(u8"7dc99c8c-d2e0-4286-aca4-9a82ac51f32d"_uuid)
            .error()
            .message("unable to read '${path}'")
            .path(path)
            );
    }

    return buffer;
}


QByteArray
file_read_all_q(
    ::fs::path const & path
)
{
    auto const
        c_file_size_max_accepted = 100000000_sz;

    ::std::ifstream
        stream(path.u8string(), ::std::ios::binary | ::std::ios::ate);

    if (!stream)
    {
        throw Error(Log(u8"b3a53591-3a7a-4e56-989b-6fea0041a600"_uuid)
            .error()
            .message("unable to open '${path}'")
            .path(path)
            );
    }

    ::std::size_t
        size = stream.tellg();

    if (size > c_file_size_max_accepted)
    {
        throw Error(Log(u8"98fc32fd-0ab4-4ee1-a6f3-9d9c2efe7100"_uuid)
            .error()
            .message("file too large; accepting '${count}' bytes or less ")
            .count(c_file_size_max_accepted)
            );
    }

    QByteArray
        buffer(size,'\0');

    stream.seekg(0);

    if (!stream.read(buffer.data(), size))
    {
        throw Error(Log(u8"c00ea126-d26e-493d-8310-9d804c111cb0"_uuid)
            .error()
            .message("unable to read '${path}'")
            .path(path)
            );
    }

    return buffer;
}


void
file_write_all(
    ::fs::path    const & file_path
,   ::std::string const & content
)
{
    ::std::ofstream
        stream(file_path, ::std::ios::binary | ::std::ios::trunc);
        stream.write(content.data(), content.size()); // throws
}


void
file_write_all(
    ::fs::path    const & file_path
,   QByteArray    const & content
)
{
    ::std::ofstream
        stream(file_path, ::std::ios::binary | ::std::ios::trunc);
        stream.write(content.data(), content.size()); // throws
}

}
