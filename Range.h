#pragma once
/* Copyright (C) Ralf Kubis */

#include "r_base/decl.h"
#include "r_base/language_tools.h"

#include <optional>
#include <algorithm>


namespace nsBase
{

template<class T>
class Range
{
    R_DTOR(Range) = default;
    R_CTOR(Range) = default;
    R_CCPY(Range) = default;
    R_CMOV(Range) = default;
    R_COPY(Range) = default;
    R_MOVE(Range) = default;

    public :
        Range(
                ::std::optional<T> min
            ,   ::std::optional<T> max
            );

    public : template<typename FROM>
        Range(
                Range<FROM> const & rhs
            )
            {
                if (rhs.min()) min_assign(*rhs.min());
                if (rhs.max()) max_assign(*rhs.max());
            }

    public : template<typename FROM>
        Range &
            operator=(
                    Range<FROM> const & rhs
                )
                {
                    if ((void*)(&rhs) != (void*)this)
                    {
                        clear();
                        if (rhs.min()) min_assign(*rhs.min());
                        if (rhs.max()) max_assign(*rhs.max());
                    }

                    return *this;
                }

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

    public : auto operator<=>(Range const &) const = default;

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

    public : void
        make_finite_if_and_capture(T const &);

    public : void
        clear();
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
                *m_min && rhs.contains(**m_min)
        ||      *m_max && rhs.contains(**m_max)
        ||  *rhs.m_min &&     contains(**rhs.m_min)
        ||  *rhs.m_max &&     contains(**rhs.m_max)
        ;
}


template<class T>
inline void
Range<T>::make_finite_if_and_capture(
    T const & x
)
{
    *m_min = ::std::min(m_min->value_or(x), x);
    *m_max = ::std::max(m_max->value_or(x), x);
}


template<class T>
inline void
Range<T>::clear()
{
    max_clear();
    min_clear();
}

}
