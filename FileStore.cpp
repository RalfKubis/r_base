#include "r_base/FileStore.h"
#include "r_base/dbc.h"
#include "r_base/Log.h"
#include "r_base/file.h"

namespace nsBase
{
using namespace ::std::string_literals;


FileStore::FileStore(
    ::fs::path base_dir_path_
)
:   m_base_dir_path(::std::move(base_dir_path_))
{
}


::tl::expected<::fs::path, Error>
FileStore::container_dir_path(
    MD5 const & buffer_id
)
{
    try
    {
        ::fs::path
            path;

        ::std::error_code
            ec;

        // convert to string (upper case)
        auto
            md5_str = to_string(buffer_id);

        auto
            dir_curr = absolute(base_dir_path());

        if (!::fs::exists(dir_curr))
            create_directory(dir_curr);

        if (DBC_FAIL(::fs::exists(dir_curr)))
            "c7e0cba7-070d-4591-8c16-71d18c4f353d"_log().throw_error();

        for (auto level=0_sz ; level<32 ;)
        {
            auto isLeaf = !::fs::exists(dir_curr / "0");

            // descent
            if (!isLeaf)
            {
                auto
                    dir_next = dir_curr / ::std::string(1,md5_str[level]);

                if (!::fs::exists(dir_next))
                    create_directory(dir_next);

                DBC_ASSERT(::fs::exists(dir_next));

                dir_curr = dir_next;
                level++;
                continue;
            }

            // leaf node reached

            if (::fs::exists(dir_curr/md5_str))
            {
                path = dir_curr;
                break;
            }

            // file does not exist -> expand
            ::std::vector<::fs::path>
                files;

            foreachFileInDir(
                    dir_curr
                ,   [&](::fs::path const & p)
                    {
                        if (is_regular_file(p))
                            files.emplace_back(p.filename());
                    }
                );

            if (files.size()<37)
            {
                path = dir_curr;
                break;
            }

            // too much files -> try to expand

            // move * -> temp/*
            auto
                dir_tmp = dir_curr / "_";
                create_directory(dir_tmp);

            try
            {
                for (auto & file : files)
                {
                    // move
                    rename(
                            dir_curr / file
                        ,   dir_tmp  / file
                        );
                }
            }
            catch(...)
            {
                for (auto & file : files)
                {
                    // move back
                    rename(
                            dir_tmp  / file
                        ,   dir_curr / file
                        ,   ec
                        );
                }

                remove(dir_tmp,ec);

                if (DBC_FAIL(!ec))
                    "a27a0ece-826f-40ca-8966-8244cafbef1c"_log().throw_error();

                path = dir_curr;
                break;
            }


            // move temp/* -> ?/*
            create_directory(dir_curr / "0");

            for (auto & file : files)
            {
                auto
                    file_str = P2S(file);

                auto
                    dir_next = dir_curr / ::std::string(1,file_str[level]);

                if (!::fs::exists(dir_next))
                    create_directory(dir_next,ec);

                if (DBC_FAIL(::fs::exists(dir_next)))
                    "74e0cc77-fa34-4e18-81e8-7644dd4248ac"_log().throw_error();

                // move to subdir
                rename(
                        dir_tmp  / file
                    ,   dir_next / file
                    );
            }
            remove(dir_tmp,ec);

            if (DBC_FAIL(!ec))
                "f1bbd87f-7258-4839-bbde-7c5d048780ad"_log().throw_error();
        }

        if (DBC_FAIL(path.empty() || ::fs::is_directory(path)))
            "970b1a69-1f33-46fe-bf5e-35dde05e89c2"_log().throw_error();

        return path;
    }
    catch(...)
    {
    }

    return ::tl::unexpected<Error>(::std::move("2d6900dd-d65c-4cca-aee8-c45afe953904"_log("failed to obtain folder for buffer '${data}'"s).data(to_string(buffer_id))));
}


tl::expected<::fs::path, Error>
FileStore::file_path(
    MD5 const & buffer_id

)
{
    auto exp = container_dir_path(buffer_id);

    if (!exp)
        return exp;

    *exp /= S2P(to_string(buffer_id));

    return exp;
}


MD5
FileStore::store(
    ::std::string const & data
)
{
    auto
        hash = ::nsBase::hash_from_buffer(data.data(), data.size());
    auto
        path = file_path(hash);

    if (!path)
        throw path.error();

    if (!fs::exists(*path))
    {
        TODO(locking)

        file_write_all(*path, data);
    }

    return hash;
}


tl::expected<::std::string, Error>
FileStore::read(
    MD5 const & buffer_id
)
{
    try
    {
        auto
            dir_path = container_dir_path(buffer_id);

        if (!dir_path)
            return ::tl::unexpected<Error>(::std::move(dir_path.error()));

        auto
            path = *dir_path / to_string(buffer_id);

        if (!::fs::exists(path))
            return ::tl::unexpected<Error>("92005a4a-8ac7-4e54-8668-f2d5e0082d83"_log("buffer file does not exist '${data}'"s).data(P2S(path)).move());

TODO(locking)

        return file_read_all(path);
    }
    catch(...)
    {
    }

    return ::tl::unexpected<Error>("b686c891-a424-4472-8a46-266bc0987d8c"_log("failed read to buffer '${data}'"s).data(to_string(buffer_id)).move());
}


::tl::expected<bool, Error>
FileStore::exists(
    MD5 const & buffer_id
)
{
    try
    {
        auto
            dir_path = container_dir_path(buffer_id);

        if (!dir_path)
            return ::tl::unexpected<Error>(::std::move(dir_path.error()));

        auto
            path = *dir_path / to_string(buffer_id);

        return ::fs::is_regular_file(path);
    }
    catch(...)
    {
    }

    return ::tl::unexpected<Error>("560e22a6-0fb3-4e7f-ae85-2689685f597d"_log("failed test to buffer for existence '${data}'"s).data(to_string(buffer_id)).move());
}

}
