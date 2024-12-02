#pragma once
// Copyright (C) Ralf Kubis

#include "r_base/language_tools.h"
#include "r_base/uuid.h"
#include "r_base/file.h"
#include "r_base/Log.h"

#include <cstdio>


namespace nsBase
{

class SessionFileLogger
{
    R_DTOR(SessionFileLogger);
    R_CTOR(SessionFileLogger) = default;
    R_CCPY(SessionFileLogger) = delete;
    R_CMOV(SessionFileLogger);
    R_COPY(SessionFileLogger) = delete;
    R_MOVE(SessionFileLogger);

    R_PROPERTY_(
            session
        ,   ::uuids::uuid
        );

    R_PROPERTY_(
            log_dir_path
        ,   ::fs::path
        );

    R_PROPERTY_(
            log_file_path
        ,   ::fs::path
        );

    R_PROPERTY_D(
            log_file
        ,   ::std::FILE *
        ,   nullptr
        );

    R_PROPERTY_(
            extension
        ,   ::std::string
        );

    R_PROPERTY_(
            time
        ,   ::std::string
        );

    public : void
        rename_if();

    public : void
        dispose();

    public : void
        operator()(::nsBase::Log &);
};

}
