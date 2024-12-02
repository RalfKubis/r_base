/* Copyright (C) Ralf Kubis */

#include "r_base/exit.h"
#include "r_base/Log.h"

#include <cstdlib>


namespace nsBase
{

[[noreturn]]
void
exit(
    ::uuids::uuid const & trace_id
,   int                   exit_code
)
{
    {
        "8a1fb351-f6d3-480f-959d-69b252038b25"_log
            ("exiting")
            .event("5b134ea3-bc81-4275-8322-6ff914156f78"_uuid)
            .trace(trace_id)
            ;
    }

    ::std::_Exit(exit_code);
}

}
