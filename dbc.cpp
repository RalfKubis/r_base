/* Copyright (C) Ralf Kubis */


#include "r_base/dbc.h"
#include "r_base/Log.h"

#ifdef WIN32
#include <windows.h>
#include <Mmsystem.h>
#endif

#include <chrono>
#include <thread>
#include <atomic>

#include <fmt/format.h>


using namespace ::std::string_literals;
using namespace ::std::chrono_literals;

bool
    dbc_handleEvents = true;


namespace
{
dbc_callback_type
    sDbC_callback = 0;
}


dbc_callback_type
dbc_callback()
{
    return sDbC_callback;
}


void
dbc_callbackSet(
    dbc_callback_type    inCallback
)
{
    sDbC_callback = inCallback;
}



#if DBC_ENABLE

bool
dbc_fail(
    bool      const inValue
,   DBC_Event       inFailType
,   char const    * inExpression
,   char const    * inFile
,   unsigned int    inLine
,   char const    * inFunction
,   char const    * inEventDescription
)
{
    if ( !inValue )
    {
        dbc_event(
                inFailType
            ,   inExpression
            ,   inFile
            ,   inLine
            ,   inFunction
            ,   inEventDescription
            );
    }

    return !inValue;
}


void
dbc_event(
    DBC_Event       inFailType
,   char const    * inExpression
,   char const    * inFile
,   unsigned int    inLine
,   char const    * inFunction
,   char const    * inEventDescription
)
{
    if (!dbc_handleEvents)
        return;

    if (sDbC_callback)
    {
        sDbC_callback(
                inFailType
            ,   inExpression
            ,   inFile
            ,   inLine
            ,   inFunction
            ,   inEventDescription
            );
    }

    char const * type {};

    switch (inFailType)
    {
    case DBC_Event::PRE_FAIL    : type = "PRE-CONDITION FAIL"; break;
    case DBC_Event::POST_FAIL   : type = "POST-CONDITION FAIL"; break;
    case DBC_Event::ASSERT_FAIL : type = "ASSERT FAIL"; break;
    default                     : type = "<unknown event>";
    }


    "5e3806a1-4e59-4341-abb9-0fdb994d7239"_log
        (inEventDescription?inEventDescription:"")
        ("dbc_type"s,type)
        .critical()
        .event("7d3b9ce3-abbd-41f6-b1bb-827ba0bcbaab"_uuid)
        .code_file(inFile?inFile:"")
        .code_line(inLine)
        .code_function(inFunction?inFunction:"")
        .code_expression(inExpression?inExpression:"")
        ;

#if 0 & !defined(WIN64)

    // DbC_handleEvents may have been unset as side-effect of sDbC_callback.
    // It is probably safe to assume that if the user pressed ignore in the DbC message,
    // he does not want to debug.
    if ( DbC_handleEvents && IsDebuggerAttached() )
        __asm int 3h

#endif

    //AskBreak(
    //        typeS
    //    ,   inExpression
    //    ,   inFile
    //    ,   inLine
    //    ,   inFunction
    //    );
}

#endif
