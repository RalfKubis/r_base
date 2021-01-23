#pragma once
/* Copyright (C) Ralf Kubis */

#include "r_base/uuid.h"

namespace nsBase
{

[[noreturn]]
void
    exit(
            ::uuids::uuid const & trace_id
        ,   int                   exit_code
        );

}
