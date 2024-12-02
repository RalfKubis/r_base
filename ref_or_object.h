#pragma once

#include "r_base/Log.h"

#include <variant>
#include <optional>
#include <functional>
#include <utility>


namespace nsBase
{

template<typename T>
class
    ref_or_object
        {
            public :
                ref_or_object(T & x)
                    :   value(::std::in_place_index<0>, ::std::ref(x))
                    {
                    }

            public : template <class... CTorArgTypes>
                ref_or_object(
                        ::std::in_place_t
                    ,   CTorArgTypes &&... construction_args
                    )
                    :   value(::std::in_place_index<1>, ::std::forward<CTorArgTypes>(construction_args)...)
                    {
                    }


            private : using
                value_t = ::std::variant<::std::reference_wrapper<T>, T>;

            private : value_t
                value;

            public : bool
                is_reference() const
                    {
                        return value.index()==0;
                    }

            public : T &
                get()
                    {
                        switch (value.index())
                            {
                            case 0 : return ::std::get<0>(value);
                            case 1 : return ::std::get<1>(value);
                            }
                    }

            public : T const &
                get() const
                    {
                        switch (value.index())
                            {
                            case 0 : return ::std::get<0>(value);
                            case 1 : return ::std::get<1>(value);
                            }
                    }

            public :
                operator T & ()
                    {
                        return get();
                    }

            public :
                operator T const & () const
                    {
                        return get();
                    }
        };

}
