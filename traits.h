#pragma once
/* Copyright (C) Ralf Kubis */

#include <type_traits>
#include <set>
#include <optional>
#include <vector>


  ////////////////////////////////////////////////////////////////////////////////
 // SFINAE helper
//
namespace nsBase
{
template<typename T, typename Enable = void>
struct is_an_optional : ::std::false_type {};

template<typename T>
struct is_an_optional<::std::optional<T>> : ::std::true_type {};


// optional
template<typename T> using enable_if_is_an_optional     = typename ::std::enable_if<  is_an_optional<T>::value >::type;
template<typename T> using enable_if_is_not_an_optional = typename ::std::enable_if< !is_an_optional<T>::value >::type;


template<
    typename T
,   ::nsBase::enable_if_is_an_optional<T> * SFINAE1 = nullptr
>
bool
    is_optional()
        {
            return true;
        }

template<
    typename T
,   ::nsBase::enable_if_is_not_an_optional<T> * SFINAE1 = nullptr
>
bool
    is_optional()
        {
            return false;
        }


template<typename T, typename Enable = void>
struct is_a_vector : ::std::false_type {};

template<typename T>
struct is_a_vector<::std::vector<T>> : ::std::true_type {};


// vector
template<typename T> using enable_if_is_a_vector     = typename ::std::enable_if<  is_a_vector<T>::value >::type;
template<typename T> using enable_if_is_not_a_vector = typename ::std::enable_if< !is_a_vector<T>::value >::type;



template<typename T, typename Enable = void>
struct is_a_set : ::std::false_type {};

template<typename T>
struct is_a_set<::std::set<T>> : ::std::true_type {};


// set
template<typename T> using enable_if_is_a_set     = typename ::std::enable_if<  is_a_set<T>::value >::type;
template<typename T> using enable_if_is_not_a_set = typename ::std::enable_if< !is_a_set<T>::value >::type;


// reference
template<typename T> using enable_if_is_reference_t    = ::std::enable_if_t<  ::std::is_reference_v<T> >;
template<typename T> using enable_if_is_no_reference_t = ::std::enable_if_t< !::std::is_reference_v<T> >;
}
