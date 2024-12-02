#pragma once
/* Copyright (C) Ralf Kubis */

#include "r_base/Log.h"

#include <string>


namespace nsBase
{

struct Status
{
    Log::Status     code {};
    ::std::string   message;

    operator bool() const
        {
            return code == Log::Status::OK;
        }
};

}
