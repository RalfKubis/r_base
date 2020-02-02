#pragma once
/* Copyright (C) Ralf Kubis */

#include "r_base/filesystem.h"

#include "r_base/decl.h"
#include "r_base/MD5.h"
#include "r_base/file.h"
#include "r_base/language_tools.h"
#include "r_base/expected.h"
#include "r_base/Error.h"


namespace nsBase
{

class FileStore
{
    R_DTOR(FileStore) = default;
    R_CTOR(FileStore) = delete;
    R_CCPY(FileStore) = delete;
    R_CMOV(FileStore) = delete;
    R_COPY(FileStore) = delete;
    R_MOVE(FileStore) = delete;

    public :
        FileStore(
                ::fs::path base_dir_path
            );

    R_PROPERTY_(
            base_dir_path
        ,   ::fs::path
        )

    public : ::tl::expected<::fs::path, Error>
        container_dir_path(
                MD5 const & buffer_id
            );

    public : ::tl::expected<::fs::path, Error>
        file_path(
                MD5 const & buffer_id
            );

    public : MD5 // throws
        store(
                ::std::string const & data
            );

    public : ::tl::expected<::std::string, Error>
        read(
                MD5 const & buffer_id
            );

    public : ::tl::expected<bool, Error>
        exists(
                MD5 const & buffer_id
            );
};

}
