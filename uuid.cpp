#include "r_base/uuid.h"
#include "r_base/dbc.h"



// KU
auto
    operator "" _uuid(
            char        const * data
        ,   ::std::size_t         len
        )
        ->  uuids::uuid
        {
            auto
                u = uuids::uuid(data);

            DBC_POST((len==0)==u.is_nil());

            return u;
        }
