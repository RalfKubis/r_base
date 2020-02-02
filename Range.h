#pragma once
/* Copyright (C) Ralf Kubis */


#include "r_base/decl.h"
#include <optional>
#include "r_base/Property.h"


namespace nsBase
{

template<class T>
class Range
{
    public :
        Range() = default;

    public :
        Range(
                ::std::optional<T> min
            ,   ::std::optional<T> max
            );


    public : bool
        operator==(
                Range const & rhs
            ) const;

    R_PROPERTY_(
            min
        ,   ::std::optional<T>
        )

    R_PROPERTY_(
            max
        ,   ::std::optional<T>
        )

    public : bool
        contains(
                T value
            ) const;

    public : bool
        is_finite() const;

    public : bool
        is_bounded() const;

    public : bool
        is_empty() const;

    public : bool
        is_intersecting(
                Range const & rhs
            ) const;
};


template<class T>
inline
Range<T>::Range(
    ::std::optional<T> min
,   ::std::optional<T> max
)
:   m_min(min)
,   m_max(max)
{
//allowed    DBC_PRE(ifThen(min && max, *min<=*max ));
}


template<class T>
inline bool
Range<T>::operator==(
    Range const & rhs
) const
{
    return
            *m_min == *rhs.m_min
        &&  *m_max == *rhs.m_max
        ;
}


template<class T>
inline bool
Range<T>::contains(
    T value
) const
{
    if (*m_min && value<**m_min)
        return false;

    if (*m_max && **m_max<value)
        return false;

    return true;
}


template<class T>
inline bool
Range<T>::is_finite() const
{
    return *m_min && *m_max;
}


template<class T>
inline bool
Range<T>::is_bounded() const
{
    return *m_min || *m_max;
}


template<class T>
inline bool
Range<T>::is_empty() const
{
    return
            is_finite()
        &&  **m_max < **m_min
        ;
}


template<class T>
inline bool
Range<T>::is_intersecting(
    Range const & rhs
) const
{
    // no intersection if one is empty
    if (is_empty() || rhs.is_empty())
        return false;

    // always intersecting if one is unbounded
    if (!is_bounded() || !rhs.is_bounded())
        return true;

    return
            *m_min     && rhs.contains(**m_min)
        ||  *m_max     && rhs.contains(**m_max)
        ||  *rhs.m_min && contains(**m_min)
        ||  *rhs.m_max && contains(**m_max)
        ;
}


}
