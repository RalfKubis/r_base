#pragma once
/* Copyright (C) Ralf Kubis */

#include "r_base/language_tools.h"
#include "r_base/Log.h"
#include "r_base/expected.h"

#include <memory>
#include <exception>
#include <string>
#include <algorithm>


namespace nsBase
{
class Error
:   public ::std::exception
{
    R_DTOR(Error) = default;
    R_CTOR(Error) = delete;

    // This is a Hack to enable throwing via ::tl::bad_expected_access;
    // The implementation of ::tl::ecpected::value() throws a copy of the error object.
    R_CCPY(Error)
        {
            m_log = ::std::move(src.m_log);
            src.m_log.do_broadcast_assign(false);
            src.m_log.broadcast_if_and_clear();
        }

    R_CMOV(Error)
        {
            operator=(::std::move(src));
        }

    R_COPY(Error) = delete;
    R_MOVE(Error)
        {
            m_log = ::std::move(src.m_log);
            return *this;
        }

    public : Error(Log && l, ::std::string const message = {})
        :   m_log{::std::move(l)}
        {
            m_log.level_raise_to(Log::Level::FAILURE);

            if (m_log.ok())
                m_log.status(Log::Status::INTERNAL);

            if (!message.empty())
                m_log(message);
        }

    public : Error(::uuids::uuid id)
        :   Error{Log{id}}
        {
        }

    public : virtual const char *
        what() const noexcept override
            {
                using namespace ::std::string_literals;

                try
                {
                    what_cache = "b25bdc13-2348-417a-bbae-9147b34539ed"s;
                    what_cache = log().message_resolved();
                }
                catch(...)
                {}

                return what_cache.c_str();
            }

    private : mutable ::std::string
        what_cache;

    private : mutable Log
        m_log;

    public : Log const &
        log() const
            {
                return m_log;
            }

    public : Log &
        log_mutable()
            {
                return m_log;
            }

    public : Log &
        operator()(
                ::std::string_view const & inValue
            )
            {
                m_log("message", inValue);
                return m_log;
            }
};


/** Get an Error object that reflects the given exception object.
    If e is of class Error or ::tl::bad_expected_access<Error>, the Log is obtained from e.
    Otherwise a new Log is created with the value of ::std::exception::what()
    stored in its 'message' attribute.
    In any case, the value of parameter 'trace_id' is added to the Logs trace
    to make the routing of the event reproducible.
*/
inline Error
to_Error(
    ::std::exception  & e
,   ::uuids::uuid       trace_id
)
{
    // in case e is ::nsBase::Error
    if (auto err = dynamic_cast<Error*>(&e))
        return {::std::move(err->log_mutable().trace(trace_id))};

    // in case e is ::tl::bad_expected_access<::nsBase::Error>
    if (auto err = dynamic_cast<::tl::bad_expected_access<Error>*>(&e))
        return {::std::move(err->error().log_mutable().trace(trace_id))};

    return Log{trace_id}(e.what()).trace(trace_id).move();
}

inline Error
copy_to_Error(
    ::std::exception const & e
,   ::uuids::uuid    trace_id
)
{
    // in case e is ::nsBase::Error
    if (auto err = dynamic_cast<Error const*>(&e))
        return err->log().copy().trace(trace_id).move();

    // in case e is ::tl::bad_expected_access<::nsBase::Error>
    if (auto err = dynamic_cast<::tl::bad_expected_access<Error>const*>(&e))
        return err->error().log().copy().trace(trace_id).move();

    return Log(trace_id)(e.what()).trace(trace_id).move();
}


template<class Expected>
void
throw_on_error(
    Expected         & exp
,   ::uuids::uuid      trace_id
)
{
    if (!exp)
        throw to_Error(exp.error(), trace_id);
}



void
throw_on_error_win(
    bool              is_ok
,   ::uuids::uuid     trace_id
);

using StatusInfo = ::tl::expected<bool, ::nsBase::Error>;

inline ::tl::expected<bool, ::nsBase::Error> &
    operator<<(
            ::tl::expected<bool, ::nsBase::Error>  & dst
        ,   ::tl::expected<bool, ::nsBase::Error> && src
        )
        {
            if (!src)
                dst = ::tl::unexpected<Error>("f885c8e1-c749-4c8c-aa30-7a95d2638cf2"_log());

            return dst;
        }

}


////////////////////////////////////////////////////////////////////////////////
// Macros
////////////////////////////////////////////////////////////////////////////////


/** This macro can be used to break out of a breakable scope
    in case the variable 'retVal' is a Status that indicates failure.

    Example:

    Status retVal;
    ...
    do
    {
        ...
        retVal = foo(); // foo() returns a Status
        BreakOnFail;
        ...
    }
    while(false);

    if ( !retVal )
        error handling

    cleanup
*/
#define BreakOnFail \
    {if (!retVal) break; else ((void)0);}

#define BreakOnFail2(RETVAL) \
    {if (!RETVAL) break; else ((void)0);}


/** This macro can be used to set a Status object with the identifier retVal
    to a fail-value and break out of the innermost breakable scope.

    Example:

    Status retVal;
    ...
    do
    {
        ...
        if ( bar()!=SUCCESS_VALUE )
            FailBBREAK;
        else        // this will work due to the 'else' in the macro
            ...
        ...
    }
    while(false);

    if ( !retVal )
        error handling

    cleanup
*/
#define FailBreak(uuid)                                                         \
    {                                                                           \
        if (true)                                                               \
        {                                                                       \
            retVal << ::tl::unexpected<::nsBase::Error>{::nsBase::Log{uuid}};   \
            break;                                                              \
        }                                                                       \
        else                                                                    \
        {                                                                       \
            ((void)0);                                                          \
        }                                                                       \
    }

#define FailRETURN(uuid)                                                        \
    {                                                                           \
        if (true)                                                               \
        {                                                                       \
            retVal << ::tl::unexpected<::nsBase::Error>{::nsBase::Log{uuid}};   \
            return;                                                             \
        }                                                                       \
        else                                                                    \
        {                                                                       \
            ((void)0);                                                          \
        }                                                                       \
    }


/** Additional to FailBreak() this macro allows to specify the name of the
    variable that has to be changed to a fail state.
*/
#define FailBreak2(uuid,RETVAL)                                                 \
    {                                                                           \
        if (true)                                                               \
        {                                                                       \
            (RETVAL) << ::tl::unexpected<::nsBase::Error>{::nsBase::Log{uuid}}; \
            break;                                                              \
        }                                                                       \
        else                                                                    \
        {                                                                       \
            ((void)0);                                                          \
        }                                                                       \
    }


/** Additional to FailBreak2(RETVAL) this macro allows to specify the status
    code that gets set at the given variable.
*/
#define FailBreak3(uuid,RETVAL,CODE)                                            \
    {                                                                           \
        if (true)                                                               \
        {                                                                       \
            (RETVAL) << ::tl::unexpected<::nsBase::Error>{::nsBase::Log{uuid}   \
                                ("code",::std::to_string(CODE))             \
                            };                                                  \
                );                                                              \
            break;                                                              \
        }                                                                       \
        else                                                                    \
        {                                                                       \
            ((void)0);                                                          \
        }                                                                       \
    }


/**
    Compose an instance of StatusInfo which is ok() in case a given expression
    evaluates to TRUE.
    If the expression evaluates to FALSE, the returned StatusInfo object indicates
    an error.

    \param  uuid                    Identifier of the check.
    \param  expectedExpression      The expression to be checked.
    \param  message                 Additional message text.
*/
inline ::nsBase::StatusInfo
    CHECK(
            ::uuids::uuid id
        ,   bool          expectedExpression
        )
        {
            if (expectedExpression)
                return {};

            return ::tl::unexpected<::nsBase::Error>{::nsBase::Log{id}};
        }


/**
    Compose an instance of StatusInfo which is ok() in case a given expression
    evaluates to TRUE.
    If the expression evaluates to FALSE, the returned StatusInfo object indicates
    an error.

    \param  uuid                    Identifier of the check.
    \param  expectedExpression      The expression to be checked.
    \param  message                 Additional message text.
*/
inline ::nsBase::StatusInfo
    CHECK2(
            ::uuids::uuid         id
        ,   bool                  expectedExpression
        ,   ::std::string const & msg
        )
        {
            if (expectedExpression)
                return {};

            return ::tl::unexpected<::nsBase::Error>{::nsBase::Log{id}(msg).move()};
        }


/**
    Mark code that is noy yet implemented.
    If that code is executed, an error is thrown.
    \param id Creator-id.
*/
inline void
    NYI(::uuids::uuid id)
        {
           ::nsBase::Log{id}("Feature not implemented").critical().throw_error();
        }


inline ::nsBase::StatusInfo
    Failure(::uuids::uuid id)
        {
            return ::tl::unexpected<::nsBase::Error>{::nsBase::Log{id}};
        }

inline ::nsBase::StatusInfo
    Failure(::uuids::uuid id, ::std::string const & msg)
        {
            return ::tl::unexpected<::nsBase::Error>{::nsBase::Log{id}(msg).move()};
        }

inline bool
    fail(::nsBase::StatusInfo const & s)
        {
            return !bool(s);
        }

inline bool
    failWin(::nsBase::StatusInfo const & s)
        {
            return !bool(s);
        }
