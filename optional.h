#pragma once
/* Copyright (C) Ralf Kubis */

#include <optional>


namespace nsBase
{

template<typename T>
::std::optional<T>
    to_optional(
            T const * p
        )
        {
            if (!p) return {};
            return {*p}; // copy
        }
        
}