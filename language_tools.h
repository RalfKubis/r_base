#pragma once

#include <cstddef>

////////////////////////////////////////
/**
    Constructor/Destructor definition
    Rule of 0/3/5
*/

#if 0 // for quick copy paste
    R_DTOR() = default;
    R_CTOR() = default;
    R_CCPY() = default;
    R_CMOV() = default;
    R_COPY() = default;
    R_MOVE() = default;
#endif

#define R_CTOR_CE(t)   public : constexpr t()
#define R_CCPY_CE(t)   public : constexpr t(t const & src)
#define R_CMOV_CE(t)   public : constexpr t(t && src)
#define R_COPY_CE(t)   public : constexpr t & operator=(t const & src)
#define R_MOVE_CE(t)   public : constexpr t & operator=(t && src)

#define R_VTOR(t)   public : virtual ~t()

#define R_DTOR(t)   public : ~t()
#define R_CTOR(t)   public : t()
#define R_CCPY(t)   public : t(t const & src)
#define R_CMOV(t)   public : t(t && src)
#define R_COPY(t)   public : t & operator=(t const & src)
#define R_MOVE(t)   public : t & operator=(t && src)

#define R_DTOR_(t)   ~t()
#define R_CTOR_(t)   t()
#define R_CCPY_(t)   t(t const & src)
#define R_CMOV_(t)   t(t && src)
#define R_COPY_(t)   t & operator=(t const & src)
#define R_MOVE_(t)   t & operator=(t && src)

#define R_DTOR_IMPL(t)   t::~t()
#define R_CTOR_IMPL(t)   t::t()
#define R_CCPY_IMPL(t)   t::t(t const & src)
#define R_CMOV_IMPL(t)   t::t(t && src)
#define R_COPY_IMPL(t)   t & t::operator=(t const & src)
#define R_MOVE_IMPL(t)   t & t::operator=(t && src)

////////////////////////////////////////
/**
    BLOCK/FIN/LEAVE
*/
#define BLOCK   do
#define LEAVE   break
#define FIN     while(false);


////////////////////////////////////////
/**
    Helper macro to express logical implication.
    This must be a macro in order to make sure that the conclusion expression
    only gets evaluated if the premise is true.


    \param  premise       The antecedent.
    \param  conclusion    The consequent.

    \return
    FALSE if the premise is TRUE and the conclusion is FALSE, otherwise TRUE.
*/
#define ifThen(premise, conclusion) \
    (bool(premise) ? bool(conclusion) : true)


////////////////////////////////////////
/**
    Helper macro to mark code.
*/
#if 0

#   define TODO(MSG)      __pragma( message( "TODO: ["      __FILE__ ":" STRING(__LINE__) "] " #MSG ) )

#   ifdef VI_SHOW_TODO_HEADER
#       define TODO_H(MSG) TODO(MSG)
#   else
#       define TODO_H(MSG)
#   endif

#   define NORELEASE(MSG) __pragma( message( "NORELEASE: [" __FILE__ ":" STRING(__LINE__) "] " #MSG ) )

#else
#define TODO(MSG)
#define TODO_H(MSG)
#define NORELEASE(MSG)
#endif

constexpr ::std::size_t
    operator "" _sz (unsigned long long int x)
        {
            return x;
        }

template<typename T>
void
    R_DELETE(T* & t)
        {
            delete t;
            t = nullptr;
        }


#include "r_base/dbc.h"
#include "r_base/uuid.h"
#include "r_base/Property.h"
