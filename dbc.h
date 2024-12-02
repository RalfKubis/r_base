#pragma once
/* Copyright (C) Ralf Kubis */


#include "r_base/decl.h"


////////////////////////////////////////////////////////////////////////////////
//  DBC_ENABLE (0|1) to compile run-time contract-checks
#define DBC_ENABLE 1

////////////////////////////////////////////////////////////////////////////////
//  In case activate run-time contract-checks are enabled (DBC_ENABLE is 1)
extern  bool
    dbc_enable;

extern  bool
    dbc_handleEvents;


enum class
    DBC_Event
        {
            PRE_FAIL
        ,   POST_FAIL
        ,   ASSERT_FAIL
        };

typedef void
    (*dbc_callback_type)(
            DBC_Event       inFailType
        ,   char const    * inExpression
        ,   char const    * inFile
        ,   unsigned int    inLine
        ,   char const    * inFunction
        ,   char const    * inEventDescription
        );



/// Get callback
extern  dbc_callback_type
    dbc_callback();

/// Set callback
extern  void
    dbc_callbackSet(
            dbc_callback_type inCallback
        );


////////////////////////////////////////////////////////////////////////////////
#if DBC_ENABLE

    // DbC on

    extern  void
        dbc_event(
                DBC_Event       inFailType
            ,   char const    * inExpression
            ,   char const    * inFile
            ,   unsigned int    inLine
            ,   char const    * inFunction
            ,   char const    * inEventDescription
            );

    extern  bool
        dbc_fail(
                bool      const inValue
            ,   DBC_Event       inFailType
            ,   char const    * inExpression
            ,   char const    * inFile
            ,   unsigned int    inLine
            ,   char const    * inFunction
            ,   char const    * inEventDescription
            );

    #define DBC_POST( EXP )                                                     \
        {                                                                       \
            if(!bool(EXP))                                                      \
            {                                                                   \
                dbc_event(                                                      \
                        DBC_Event::POST_FAIL                                    \
                    ,   #EXP                                                    \
                    ,   __FILE__                                                \
                    ,   __LINE__                                                \
                    ,   __FUNCTION__                                            \
                    ,   {}                                                      \
                    );                                                          \
            }                                                                   \
        }

    #define DBC_PRE( EXP )                                                      \
        {                                                                       \
            if(!bool(EXP))                                                      \
            {                                                                   \
                dbc_event(                                                      \
                        DBC_Event::PRE_FAIL                                     \
                    ,   #EXP                                                    \
                    ,   __FILE__                                                \
                    ,   __LINE__                                                \
                    ,   __FUNCTION__                                            \
                    ,   {}                                                      \
                    );                                                          \
            }                                                                   \
        }

    #define DBC_ASSERT( EXP )                                                   \
        {                                                                       \
            if(!bool(EXP))                                                      \
            {                                                                   \
                dbc_event(                                                      \
                        DBC_Event::ASSERT_FAIL                                  \
                    ,   #EXP                                                    \
                    ,   __FILE__                                                \
                    ,   __LINE__                                                \
                    ,   __FUNCTION__                                            \
                    ,   {}                                                      \
                    );                                                          \
            }                                                                   \
        }


    #define DBC_FAIL( EXP )                                                     \
        dbc_fail(                                                               \
                bool(EXP)                                                       \
            ,   DBC_Event::ASSERT_FAIL                                          \
            ,   #EXP                                                            \
            ,   __FILE__                                                        \
            ,   __LINE__                                                        \
            ,   __FUNCTION__                                                    \
            ,   {}                                                              \
            )

    #define DBC_GOOD( EXP )  !DBC_FAIL( EXP )


////////////////////////////////////////////////////////////////////////////////
#else

// DbC off

#define DBC_PRE( e )

#define DBC_POST( e )

#define DBC_ASSERT( e )         ((void)0)

#define DBC_ASSERT2( e, m )     ((void)0)

#define DBC_FAIL( EXP ) (!(EXP))

#define DBC_GOOD( EXP )  (EXP)

#endif
