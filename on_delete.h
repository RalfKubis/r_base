#pragma once
/* Copyright (C) Ralf Kubis */

#include <functional>
#include <vector>

#include "r_base/language_tools.h"


namespace nsBase
{

/** This class is a RAII helper.
    It can store a functor which gets called by the objects destructor.
    It implements the dispose-pattern.
*/
class on_delete
{
    public : using
        func_t = ::std::function<void()>;

    R_PROPERTY_(
            func
        ,   func_t
        )


    R_DTOR(on_delete){dispose();}
    R_CTOR(on_delete) = default;
    R_CCPY(on_delete) = delete;
    R_CMOV(on_delete){operator=(::std::move(src));}
    R_COPY(on_delete) = delete;
    R_MOVE(on_delete){if (this!=&src)::std::swap(m_func, src.m_func); return *this;}

    public :
        on_delete(
                func_t && f
            )
            :   m_func{::std::move(f)}
            {
            }

    public : void
        release()
            {
                if (func())
                    func_clear();
            }

    public : void
        dispose()
            {
                if (func())
                    func()();

                release();
            }

    /** Test whether the instance is un-disposed.
    */
    public :
        operator bool() const
            {
                return bool(func());
            }
};

using on_deletes_t = ::std::vector<on_delete>;

}
