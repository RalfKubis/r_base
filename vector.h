#pragma once
/* Copyright (C) Ralf Kubis */

#include "r_base/decl.h"
#include "r_base/dbc.h"

#include <vector>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <iterator>
#include <optional>


namespace nsBase
{
/**
    Query if the target index is a valid argument of the target containers index operator.

    \tparam C   Type of the target container.
    \tparam I   Type of the target index.
    \param  inIndex     The target index.
    \param  inContainer The target container.

    \return
    TRUE if the target index is valid, FALSE if not.
*/
template<
    typename C
,   typename I
>
bool
isValidIndex(
    I const & inIndex
,   C const & inContainer
)
{
    return  inIndex >= 0
        &&  inIndex < (I)inContainer.size()
        ;
}


/**
    Query if the target container contains the target value.

    \tparam C   Type of the target container.
    \tparam V   Type of the target value.
    \param  inContainer The target container.
    \param  inValue     The target value.

    \return
    TRUE if the target container contains the target value, FALSE if not.
*/
template<
    typename C
,   typename V
>
bool
contains(
    C const & inContainer
,   V const & inValue
)
{
    return ::std::find(inContainer.begin(),inContainer.end(),inValue) != inContainer.end();
//    return inContainer.find( inContainer.begin(),inContainer.end(), inValue) != inContainer.end();
}


/**
    Query if the target container contains the target key.

    \tparam C   Type of the target container.
    \tparam V   Type of the target key.
    \param  inContainer The target container.
    \param  inValue     The target key.

    \return
    TRUE if the target container contains the target value, FALSE if not.
*/
template<
    typename C
,   typename V
>
bool
map_contains(
    C const & inContainer
,   V const & inKey
)
{
    return inContainer.find(inKey) != inContainer.end();
}


/**
    .

    \tparam C   Type of the target container.
    \tparam V   Type of the target value.
    \param  inContainer The target container.
    \param  inValue     The target value.

    \return
    .
*/
template<
    typename C
,   typename V
>
int
index(
    C const & inContainer
,   V const & inValue
)
{
    int
        retVal = -1;

    auto
        it = ::std::find( inContainer.begin(), inContainer.end(), inValue );

    if ( it != inContainer.end() )
        retVal = ::std::distance(inContainer.begin(),it);

    DBC_POST( retVal==-1 || isValidIndex(retVal,inContainer) );

    return retVal;
}


/**
    Remove the i-th element from the target vector.

    \tparam C   Type of the target container.
    \tparam I   Type of the target index.
    \param  inContainer The target container.
    \param  inIndex     The target index.
*/
template<
    typename C
,   typename I
>
void
remove(
    C       & inoutContainer
,   I const & inIndex
)
{
    inoutContainer.erase( inoutContainer.begin() + inIndex );
}

/**
    Remove the first found element from the target collection that equals
    to the given key.

    \tparam C   Type of the target collection.
    \tparam I   Type of the key.
    \param  collection The target collection.
    \param  key The key.

    \return TRUE if the key was found, FALSE otherwise;
*/
template<
    typename C
,   typename I
>
bool
remove_by_key(
    C       & collection
,   I const & key
)
{
    auto
        it = ::std::find(
                collection.begin()
            ,   collection.end()
            ,   key
            );

    if (it==collection.end())
        return false;

    collection.erase(it);

    return true;
}


// from https://stackoverflow.com/questions/800955/remove-if-equivalent-for-stdmap
template<typename ContainerT, typename PredicateT>
void
erase_if(
    ContainerT       & container
,   PredicateT const & predicate
)
{
    for (auto it=container.begin() ; it!=container.end() ;)
    {
        if (predicate(*it))
            it = container.erase(it);
        else
            ++it;
    }
}

template<typename ContainerT, typename PredicateT>
::std::optional<typename ContainerT::value_type>
find_if(
    ContainerT       & container
,   PredicateT const & predicate
)
{
    if (auto it = ::std::find_if(container.begin(), container.end(), predicate); it!=container.end())
        return *it;
    return {};
}


#define r_value_type(container) ::std::remove_reference<decltype(container)>::type::value_type


template<typename ContainerT, typename PredicateT>
void
remove_if_and_erase(
    ContainerT       & container
,   PredicateT const & predicate
)
{
    container.erase(
            ::std::remove_if(
                    container.begin()
                ,   container.end()
                ,   predicate
                )
        ,   container.end()
        );
}


template<class ContainerTo, class ContainerFrom>
void
append(
    ContainerTo         & to
,   ContainerFrom const & from
)
{
    to.insert(::std::end(to), ::std::begin(from), ::std::end(from));
}


template<class ContainerTo, class ContainerFrom>
void
move(
    ContainerTo     & to
,   ContainerFrom  && from
)
{
    ::std::move(::std::begin(from), ::std::end(from), ::std::back_inserter(to));
}


/// insert an element into a sorted vector while keeping it sorted
template<typename T>
typename ::std::vector<T>::iterator
    insert_sorted(
            ::std::vector<T>  & vec
        ,   T           const & item
        )
        {
            return vec.insert(
                    ::std::upper_bound(vec.begin(), vec.end(), item)
                ,   item
                );
        }


/// insert an element into a sorted vector while keeping it sorted
template<typename T, typename Pred>
typename ::std::vector<T>::iterator
    insert_sorted(
            ::std::vector<T>  & vec
        ,   T           const & item
        ,   Pred                pred
        )
        {
            return vec.insert(
                    ::std::upper_bound(vec.begin(), vec.end(), item, pred)
                ,   item
                );
        }


// get the tail of a contiguous collection (path) if it has the given prefix, otherwise an empty optional
template<typename Collection>
::std::optional<Collection>
    tail_if(
            Collection const & prefix
        ,   Collection const & path
        )
        {
            auto it_prefix = prefix.begin();
            auto it_path   = path.begin();

            //TODO: use ::std::mismatch
            for(;;)
            {
                if (it_prefix==prefix.end())
                    break;

                if (it_path==path.end())
                    return {};

                if (*it_path!=*it_prefix)
                    return {};

                ++it_prefix;
                ++it_path;
            }

            Collection
                tail;

            ::std::for_each(
                    it_path
                ,   path.end()
                ,   [&](auto & s){tail.emplace_back(s);}
                );

            return tail;
        }
}
