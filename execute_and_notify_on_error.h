#pragma once

#include <optional>
#include <functional>

#include "r_base/expected.h"
#include "r_base/uuid.h"
#include "r_base/Error.h"


namespace nsBase
{

inline ::tl::expected<void, ::nsBase::Error>
    execute_and_notify_on_error(
            ::uuids::uuid           trace_id
        ,   ::std::function<void()> func
        )
        {
            using namespace ::nsBase;
            using namespace ::std::string_literals;

            ::std::optional<Error>
                err;

            try
            {
                if (func)
                    func();
            }
            catch(::std::exception & e)
            {
                err = to_Error(e, trace_id);
            }
            catch(::nsBase::Log & l)
            {
                err = Error(l.move());
            }
            catch(...)
            {
                err = Error{Log{trace_id}("unspecified error"s).move()};
            }

            if (err)
                return ::tl::make_unexpected<Error>(::std::move(*err));

            return {};
        }
}
