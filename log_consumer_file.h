#include "r_base/Log.h"

#include "r_base/filesystem.h"


namespace nsBase
{

void
    log_consumer_file_path_assign(
            ::fs::path const &
        );

::fs::path
    log_consumer_file_path();


void
    log_consumer_file(
            Log & log
        );

}
