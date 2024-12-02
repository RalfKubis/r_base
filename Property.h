#pragma once
/* Copyright (C) Ralf Kubis */

#include "r_base/decl.h"
#include "r_base/traits.h"
#include "r_base/Log.h"

#include <type_traits>


namespace R_fallback
{
inline void
    hash_of_memory_state_clear()
        {
        }
}

namespace nsBase
{
    class Error;
}

////////////////////////////////////////////////////////////////////////////////
/**
    Properties (Class Members)

    It turned out that often standard set() and get() methods have to be programmed
    which require to type the same lines of code again and again.
    Class declarations get cluttered with trivial accessors.
    Refactoring is more expensive if, when properties get changed, the signature
    of many functions need to be changed instead of just one identifier.

    Similar to corresponding build-in features of other modern languages,
    these macros provide means to declare members (i.e. properties) of a class. Simple
    const-correct accessor functions are automatically added, reducing code duplication.
    The member variable itself is declared private, which fulfills the
    programming paradigm of encapsulation.
    Optionally, accessor modifiers can be specified.

    The initial or default value of the member can be specified.
    Internally an inner class is instantiated that encloses the member variable,
    setting it to the default value in its constructor. I.e. there is no longer the
    need to manually mention this member in initialisation lists.
    By this it also becomes very clear in the signature what the default value
    of a certain member is. A vital information that is often not taken
    care of in a properties documentation.

    In case non-trivial get() or set() methods are required, one can specify
    set- and get-hooks.

    The overall goal of using this macro is to type less code, reduce redundancy,
    save the programmers time, eliminate const mistakes, get better documentation
    and quickly get encapsulation even if coding happens under time pressure or
    if the code is still in the flow and might very likely get reworked in the
    next iteration.

    It appears that in the code base, direct accesses to members are very
    often left unsolved after task completion which can be avoided by using
    these macros in the first place.

    Since macros should better get avoided in favor or template meta programming
    its is a goal to refactor this implementation to met this requirement.
    If then the signature of this API changes, it should be a straightforward
    task to change the clients - if not even by auto-tooling.

    The macro identifier follows a name mangling scheme:


    R_PROPERTY_[A][D][G][L][Q][S]

    The optional modifier letters need to appear in lexical order, which
    should ease to build the identifier.

    A
        When used, the corresponding parameter is used as access modifier that
        is applied to the mutating methods (like _assign() and _clear()).

    D
        When used, the corresponding parameter specifies the default value of
        the property. If not used, the default initialisation is used.
        (or zero initialisation for POD types)

    G
        When used, the corresponding parameter specifies a user defined
        get-method.

    O
        When used, the property might have no value. Reading an empty property
        throws. The descriptor name_valid() is added. Setting the properties
        value makes it valid. The method name_clear() removes its value.

    Q
        When used, the corresponding parameter gets this value as type qualifier.
        This could be 'mutable' or 'const'.

    R
        When used, a reference is stored.

    S
        When used, the corresponding parameter specifies a user defined
        set-method.

    All macros have two mandatory parameters. The NAME of the property and
    its TYPE.
    Modifier letters might require more parameters.
    The order of the parameters corresponds to the order of the modifier letters
    in the macro identifier, which again should ease the use of the macros.


    R_PROPERTY_*(
            NAME        // required
        ,   TYPE        // required
       [,   ACCESSOR]   // when A is used
       [,   DEFAULT]    // when D is used
       [,   GETHOOK]    // when G is used
       [,   QUALIFIER]  // when Q is used
       [,   SETHOOK]    // when S is used
        )
*/
#define R_PROPERTY_FIELD_D(      NAME    ,TYPE  ,ACCESSOR    ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK  )     \
                                                                                \
    private : class property_##NAME                                             \
        {                                                                       \
            public : using                                                      \
                value_type = TYPE;                                              \
                                                                                \
            private : value_type                                                \
                mValue;                                                         \
                                                                                \
            public :                                                            \
                property_##NAME()                                               \
                    :   mValue(defaultValue())                                  \
                    {                                                           \
                    }                                                           \
                                                                                \
            public :                                                            \
                property_##NAME(                                                \
                        value_type const & inOther                              \
                    )                                                           \
                    :   mValue(inOther)                                         \
                    {                                                           \
                    }                                                           \
                                                                                \
            public : value_type &                                               \
                operator*()                                                     \
                    {                                                           \
                        return mValue;                                          \
                    }                                                           \
                                                                                \
            public : value_type const &                                         \
                operator*() const                                               \
                    {                                                           \
                        return mValue;                                          \
                    }                                                           \
                                                                                \
            public : value_type *                                               \
                operator->()                                                    \
                    {                                                           \
                        return &mValue;                                         \
                    }                                                           \
                                                                                \
            public : value_type const *                                         \
                operator->() const                                              \
                    {                                                           \
                        return &mValue;                                         \
                    }                                                           \
                                                                                \
            public : static value_type                                          \
                defaultValue()                                                  \
                    {                                                           \
                        return DEFAULT;                                         \
                    }                                                           \
        }                                                                       \
            QUALIFIER m_##NAME;                                                 \
        public : using                                                          \
            property_##NAME##_type = TYPE;                                      \
        private :


#define R_PROPERTY_FIELD(      NAME    ,TYPE  ,ACCESSOR    ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK  )     \
                                                                                \
    private : class property_##NAME                                             \
        {                                                                       \
            public : using                                                      \
                value_type = TYPE;                                              \
                                                                                \
            private : value_type                                                \
                mValue;                                                         \
                                                                                \
            public : template<class ... Constructor_args>                       \
                property_##NAME(Constructor_args && ... args)                   \
                    :   mValue(::std::forward<Constructor_args>(args)...)       \
                    {                                                           \
                    }                                                           \
                                                                                \
            public :                                                            \
                property_##NAME(                                                \
                        value_type const & inOther                              \
                    )                                                           \
                    :   mValue(inOther)                                         \
                    {                                                           \
                    }                                                           \
                                                                                \
            public : value_type &                                               \
                operator*()                                                     \
                    {                                                           \
                        return mValue;                                          \
                    }                                                           \
                                                                                \
            public : value_type const &                                         \
                operator*() const                                               \
                    {                                                           \
                        return mValue;                                          \
                    }                                                           \
                                                                                \
            public : value_type *                                               \
                operator->()                                                    \
                    {                                                           \
                        return &mValue;                                         \
                    }                                                           \
                                                                                \
            public : value_type const *                                         \
                operator->() const                                              \
                    {                                                           \
                        return &mValue;                                         \
                    }                                                           \
        }                                                                       \
            QUALIFIER m_##NAME;                                                 \
        public : using                                                          \
            property_##NAME##_type = TYPE;                                      \
        private :


#define R_PROPERTY_FIELD_O(      NAME    ,TYPE  ,ACCESSOR    ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK  )     \
                                                                                \
    private : class property_##NAME                                             \
        {                                                                       \
            public : using                                                      \
                value_type = TYPE;                                              \
                                                                                \
            private : value_type                                                \
                mValue;                                                         \
                                                                                \
            private : bool                                                      \
                mValid {};                                                      \
                                                                                \
            public : void                                                       \
                validate()                                                      \
                    {                                                           \
                        mValid = true;                                          \
                    }                                                           \
                                                                                \
            public :                                                            \
                property_##NAME()                                               \
                    {                                                           \
                    }                                                           \
                                                                                \
            public : template<class ... Constructor_args>                       \
                property_##NAME(Constructor_args && ... args)                   \
                    :   mValue(::std::forward<Constructor_args>(args)...)       \
                    ,   mValid{true}                                            \
                    {                                                           \
                    }                                                           \
                                                                                \
            public : void                                                       \
                assign(value_type const & v)                                    \
                    {                                                           \
                        mValue = v;                                             \
                        mValid = true;                                          \
                    }                                                           \
                                                                                \
            public : void                                                       \
                assign(value_type && v)                                         \
                    {                                                           \
                        mValue = ::std::move(v);                                \
                        mValid = true;                                          \
                    }                                                           \
                                                                                \
            public : template<class ... Constructor_args> void                  \
                emplace(Constructor_args && ... args)                           \
                    {                                                           \
                        mValue = value_type(::std::forward<Constructor_args>(args)...); \
                        mValid = true;                                          \
                    }                                                           \
                                                                                \
            public : void                                                       \
                clear()                                                         \
                    {                                                           \
                        mValue = value_type{};                                  \
                        mValid = {};                                            \
                    }                                                           \
                                                                                \
            public : bool                                                       \
                valid() const                                                   \
                    {                                                           \
                        return mValid;                                          \
                    }                                                           \
                                                                                \
            public : value_type &                                               \
                operator*()                                                     \
                    {                                                           \
                        using namespace ::nsBase;                               \
                        if (!mValid)                                            \
                            "bc0b47d5-8817-4870-8a0b-cdaeadd6ff1a"_log["sparse property"]("bad access to invalid property '${property_name}'")("property_name", ::std::string_view{#NAME})("property_type", ::std::string_view{#TYPE}).throw_FAILED_PRECONDITION(); \
                        return mValue;                                          \
                    }                                                           \
                                                                                \
            public : value_type const &                                         \
                operator*() const                                               \
                    {                                                           \
                        using namespace ::nsBase;                               \
                        if (!mValid)                                            \
                            "75cec35c-b2ad-484f-8075-80078b6289a1"_log["sparse property"]("bad access to invalid property '${property_name}'")("property_name", ::std::string_view{#NAME})("property_type", ::std::string_view{#TYPE}).throw_FAILED_PRECONDITION(); \
                        return mValue;                                          \
                    }                                                           \
                                                                                \
            public : value_type *                                               \
                operator->()                                                    \
                    {                                                           \
                        using namespace ::nsBase;                               \
                        if (!mValid)                                            \
                            "9228f22c-6c48-48db-a41b-b5cef3da3890"_log["sparse property"]("bad access to invalid property '${property_name}'")("property_name", ::std::string_view{#NAME})("property_type", ::std::string_view{#TYPE}).throw_FAILED_PRECONDITION(); \
                        return &mValue;                                         \
                    }                                                           \
                                                                                \
            public : value_type const *                                         \
                operator->() const                                              \
                    {                                                           \
                        using namespace ::nsBase;                               \
                        if (!mValid)                                            \
                            "e609ce5e-7cdd-4c55-bf2c-5b9a709f2a00"_log["sparse property"]("bad access to invalid property '${property_name}'")("property_name", ::std::string_view{#NAME})("property_type", ::std::string_view{#TYPE}).throw_FAILED_PRECONDITION(); \
                        return &mValue;                                         \
                    }                                                           \
        }                                                                       \
            QUALIFIER m_##NAME;                                                 \
        public : using                                                          \
            property_##NAME##_type = TYPE;                                      \
        private :


#define R_PROPERTY_FIELD2(      NAME    ,TYPE  ,ACCESSOR    ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK  )     \
                                                                                \
    private : class property_##NAME                                             \
        {                                                                       \
            public : using                                                      \
                value_type = TYPE;                                              \
                                                                                \
            private : value_type                                                \
                mValue;                                                         \
                                                                                \
            R_DTOR(property_##NAME) = default;                                  \
            R_CCPY(property_##NAME) = default;                                  \
            R_CMOV(property_##NAME) = default;                                  \
            R_COPY(property_##NAME) = default;                                  \
            R_MOVE(property_##NAME) = default;                                  \
                                                                                \
            public : template<class ... Constructor_args>                       \
                property_##NAME(Constructor_args && ... args)                   \
                    :   mValue(::std::forward<Constructor_args>(args)...)       \
                    {                                                           \
                    }                                                           \
                                                                                \
            public :                                                            \
                property_##NAME(                                                \
                        value_type && val                                       \
                    )                                                           \
                    :   mValue(::std::move(val))                                \
                    {                                                           \
                    }                                                           \
                                                                                \
            public : value_type &                                               \
                operator*()                                                     \
                    {                                                           \
                        return mValue;                                          \
                    }                                                           \
                                                                                \
            public : value_type const &                                         \
                operator*() const                                               \
                    {                                                           \
                        return mValue;                                          \
                    }                                                           \
                                                                                \
            public : value_type *                                               \
                operator->()                                                    \
                    {                                                           \
                        return &mValue;                                         \
                    }                                                           \
                                                                                \
            public : value_type const *                                         \
                operator->() const                                              \
                    {                                                           \
                        return &mValue;                                         \
                    }                                                           \
        }                                                                       \
            QUALIFIER m_##NAME;                                                 \
        public : using                                                          \
            property_##NAME##_type = TYPE;                                      \
        private :


#define R_PROPERTY_FIELD_R(      NAME    ,TYPE  ,ACCESSOR    ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK  )     \
                                                                                \
    private : class property_##NAME                                             \
        {                                                                       \
            static_assert(!::std::is_reference_v<TYPE>, "must not be a reference"); \
                                                                                \
            public : using                                                      \
                value_type = TYPE;                                              \
                                                                                \
            public : using                                                      \
                value_wrapper_type = ::std::reference_wrapper<TYPE>;            \
                                                                                \
            private : value_wrapper_type                                        \
                mValue;                                                         \
                                                                                \
            R_DTOR(property_##NAME) = default;                                  \
            R_CTOR(property_##NAME) = delete;                                   \
            R_CCPY(property_##NAME) = default;                                  \
            R_CMOV(property_##NAME) = default;                                  \
            R_COPY(property_##NAME) = default;                                  \
            R_MOVE(property_##NAME) = default;                                  \
                                                                                \
            public :                                                            \
                property_##NAME(                                                \
                        value_type & val                                        \
                    )                                                           \
                    :   mValue{val}                                             \
                    {                                                           \
                    }                                                           \
                                                                                \
            public : value_type &                                               \
                operator*()                                                     \
                    {                                                           \
                        return mValue;                                          \
                    }                                                           \
                                                                                \
            public : value_type const &                                         \
                operator*() const                                               \
                    {                                                           \
                        return mValue.get();                                    \
                    }                                                           \
                                                                                \
            public : value_type *                                               \
                operator->()                                                    \
                    {                                                           \
                        return &(mValue.get());                                 \
                    }                                                           \
                                                                                \
            public : value_type const *                                         \
                operator->() const                                              \
                    {                                                           \
                        return &(mValue.get());                                 \
                    }                                                           \
        }                                                                       \
            QUALIFIER m_##NAME;                                                 \
        public : using                                                          \
            property_##NAME##_type = TYPE;                                      \
        private :


////////////////////////////////////////////////////////////////////////////////
//  GET
//
#define R_PROPERTY_GET_(       NAME    ,TYPE  ,ACCESSOR    ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK  )  \
    public :  TYPE const &                                                                                  \
        NAME() const                                                                                        \
            {                                                                                               \
                return *m_##NAME;                                                                           \
            }

#define R_PROPERTY_GET_O(      NAME    ,TYPE  ,ACCESSOR    ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK  )  \
                                                                                                            \
    public :  TYPE const *                                                                                  \
        NAME##_if() const                                                                                   \
            {                                                                                               \
                if (NAME##_valid())                                                                         \
                    return &NAME();                                                                         \
                return {};                                                                                  \
            }                                                                                               \
                                                                                                            \
    public :  ::std::optional<TYPE>                                                                         \
        NAME##_if_as_opt() const                                                                            \
            {                                                                                               \
                if (NAME##_valid())                                                                         \
                    return NAME();                                                                          \
                return {};                                                                                  \
            }                                                                                               \
                                                                                                            \
    public :  TYPE                                                                                          \
        NAME##_or() const                                                                                   \
            {                                                                                               \
                if (NAME##_valid())                                                                         \
                    return {NAME()};                                                                        \
                return TYPE{};                                                                              \
            }                                                                                               \
                                                                                                            \
    public :  TYPE &                                                                                        \
        NAME##_mutable()                                                                                    \
            {                                                                                               \
                m_##NAME.validate();                                                                        \
                return *m_##NAME;                                                                           \
            }


#define R_PROPERTY_GET_m(      NAME    ,TYPE  ,ACCESSOR    ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK  )  \
    public :  TYPE &                                                                                        \
        NAME##_mutable()                                                                                    \
            {                                                                                               \
                return *m_##NAME;                                                                           \
            }

#define R_PROPERTY_GET_h(      NAME    ,TYPE  ,ACCESSOR    ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK  )  \
    public : TYPE                                                                                           \
        NAME() const                                                                                        \
            {                                                                                               \
                return GETHOOK();                                                                           \
            }


////////////////////////////////////////////////////////////////////////////////
//  Emplace
//
#define R_PROPERTY_EMP_O(      NAME    ,TYPE  ,ACCESSOR    ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK  )  \
    public :  template<class ... Constructor_args>                                                          \
        TYPE &                                                                                              \
        NAME##_emplace(Constructor_args && ... args)                                                        \
            {                                                                                               \
                m_##NAME.emplace(::std::forward<Constructor_args>(args)...);                                \
                                                                                                            \
                using namespace R_fallback;                                                                 \
                hash_of_memory_state_clear();                                                               \
                                                                                                            \
                return m_##NAME.operator*();                                                                \
            }                                                                                               \
                                                                                                            \
    public :  template<class ... Constructor_args>                                                          \
        TYPE &                                                                                              \
        NAME##_obtain(Constructor_args && ... args)                                                         \
            {                                                                                               \
                if (!NAME##_valid())                                                                        \
                {                                                                                           \
                    m_##NAME.emplace(::std::forward<Constructor_args>(args)...);                            \
                                                                                                            \
                    using namespace R_fallback;                                                             \
                    hash_of_memory_state_clear();                                                           \
                }                                                                                           \
                return m_##NAME.operator*();                                                                \
            }


////////////////////////////////////////////////////////////////////////////////
//  SET
//
#define R_PROPERTY_SET_(       NAME    ,TYPE  ,ACCESSOR    ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK  )  \
    ACCESSOR : decltype(auto)                                                                               \
        NAME##_assign(                                                                                      \
                TYPE const & inValue                                                                        \
            )                                                                                               \
            {                                                                                               \
                 *m_##NAME = inValue;                                                                       \
                                                                                                            \
                using namespace R_fallback;                                                                 \
                hash_of_memory_state_clear();                                                               \
                                                                                                            \
                return *this;                                                                               \
            }                                                                                               \
                                                                                                            \
    ACCESSOR : decltype(auto)                                                                               \
        NAME##_assign(                                                                                      \
                TYPE && inValue                                                                             \
            )                                                                                               \
            {                                                                                               \
                *m_##NAME = ::std::move(inValue);                                                           \
                                                                                                            \
                using namespace R_fallback;                                                                 \
                hash_of_memory_state_clear();                                                               \
                                                                                                            \
                return *this;                                                                               \
            }                                                                                               \
    ACCESSOR : decltype(auto)                                                                               \
        NAME(                                                                                               \
                TYPE const & inValue                                                                        \
            )                                                                                               \
            {                                                                                               \
                return NAME##_assign(inValue);                                                              \
            }                                                                                               \
                                                                                                            \
    ACCESSOR : decltype(auto)                                                                               \
        NAME(                                                                                               \
                TYPE && inValue                                                                             \
            )                                                                                               \
            {                                                                                               \
                return NAME##_assign(::std::move(inValue));                                                 \
            }

#define R_PROPERTY_SET_O(      NAME    ,TYPE  ,ACCESSOR    ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK  )  \
    ACCESSOR : decltype(auto)                                                                               \
        NAME##_assign(                                                                                      \
                TYPE const & inValue                                                                        \
            )                                                                                               \
            {                                                                                               \
                m_##NAME.assign(inValue);                                                                   \
                                                                                                            \
                using namespace R_fallback;                                                                 \
                hash_of_memory_state_clear();                                                               \
                                                                                                            \
                return *this;                                                                               \
            }                                                                                               \
                                                                                                            \
    ACCESSOR : decltype(auto)                                                                               \
        NAME##_assign(                                                                                      \
                TYPE && inValue                                                                             \
            )                                                                                               \
            {                                                                                               \
                m_##NAME.assign(::std::move(inValue));                                                      \
                                                                                                            \
                using namespace R_fallback;                                                                 \
                hash_of_memory_state_clear();                                                               \
                                                                                                            \
                return *this;                                                                               \
            }                                                                                               \
                                                                                                            \
    ACCESSOR : decltype(auto)                                                                               \
        NAME##_assign_if(                                                                                   \
                TYPE const * inValue                                                                        \
            )                                                                                               \
            {                                                                                               \
                if (inValue)                                                                                \
                    NAME##_assign(*inValue);                                                                \
                                                                                                            \
                return *this;                                                                               \
            }                                                                                               \
                                                                                                            \
    ACCESSOR : decltype(auto)                                                                               \
        NAME##_assign_if(                                                                                   \
                ::std::optional<TYPE> const & inValue                                                       \
            )                                                                                               \
            {                                                                                               \
                if (inValue)                                                                                \
                    NAME##_assign(*inValue);                                                                \
                                                                                                            \
                return *this;                                                                               \
            }                                                                                               \
                                                                                                            \
    ACCESSOR : decltype(auto)                                                                               \
        NAME##_assign_if(                                                                                   \
                ::std::optional<TYPE> && inValue                                                            \
            )                                                                                               \
            {                                                                                               \
                if (inValue)                                                                                \
                    NAME##_assign(::std::move(*inValue));                                                   \
                                                                                                            \
                return *this;                                                                               \
            }                                                                                               \
                                                                                                            \
    ACCESSOR : decltype(auto)                                                                               \
        NAME(                                                                                               \
                TYPE const & inValue                                                                        \
            )                                                                                               \
            {                                                                                               \
                return NAME##_assign(inValue);                                                              \
            }                                                                                               \
                                                                                                            \
    ACCESSOR : decltype(auto)                                                                               \
        NAME(                                                                                               \
                TYPE && inValue                                                                             \
            )                                                                                               \
            {                                                                                               \
                return NAME##_assign(::std::move(inValue));                                                 \
            }                                                                                               \
                                                                                                            \
    ACCESSOR : decltype(auto)                                                                               \
        NAME##_if(                                                                                          \
                TYPE const * inValue                                                                        \
            )                                                                                               \
            {                                                                                               \
                return NAME##_assign_if(inValue);                                                           \
            }                                                                                               \
                                                                                                            \
    ACCESSOR : decltype(auto)                                                                               \
        NAME##_if(                                                                                          \
                ::std::optional<TYPE> const & inValue                                                       \
            )                                                                                               \
            {                                                                                               \
                return NAME##_assign_if(inValue);                                                           \
            }                                                                                               \
                                                                                                            \
    ACCESSOR : decltype(auto)                                                                               \
        NAME##_if(                                                                                          \
                ::std::optional<TYPE> && inValue                                                            \
            )                                                                                               \
            {                                                                                               \
                return NAME##_assign_if(::std::move(inValue));                                              \
            }                                                                                               \
                                                                                                            \
    ACCESSOR : decltype(auto)                                                                               \
        NAME##_clone(                                                                                       \
                TYPE const * inValue                                                                        \
            )                                                                                               \
            {                                                                                               \
                if (inValue)                                                                                \
                    NAME##_assign(*inValue);                                                                \
                else                                                                                        \
                    NAME##_clear();                                                                         \
                                                                                                            \
                return *this;                                                                               \
            }                                                                                               \
                                                                                                            \
ACCESSOR : decltype(auto)                                                                                   \
        NAME##_clone(                                                                                       \
                ::std::optional<TYPE> const & inValue                                                       \
            )                                                                                               \
            {                                                                                               \
                if (inValue)                                                                                \
                    NAME##_assign(*inValue);                                                                \
                else                                                                                        \
                    NAME##_clear();                                                                         \
                                                                                                            \
                return *this;                                                                               \
            }

// copy and move
#define R_PROPERTY_SET_2(       NAME    ,TYPE  ,ACCESSOR    ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK  ) \
    ACCESSOR : decltype(auto)                                                                               \
        NAME##_assign(                                                                                      \
                TYPE const & inValue                                                                        \
            )                                                                                               \
            {                                                                                               \
                *m_##NAME = inValue;                                                                        \
                                                                                                            \
                using namespace R_fallback;                                                                 \
                hash_of_memory_state_clear();                                                               \
                                                                                                            \
                return *this;                                                                               \
            }                                                                                               \
                                                                                                            \
    ACCESSOR : decltype(auto)                                                                               \
        NAME##_assign(                                                                                      \
                TYPE && inValue                                                                             \
            )                                                                                               \
            {                                                                                               \
                *m_##NAME = ::std::move(inValue);                                                           \
                                                                                                            \
                using namespace R_fallback;                                                                 \
                hash_of_memory_state_clear();                                                               \
                                                                                                            \
                return *this;                                                                               \
            }

// move
#define R_PROPERTY_SET_3(       NAME    ,TYPE  ,ACCESSOR    ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK  ) \
                                                                                                            \
    ACCESSOR : decltype(auto)                                                                               \
        NAME##_assign(                                                                                      \
                TYPE && inValue                                                                             \
            )                                                                                               \
            {                                                                                               \
                *m_##NAME = ::std::move(inValue);                                                           \
                                                                                                            \
                using namespace R_fallback;                                                                 \
                hash_of_memory_state_clear();                                                               \
                                                                                                            \
                return *this;                                                                               \
            }

#if 0 //KU: R_PROPERTY_SET_3 shall support non-copyable value types, thus a copy-assignment is not supported
    ACCESSOR : decltype(auto)                                                                               \
        NAME##_assign(                                                                                      \
                TYPE const & inValue                                                                        \
            )                                                                                               \
            {                                                                                               \
                 *m_##NAME = inValue;                                                                       \
                                                                                                            \
                using namespace R_fallback;                                                                 \
                hash_of_memory_state_clear();                                                               \
                                                                                                            \
                return *this;                                                                               \
            }
#endif


#define R_PROPERTY_SET_3h(     NAME    ,TYPE  ,ACCESSOR    ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK  )  \
    ACCESSOR : decltype(auto)                                                                               \
        NAME##_assign(                                                                                      \
                TYPE && inValue                                                                             \
            )                                                                                               \
            {                                                                                               \
                SETHOOK(::std::move(inValue));                                                              \
                using namespace R_fallback;                                                                 \
                hash_of_memory_state_clear();                                                               \
                                                                                                            \
                return *this;                                                                               \
            }

#define R_PROPERTY_SET_h(      NAME    ,TYPE  ,ACCESSOR    ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK  )  \
    ACCESSOR : decltype(auto)                                                                               \
        NAME##_assign(                                                                                      \
                TYPE const & inValue                                                                        \
            )                                                                                               \
            {                                                                                               \
                SETHOOK(inValue);                                                                           \
                using namespace R_fallback;                                                                 \
                hash_of_memory_state_clear();                                                               \
                                                                                                            \
                return *this;                                                                               \
            }


////////////////////////////////////////////////////////////////////////////////
//  DEFAULT
//
#define R_PROPERTY_DEFAULT(    NAME    ,TYPE  ,ACCESSOR    ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK  )  \
    public : static TYPE                                                                                    \
        [[nodiscard]] NAME##_default()                                                                      \
            {                                                                                               \
                return DEFAULT;                                                                             \
            }

#define R_PROPERTY_DEFAULTC(   NAME    ,TYPE  ,ACCESSOR    ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK  )  \
    public : [[nodiscard]] static TYPE                                                                      \
        NAME##_default()                                                                                    \
            {                                                                                               \
                return {};                                                                                  \
            }

////////////////////////////////////////////////////////////////////////////////
#define R_PROPERTY_CLEAR(      NAME    ,TYPE  ,ACCESSOR    ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK  )  \
    ACCESSOR : decltype(auto)                                                                               \
        NAME##_clear()                                                                                      \
            {                                                                                               \
                NAME##_assign( NAME##_default() );                                                          \
                                                                                                            \
                return *this;                                                                               \
            }

////////////////////////////////////////////////////////////////////////////////
#define R_PROPERTY_CLEAR_O(    NAME    ,TYPE  ,ACCESSOR    ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK  )  \
    ACCESSOR : decltype(auto)                                                                               \
        NAME##_clear()                                                                                      \
            {                                                                                               \
                m_##NAME.clear();                                                                           \
                                                                                                            \
                return *this;                                                                               \
            }

////////////////////////////////////////////////////////////////////////////////
#define R_PROPERTY_VALID_O(    NAME    ,TYPE  ,ACCESSOR    ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK  )  \
    ACCESSOR : bool                                                                                         \
        NAME##_valid() const                                                                                \
            {                                                                                               \
                return m_##NAME.valid();                                                                    \
            }

////////////////////////////////////////////////////////////////////////////////
#define R_PROPERTY_OPTI_O( NAME    ,TYPE  ,ACCESSOR    ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK  )      \
    ACCESSOR : static bool                                                                                  \
        NAME##_is_optional()                                                                                \
            {                                                                                               \
                return ::nsBase::is_optional<TYPE>();                                                       \
            }

////////////////////////////////////////////////////////////////////////////////
#define R_PROPERTY_ADGS(       NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,GETHOOK                ,SETHOOK    )   \
        R_PROPERTY_FIELD_D (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,GETHOOK    ,           ,SETHOOK    )   \
        R_PROPERTY_DEFAULT (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,GETHOOK    ,           ,SETHOOK    )   \
        R_PROPERTY_GET_h   (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,GETHOOK    ,           ,SETHOOK    )   \
        R_PROPERTY_SET_h   (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,GETHOOK    ,           ,SETHOOK    )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,GETHOOK    ,           ,SETHOOK    )

#define R_PROPERTY_DGS(        NAME    ,TYPE               ,DEFAULT    ,GETHOOK                ,SETHOOK    )   \
        R_PROPERTY_FIELD_D (   NAME    ,TYPE   ,public     ,DEFAULT    ,GETHOOK    ,           ,SETHOOK    )   \
        R_PROPERTY_DEFAULT (   NAME    ,TYPE   ,public     ,DEFAULT    ,GETHOOK    ,           ,SETHOOK    )   \
        R_PROPERTY_GET_h   (   NAME    ,TYPE   ,public     ,DEFAULT    ,GETHOOK    ,           ,SETHOOK    )   \
        R_PROPERTY_SET_h   (   NAME    ,TYPE   ,public     ,DEFAULT    ,GETHOOK    ,           ,SETHOOK    )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,public     ,DEFAULT    ,GETHOOK    ,           ,SETHOOK    )

#define R_PROPERTY_DS(         NAME    ,TYPE               ,DEFAULT                            ,SETHOOK    )   \
        R_PROPERTY_FIELD_D (   NAME    ,TYPE   ,public     ,DEFAULT    ,0          ,           ,SETHOOK    )   \
        R_PROPERTY_DEFAULT (   NAME    ,TYPE   ,public     ,DEFAULT    ,0          ,           ,SETHOOK    )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,public     ,DEFAULT    ,0          ,           ,SETHOOK    )   \
        R_PROPERTY_SET_h   (   NAME    ,TYPE   ,public     ,DEFAULT    ,0          ,           ,SETHOOK    )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,public     ,DEFAULT    ,0          ,           ,SETHOOK    )

#define R_PROPERTY_DG(         NAME    ,TYPE               ,DEFAULT    ,GETHOOK                            )   \
        R_PROPERTY_FIELD_D (   NAME    ,TYPE   ,public     ,DEFAULT    ,GETHOOK    ,           ,0          )   \
        R_PROPERTY_DEFAULT (   NAME    ,TYPE   ,public     ,DEFAULT    ,GETHOOK    ,           ,0          )   \
        R_PROPERTY_GET_h   (   NAME    ,TYPE   ,public     ,DEFAULT    ,GETHOOK    ,           ,0          )   \
        R_PROPERTY_SET_    (   NAME    ,TYPE   ,public     ,DEFAULT    ,GETHOOK    ,           ,0          )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,public     ,DEFAULT    ,GETHOOK    ,           ,0          )

#define R_PROPERTY_D(          NAME    ,TYPE               ,DEFAULT                                        )   \
        R_PROPERTY_FIELD_D (   NAME    ,TYPE   ,public     ,DEFAULT    ,0          ,           ,0          )   \
        R_PROPERTY_DEFAULT (   NAME    ,TYPE   ,public     ,DEFAULT    ,0          ,           ,0          )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,public     ,DEFAULT    ,0          ,           ,0          )   \
        R_PROPERTY_SET_    (   NAME    ,TYPE   ,public     ,DEFAULT    ,0          ,           ,0          )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,public     ,DEFAULT    ,0          ,           ,0          )

#define R_PROPERTY_ADS(        NAME    ,TYPE   ,ACCESSOR   ,DEFAULT                            ,SETHOOK    )   \
        R_PROPERTY_FIELD_D (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,0          ,           ,SETHOOK    )   \
        R_PROPERTY_DEFAULT (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,0          ,           ,SETHOOK    )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,0          ,           ,SETHOOK    )   \
        R_PROPERTY_SET_h   (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,0          ,           ,SETHOOK    )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,0          ,           ,SETHOOK    )

#define R_PROPERTY_ADG(        NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,GETHOOK                            )   \
        R_PROPERTY_FIELD_D (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,GETHOOK    ,           ,0          )   \
        R_PROPERTY_DEFAULT (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,GETHOOK    ,           ,0          )   \
        R_PROPERTY_GET_h   (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,GETHOOK    ,           ,0          )   \
        R_PROPERTY_SET_    (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,GETHOOK    ,           ,0          )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,GETHOOK    ,           ,0          )

#define R_PROPERTY_AD(         NAME    ,TYPE   ,ACCESSOR   ,DEFAULT                                        )   \
        R_PROPERTY_FIELD_D (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,0          ,           ,0          )   \
        R_PROPERTY_DEFAULT (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,0          ,           ,0          )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,0          ,           ,0          )   \
        R_PROPERTY_SET_    (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,0          ,           ,0          )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,0          ,           ,0          )

#define R_PROPERTY_AGS(        NAME    ,TYPE   ,ACCESSOR               ,GETHOOK                ,SETHOOK    )   \
        R_PROPERTY_FIELD   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,GETHOOK    ,           ,SETHOOK    )   \
        R_PROPERTY_DEFAULTC(   NAME    ,TYPE   ,ACCESSOR   ,{}         ,GETHOOK    ,           ,SETHOOK    )   \
        R_PROPERTY_GET_h   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,GETHOOK    ,           ,SETHOOK    )   \
        R_PROPERTY_SET_h   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,GETHOOK    ,           ,SETHOOK    )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,GETHOOK    ,           ,SETHOOK    )

#define R_PROPERTY_GS(         NAME    ,TYPE                           ,GETHOOK                ,SETHOOK    )   \
        R_PROPERTY_FIELD   (   NAME    ,TYPE   ,public     ,{}         ,GETHOOK    ,           ,SETHOOK    )   \
        R_PROPERTY_DEFAULTC(   NAME    ,TYPE   ,public     ,{}         ,GETHOOK    ,           ,SETHOOK    )   \
        R_PROPERTY_GET_h   (   NAME    ,TYPE   ,public     ,{}         ,GETHOOK    ,           ,SETHOOK    )   \
        R_PROPERTY_SET_h   (   NAME    ,TYPE   ,public     ,{}         ,GETHOOK    ,           ,SETHOOK    )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,public     ,{}         ,GETHOOK    ,           ,SETHOOK    )

#define R_PROPERTY_S(          NAME    ,TYPE                                                   ,SETHOOK    )   \
        R_PROPERTY_FIELD   (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,SETHOOK    )   \
        R_PROPERTY_DEFAULTC(   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,SETHOOK    )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,SETHOOK    )   \
        R_PROPERTY_SET_h   (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,SETHOOK    )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,SETHOOK    )

#define R_PROPERTY_G(          NAME    ,TYPE                           ,GETHOOK                            )   \
        R_PROPERTY_FIELD   (   NAME    ,TYPE   ,public     ,{}         ,GETHOOK    ,           ,0          )   \
        R_PROPERTY_DEFAULTC(   NAME    ,TYPE   ,public     ,{}         ,GETHOOK    ,           ,0          )   \
        R_PROPERTY_GET_h   (   NAME    ,TYPE   ,public     ,{}         ,GETHOOK    ,           ,0          )   \
        R_PROPERTY_SET_    (   NAME    ,TYPE   ,public     ,{}         ,GETHOOK    ,           ,0          )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,public     ,{}         ,GETHOOK    ,           ,0          )

#define R_PROPERTY_(           NAME    ,TYPE                                                               )   \
        R_PROPERTY_FIELD   (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_DEFAULTC(   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_SET_    (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )

#define R_PROPERTY_M(          NAME    ,TYPE                                                               )   \
        R_PROPERTY_FIELD   (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_DEFAULTC(   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_GET_m   (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_SET_    (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )

#define R_PROPERTY_O(          NAME    ,TYPE                                                               )   \
        R_PROPERTY_FIELD_O (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_GET_O   (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_EMP_O   (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_CLEAR_O (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_SET_O   (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_VALID_O (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_OPTI_O  (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )

#define R_PROPERTY_2(          NAME    ,TYPE                                                               )   \
        R_PROPERTY_FIELD2  (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_SET_2   (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )

#define R_PROPERTY_3(          NAME    ,TYPE                                                               )   \
        R_PROPERTY_FIELD2  (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_SET_3   (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )

#define R_PROPERTY_3M(         NAME    ,TYPE                                                               )   \
        R_PROPERTY_FIELD2  (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_GET_m   (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_SET_3   (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )

#define R_PROPERTY_3MQ(        NAME    ,TYPE                                       ,QUALIFIER              )   \
        R_PROPERTY_FIELD2  (   NAME    ,TYPE   ,public     ,{}         ,0          ,QUALIFIER  ,0          )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,public     ,{}         ,0          ,QUALIFIER  ,0          )   \
        R_PROPERTY_GET_m   (   NAME    ,TYPE   ,public     ,{}         ,0          ,QUALIFIER  ,0          )   \
        R_PROPERTY_SET_3   (   NAME    ,TYPE   ,public     ,{}         ,0          ,QUALIFIER  ,0          )

#define R_PROPERTY_3AM(        NAME    ,TYPE   ,ACCESSOR                                                   )   \
        R_PROPERTY_FIELD2  (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_GET_m   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_SET_3   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,0          ,           ,0          )

#define R_PROPERTY_3MS(        NAME    ,TYPE                                                   ,SETHOOK    )   \
        R_PROPERTY_FIELD2  (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_GET_m   (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_SET_3h  (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,SETHOOK    )   \

#define R_PROPERTY_AS(         NAME    ,TYPE   ,ACCESSOR                                       ,SETHOOK    )   \
        R_PROPERTY_FIELD   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,0          ,           ,SETHOOK    )   \
        R_PROPERTY_DEFAULTC(   NAME    ,TYPE   ,ACCESSOR   ,{}         ,0          ,           ,SETHOOK    )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,0          ,           ,SETHOOK    )   \
        R_PROPERTY_SET_h   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,0          ,           ,SETHOOK    )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,0          ,           ,SETHOOK    )

#define R_PROPERTY_AG(         NAME    ,TYPE   ,ACCESSOR               ,GETHOOK                            )   \
        R_PROPERTY_FIELD   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,GETHOOK    ,           ,0          )   \
        R_PROPERTY_DEFAULTC(   NAME    ,TYPE   ,ACCESSOR   ,{}         ,GETHOOK    ,           ,0          )   \
        R_PROPERTY_GET_h   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,GETHOOK    ,           ,0          )   \
        R_PROPERTY_SET_    (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,GETHOOK    ,           ,0          )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,GETHOOK    ,           ,0          )

#define R_PROPERTY_A(          NAME    ,TYPE   ,ACCESSOR                                                   )   \
        R_PROPERTY_FIELD   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_DEFAULTC(   NAME    ,TYPE   ,ACCESSOR   ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_SET_    (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,0          ,           ,0          )

#define R_PROPERTY_MR(         NAME    ,TYPE                                                               )   \
        R_PROPERTY_FIELD_R (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_GET_m   (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )
// TYPE was not copyable
//      R_PROPERTY_SET_    (   NAME    ,TYPE   ,public     ,{}         ,0          ,           ,0          )

////////////////////////////////////////////////////////////////////////////////
#define R_PROPERTY_ADGQS(      NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_FIELD_D (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_DEFAULT (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_GET_h   (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_SET_h   (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK    )

#define R_PROPERTY_DGQS(       NAME    ,TYPE               ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_FIELD_D (   NAME    ,TYPE   ,public     ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_DEFAULT (   NAME    ,TYPE   ,public     ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_GET_h   (   NAME    ,TYPE   ,public     ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_SET_h   (   NAME    ,TYPE   ,public     ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,public     ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,SETHOOK    )

#define R_PROPERTY_DQS(        NAME    ,TYPE               ,DEFAULT                ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_FIELD_D (   NAME    ,TYPE   ,public     ,DEFAULT    ,0          ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_DEFAULT (   NAME    ,TYPE   ,public     ,DEFAULT    ,0          ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,public     ,DEFAULT    ,0          ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_SET_h   (   NAME    ,TYPE   ,public     ,DEFAULT    ,0          ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,public     ,DEFAULT    ,0          ,QUALIFIER  ,SETHOOK    )

#define R_PROPERTY_DGQ(        NAME    ,TYPE               ,DEFAULT    ,GETHOOK    ,QUALIFIER              )   \
        R_PROPERTY_FIELD_D (   NAME    ,TYPE   ,public     ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,0          )   \
        R_PROPERTY_DEFAULT (   NAME    ,TYPE   ,public     ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,0          )   \
        R_PROPERTY_GET_h   (   NAME    ,TYPE   ,public     ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,0          )   \
        R_PROPERTY_SET_    (   NAME    ,TYPE   ,public     ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,0          )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,public     ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,0          )

#define R_PROPERTY_DQ(         NAME    ,TYPE               ,DEFAULT                ,QUALIFIER              )   \
        R_PROPERTY_FIELD_D (   NAME    ,TYPE   ,public     ,DEFAULT    ,0          ,QUALIFIER  ,0          )   \
        R_PROPERTY_DEFAULT (   NAME    ,TYPE   ,public     ,DEFAULT    ,0          ,QUALIFIER  ,0          )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,public     ,DEFAULT    ,0          ,QUALIFIER  ,0          )   \
        R_PROPERTY_SET_    (   NAME    ,TYPE   ,public     ,DEFAULT    ,0          ,QUALIFIER  ,0          )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,public     ,DEFAULT    ,0          ,QUALIFIER  ,0          )

#define R_PROPERTY_ADQS(       NAME    ,TYPE   ,ACCESSOR   ,DEFAULT                ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_FIELD_D (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,0          ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_DEFAULT (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,0          ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,0          ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_SET_h   (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,0          ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,0          ,QUALIFIER  ,SETHOOK    )

#define R_PROPERTY_ADGQ(       NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,GETHOOK    ,QUALIFIER              )   \
        R_PROPERTY_FIELD_D (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,0          )   \
        R_PROPERTY_DEFAULT (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,0          )   \
        R_PROPERTY_GET_h   (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,0          )   \
        R_PROPERTY_SET_    (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,0          )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,GETHOOK    ,QUALIFIER  ,0          )

#define R_PROPERTY_ADQ(        NAME    ,TYPE   ,ACCESSOR   ,DEFAULT                ,QUALIFIER              )   \
        R_PROPERTY_FIELD_D (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,0          ,QUALIFIER  ,0          )   \
        R_PROPERTY_DEFAULT (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,0          ,QUALIFIER  ,0          )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,0          ,QUALIFIER  ,0          )   \
        R_PROPERTY_SET_    (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,0          ,QUALIFIER  ,0          )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,ACCESSOR   ,DEFAULT    ,0          ,QUALIFIER  ,0          )

#define R_PROPERTY_AGQS(       NAME    ,TYPE   ,ACCESSOR               ,GETHOOK    ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_FIELD   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,GETHOOK    ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_DEFAULTC(   NAME    ,TYPE   ,ACCESSOR   ,{}         ,GETHOOK    ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_GET_h   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,GETHOOK    ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_SET_h   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,GETHOOK    ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,GETHOOK    ,QUALIFIER  ,SETHOOK    )

#define R_PROPERTY_GQS(        NAME    ,TYPE                           ,GETHOOK    ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_FIELD   (   NAME    ,TYPE   ,public     ,{}         ,GETHOOK    ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_DEFAULTC(   NAME    ,TYPE   ,public     ,{}         ,GETHOOK    ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_GET_h   (   NAME    ,TYPE   ,public     ,{}         ,GETHOOK    ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_SET_h   (   NAME    ,TYPE   ,public     ,{}         ,GETHOOK    ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,public     ,{}         ,GETHOOK    ,QUALIFIER  ,SETHOOK    )

#define R_PROPERTY_QS(         NAME    ,TYPE                                       ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_FIELD   (   NAME    ,TYPE   ,public     ,{}         ,0          ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_DEFAULTC(   NAME    ,TYPE   ,public     ,{}         ,0          ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,public     ,{}         ,0          ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_SET_h   (   NAME    ,TYPE   ,public     ,{}         ,0          ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,public     ,{}         ,0          ,QUALIFIER  ,SETHOOK    )

#define R_PROPERTY_GQ(         NAME    ,TYPE                           ,GETHOOK    ,QUALIFIER              )   \
        R_PROPERTY_FIELD   (   NAME    ,TYPE   ,public     ,{}         ,GETHOOK    ,QUALIFIER  ,0          )   \
        R_PROPERTY_DEFAULTC(   NAME    ,TYPE   ,public     ,{}         ,GETHOOK    ,QUALIFIER  ,0          )   \
        R_PROPERTY_GET_h   (   NAME    ,TYPE   ,public     ,{}         ,GETHOOK    ,QUALIFIER  ,0          )   \
        R_PROPERTY_SET_    (   NAME    ,TYPE   ,public     ,{}         ,GETHOOK    ,QUALIFIER  ,0          )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,public     ,{}         ,GETHOOK    ,QUALIFIER  ,0          )

#define R_PROPERTY_Q(          NAME    ,TYPE                                       ,QUALIFIER              )   \
        R_PROPERTY_FIELD   (   NAME    ,TYPE   ,public     ,{}         ,0          ,QUALIFIER  ,0          )   \
        R_PROPERTY_DEFAULTC(   NAME    ,TYPE   ,public     ,{}         ,0          ,QUALIFIER  ,0          )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,public     ,{}         ,0          ,QUALIFIER  ,0          )   \
        R_PROPERTY_SET_    (   NAME    ,TYPE   ,public     ,{}         ,0          ,QUALIFIER  ,0          )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,public     ,{}         ,0          ,QUALIFIER  ,0          )

#define R_PROPERTY_AQS(        NAME    ,TYPE   ,ACCESSOR                           ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_FIELD   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,0          ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_DEFAULTC(   NAME    ,TYPE   ,ACCESSOR   ,{}         ,0          ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,0          ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_SET_h   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,0          ,QUALIFIER  ,SETHOOK    )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,0          ,QUALIFIER  ,SETHOOK    )

#define R_PROPERTY_AGQ(        NAME    ,TYPE   ,ACCESSOR               ,GETHOOK    ,QUALIFIER              )   \
        R_PROPERTY_FIELD   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,GETHOOK    ,QUALIFIER  ,0          )   \
        R_PROPERTY_DEFAULTC(   NAME    ,TYPE   ,ACCESSOR   ,{}         ,GETHOOK    ,QUALIFIER  ,0          )   \
        R_PROPERTY_GET_h   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,GETHOOK    ,QUALIFIER  ,0          )   \
        R_PROPERTY_SET_    (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,GETHOOK    ,QUALIFIER  ,0          )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,GETHOOK    ,QUALIFIER  ,0          )

#define R_PROPERTY_AQ(         NAME    ,TYPE   ,ACCESSOR                           ,QUALIFIER              )   \
        R_PROPERTY_FIELD   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,0          ,QUALIFIER  ,0          )   \
        R_PROPERTY_DEFAULTC(   NAME    ,TYPE   ,ACCESSOR   ,{}         ,0          ,QUALIFIER  ,0          )   \
        R_PROPERTY_GET_    (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,0          ,QUALIFIER  ,0          )   \
        R_PROPERTY_SET_    (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,0          ,QUALIFIER  ,0          )   \
        R_PROPERTY_CLEAR   (   NAME    ,TYPE   ,ACCESSOR   ,{}         ,0          ,QUALIFIER  ,0          )
