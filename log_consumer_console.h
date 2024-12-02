#include "r_base/Log.h"

#include <optional>


namespace nsBase
{

/** requirement.6125bf04-81b8-4ab2-b681-b9e99e752a29
*/
extern bool
    log_consumer_console_be_verbose;

/** requirement.f71e96cb-7495-437b-823b-1dd47b6c7a01
*/
extern ::std::optional<bool>
    log_consumer_console_dump_in_local_time;

/** requirement.7ac901b5-a504-4be2-b207-d1a6940fc723
*/
extern ::std::optional<bool>
    log_consumer_console_dump_date;

/** requirement.38d37bf9-4145-4398-b49f-6a738d6c21b7
*/
void
    log_consumer_console(
            Log & log
        );

}
