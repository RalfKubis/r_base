#pragma once
/* Copyright (C) Ralf Kubis */

#include "r_base/language_tools.h"

#include <functional>
#include <vector>
#include <string>
#include <fstream>

#include "r_base/filesystem.h"
#include "r_base/Error.h"
#include "r_base/expected.h"

namespace nsBase
{
/**
    Call the target functor for each subdirectory inside the target folder.

    \param  inDir       The target folder.
    \param  inAction    The target functor.
*/
void
    foreachSubdirectory(
            ::fs::path                          const & inDir
        ,   ::std::function<void(::fs::path const &)>   inAction
        );

/**
    Call the target functor for each plain file inside the target folder.

    \param  inDir       The target folder.
    \param  inAction    The target functor.
*/
void
    foreachFileInDir(
            ::fs::path                          const & inDir
        ,   ::std::function<void(::fs::path const &)>   inAction
        );


// throws
::std::string
    file_read_all(
            ::fs::path const & file_path
        );



// throws
void
    file_write_all(
            ::fs::path        const & file_path
        ,   ::std::string     const & content
        ,   ::std::ios_base::openmode mode = ::std::ios::binary | ::std::ios::trunc
        );

}
