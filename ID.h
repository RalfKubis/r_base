#pragma once
/* Copyright (C) Ralf Kubis */

#include "r_base/decl.h"
#include "r_base/dbc.h"
#include "r_base/language_tools.h"

#include <functional>
#include <cstdint>


namespace nsBase
{

template<class Object, typename Value>
class ID
{
    R_DTOR(ID) = default;
    R_CTOR(ID) = delete; // There is no empty instance.
    R_CCPY(ID) = default;
    R_CMOV(ID) = default;
    R_COPY(ID) = default;
    R_MOVE(ID) = default;

    public : using
        value_type = Value;

    public : explicit
        ID(
                value_type const & v
            )
            :   m_value( v )
            {
            }

    /** The internal representation.
    */
    protected : value_type
        m_value;

    public : void
        value_assign(
                value_type const & new_value
            )
            {
                m_value = new_value;
            }

    public : value_type const &
        value() const
            {
                return m_value;
            }

    /** Comparison.
    */
    public : bool
        operator==(
                ID const & other
            ) const
            {
                return m_value == other.m_value;
            }

    /** Comparison.
    */
    public : bool
        operator!=(
                ID const & other
            ) const
            {
                return !operator==(other);
            }

    /** Some default order without semantics.
        Can be changed by overriding.
    */
    public : bool
        operator<(
                ID const & other
            ) const
            {
                return value() < other.value();
            }

    /** Some default order without semantics.
        Can be changed by overriding.
    */
    public : bool
        operator>(
                ID const & other
            ) const
            {
                return value() > other.value();
            }

    /** Some default order without semantics.
        Can be changed by overriding.
    */
    public : bool
        operator<=(
                ID const & other
            ) const
            {
                return  operator<(other)
                    ||  operator==(other)
                    ;
            }

    /** Some default order without semantics.
        Can be changed by overriding.
    */
    public : bool
        operator>=(
                ID const & other
            ) const
            {
                return  operator>(other)
                    ||  operator==(other)
                    ;
            }
};




#define DECLARE_ID_TYPE(Name,ValueType)                                        \
                                                                               \
class Name                                                                     \
{                                                                              \
    public : using                                                             \
        value_type = ValueType;                                                \
                                                                               \
    /** The internal representation.                                           \
    */                                                                         \
    protected : value_type                                                     \
        m_value;                                                               \
                                                                               \
    public : void                                                              \
        value_assign(                                                          \
                value_type const & new_value                                   \
            )                                                                  \
            {                                                                  \
                m_value = new_value;                                           \
            }                                                                  \
                                                                               \
    public : value_type const &                                                \
        value() const                                                          \
            {                                                                  \
                return m_value;                                                \
            }                                                                  \
                                                                               \
    /** There is no empty instance.                                            \
    */                                                                         \
    public :                                                                   \
        Name() = delete;                                                       \
                                                                               \
    public :                                                                   \
        ~Name() = default;                                                     \
                                                                               \
    public :                                                                   \
        Name(                                                                  \
                Name const & other                                             \
            )                                                                  \
            :   m_value( other.m_value )                                       \
            {                                                                  \
            }                                                                  \
                                                                               \
    public : explicit                                                          \
        Name(                                                                  \
                value_type const & v                                           \
            )                                                                  \
            :   m_value( v )                                                   \
            {                                                                  \
            }                                                                  \
                                                                               \
                                                                               \
    /** Comparison.                                                            \
    */                                                                         \
    public : bool                                                              \
        operator==(                                                            \
                Name const & other                                             \
            ) const                                                            \
            {                                                                  \
                return m_value == other.m_value;                               \
            }                                                                  \
                                                                               \
    /** Comparison.                                                            \
    */                                                                         \
    public : bool                                                              \
        operator!=(                                                            \
                Name const & other                                             \
            ) const                                                            \
            {                                                                  \
                return !operator==(other);                                     \
            }                                                                  \
                                                                               \
    /** Some default order without semantics.                                  \
        Can be changed by overriding.                                          \
    */                                                                         \
    public : bool                                                              \
        operator<(                                                             \
                Name const & other                                             \
            ) const                                                            \
            {                                                                  \
                return value() < other.value();                                \
            }                                                                  \
                                                                               \
    /** Some default order without semantics.                                  \
        Can be changed by overriding.                                          \
    */                                                                         \
    public : bool                                                              \
        operator>(                                                             \
                Name const & other                                             \
            ) const                                                            \
            {                                                                  \
                return value() > other.value();                                \
            }                                                                  \
                                                                               \
    /** Some default order without semantics.                                  \
        Can be changed by overriding.                                          \
    */                                                                         \
    public : bool                                                              \
        operator<=(                                                            \
                Name const & other                                             \
            ) const                                                            \
            {                                                                  \
                return  operator<(other)                                       \
                    ||  operator==(other)                                      \
                    ;                                                          \
            }                                                                  \
                                                                               \
    /** Some default order without semantics.                                  \
        Can be changed by overriding.                                          \
    */                                                                         \
    public : bool                                                              \
        operator>=(                                                            \
                Name const & other                                             \
            ) const                                                            \
            {                                                                  \
                return  operator>(other)                                       \
                    ||  operator==(other)                                      \
                    ;                                                          \
            }                                                                  \
};


// NOTE: template instantiations are performed in the .cpp file

}
