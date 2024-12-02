#include "r_base/uuid.h"

#include "r_base/Log.h"


namespace nsBase::uuids
{

::std::optional<::uuids::uuid>
    from_string(
            ::std::string_view const & s
        )
        {
            if (s.empty())
                return {};

            auto u = ::uuids::uuid::from_string(s);

            if (!u)
                "96c376a8-23d3-467c-ab90-f61c62602ea3"_log().throw_error();

            return u;
        }

::std::optional<::uuids::uuid>
    from_string_with_empty_and_NIL_to_nullopt(
            ::std::string_view const & s
        )
        {
            auto o = from_string(s);

            if (o && o->is_nil())
                o = {};

            return o;
        }

::uuids::uuid
    from_string_with_empty_to_NIL(
            ::std::string_view const & s
        )
        {
            return from_string(s).value_or(::uuids::uuid{});
        }
}


namespace uuids
{

::std::string
    to_string_with_empty_to_NIL(
            ::std::optional<::uuids::uuid> const & u
        )
        {
            return to_string(u.value_or(::uuids::uuid{}));
        }

::std::string
    to_string8(
            ::uuids::uuid const & u
        )
        {
            return to_string(u).substr(0,8);
        }

::std::string
    to_string8_with_empty_to_NIL(
            ::std::optional<::uuids::uuid> const & u
        )
        {
            return to_string8(u.value_or(::uuids::uuid{}));
        }
}
