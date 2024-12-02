/* Copyright (C) Ralf Kubis */

#include "r_base/file.h"
#include "r_base/Log.h"
#include "r_base/Error.h"
#include "r_base/expected.h"

#include <fstream>


namespace nsBase
{
static auto log_scope = "nsBase.file";

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
        try
        {
            if (!::fs::is_directory(entry.status()))
                continue;
        }
        catch(...)
        {
            continue;
        }

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
        c_file_size_max_accepted = 5'000'000'000_sz;

    if (!::fs::is_regular_file(path))
        "f4a3b3e5-0b4c-4993-b12c-6528e9000ec1"_log[log_scope]
        ("this is not a regular file '${path}'")
        .path(path)
        .throw_error()
        ;

    ::std::ifstream
        stream {path, ::std::ios::in | ::std::ios::binary | ::std::ios::ate};

    if (!stream)
        "e5c20901-f77e-4c5a-bdaa-cdd36fbe2828"_log[log_scope]
        ("unable to open file '${path}'")
        .path(path)
        .throw_error()
        ;

    ::std::size_t
        size = stream.tellg();

    if (size > c_file_size_max_accepted)
        "0436ac75-9bed-4dc0-834f-ed4e1b618246"_log[log_scope]
        ("file '${path}' is too large - accepting '${count}' bytes or less but got '${size}'")
        .path(path)
        .count(c_file_size_max_accepted)
        ("size", size)
        .throw_error()
        ;

    ::std::string
        buffer(size,'\0');

    stream.seekg(0);

    if (!stream.read(&buffer[0], size))
        "7dc99c8c-d2e0-4286-aca4-9a82ac51f32d"_log[log_scope]
        ("unable to read file '${path}'")
        .path(path)
        .throw_error()
        ;

    return buffer;
}


::std::basic_string<::std::uint8_t>
file_read_all_bytes(
    ::fs::path const & path
)
{
    auto const
        c_file_size_max_accepted = 5'000'000'000_sz;

    if (!::fs::is_regular_file(path))
        "23257cd9-db55-4082-bc4e-acf91a211f29"_log[log_scope]
        ("this is not a regular file '${path}'")
        .path(path)
        .throw_error()
        ;

    ::std::ifstream
        stream {path, ::std::ios::in | ::std::ios::binary | ::std::ios::ate};

    if (!stream)
        "5b429237-3602-4abe-a0b4-f7ef3fed7bbe"_log[log_scope]
        ("unable to open file '${path}'")
        .path(path)
        .throw_error()
        ;

    ::std::size_t
        size = stream.tellg();

    if (!size)
        return {};

    if (size > c_file_size_max_accepted)
        "63177b7a-a5d6-4940-a8c3-e9c37132a7e5"_log[log_scope]
        ("file '${path}' is too large - accepting '${count}' bytes or less but got '${size}'")
        .path(path)
        .count(c_file_size_max_accepted)
        ("size", size)
        .throw_error()
        ;

    ::std::basic_string<::std::uint8_t>
        buffer(size, 0);

    stream.seekg(0);

    if (!stream.read(reinterpret_cast<char*>(&buffer[0]), size))
        "9f003173-a71d-4f5f-b47d-543119a1e1db"_log[log_scope]
        ("unable to read file '${path}'")
        .path(path)
        .throw_error()
        ;

    return buffer;
}



void
file_write_all(
    ::fs::path        const & file_path
,   ::std::string     const & content
,   ::std::ios_base::openmode mode
)
{
    ::std::ofstream
        stream(file_path, mode);
        stream.write(content.data(), content.size()); // throws
}

}
