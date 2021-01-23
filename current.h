#pragma once
/* Copyright (C) Ralf Kubis */

#include "r_base/uuid.h"
#include "r_base/time.h"

#include <string>


namespace nsBase{namespace current
{

void
    application_id_assign(
            ::uuids::uuid const & id
        );

::uuids::uuid
    application_id();


void
    thread_session_id_assign(
            ::uuids::uuid const & id
        );

::uuids::uuid
    thread_session_id();


::std::string
    thread();

::nsBase::time::time_point_t
    time();

::std::string
    user();

::std::string
    host();

}}
