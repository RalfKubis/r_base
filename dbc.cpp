/* Copyright (C) Ralf Kubis */

#include "r_base/dbc.h"
#include "r_base/Log.h"

#ifdef WIN32
#include <windows.h>
#endif


using namespace ::std::string_literals;

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



namespace
{
bool
AskBreak(
    char const    * inMessageBoxTitle
,   const char    * inExpression
,   const char    * inFile
,   unsigned int    inLine
,   char const    * inFunction
)
{
#ifdef WIN32

    char buf[1024];

    sprintf(
            buf
        ,   "exp : %s\n"
            "func: %s\n"
            "line: %u\n"
            "file: %s\n"
        ,   inExpression     ==0 ? "" : inExpression
        ,   inFunction       ==0 ? "" : inFunction
        ,   inLine
        ,   inFile           ==0 ? "" : inFile
        );

    int nCode = MessageBoxA(
            0
        ,   buf
        ,   inMessageBoxTitle
        ,   MB_SYSTEMMODAL|MB_ICONHAND|MB_OKCANCEL|MB_SETFOREGROUND
        );

    return (nCode == IDCANCEL);
#else
    return false;
#endif
}
}


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

    char const * type = nullptr;

    switch (inFailType)
    {
    case DBC_Event::PRE_FAIL    : type = "PRE-CONDITION FAIL"; break;
    case DBC_Event::POST_FAIL   : type = "POST-CONDITION FAIL"; break;
    case DBC_Event::ASSERT_FAIL : type = "ASSERT FAIL"; break;
    default                     : type = "<unknown event>";
    }


    nsBase::Log("5e3806a1-4e59-4341-abb9-0fdb994d7239"_uuid)
        .critical()
        .event("7d3b9ce3-abbd-41f6-b1bb-827ba0bcbaab"_uuid)
        .att(u8"dbc_type"s,type)
        .code_file(inFile?inFile:"")
        .code_line(inLine)
        .code_function(inFunction?inFunction:"")
        .code_expression(inExpression?inExpression:"")
        .message(inEventDescription?inEventDescription:"")
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


#if 0
#pragma comment( linker, "/DEFAULTLIB:Winmm.lib" )


static auto
    beep = false;


void
beepThread()
{
    do
    {
        Sleep(1);

        if ( beep )
        {
            ::std::string
                fn( "c:/home/rkubis/dev/klick.wav" );

            // stop async playing sound
            PlaySoundA(
                    fn.c_str() // pszSound
                ,   0
                ,   0
                );

            PlaySoundA(
                    fn.c_str()
                ,   0
                ,   0

                    // The sound is played asynchronously and PlaySound returns
                    //  immediately after beginning the sound. To terminate an
                    //  asynchronously played waveform sound, call PlaySound with
                    //  pszSound set to NULL.
                |   SND_ASYNC // 0 // SND_ASYNC // (mTickStyle==1?SND_ASYNC:0)
                    // The pszSound parameter is a file name. If the file cannot be
                    //  found, the function plays the default sound unless the
                    //  SND_NODEFAULT flag is set.
                |   SND_FILENAME
                    // If the driver is busy, return immediately without playing the sound.
                |   SND_NOWAIT // this doesnt work either
                    // No default sound event is used
                |   SND_NODEFAULT
            );

            beep = false;
        }
    }
    while(true);
}


void
dbgTick()
{
    beep = true;
    ::std::cout << "." << ::std::flush;
}


    new ::std::thread(beepThread);

#endif

////////////////////////////////////////////////////////////////////////////////
// !!!!! Windows Vista doesnt interrupt previous sounds
void
dbgTick(
   unsigned int const inStyle
)
{

    // play new sound
    PlaySound(
            "c:\\Windows\\media\\Windows Information Bar.wav"
        ,   0
        ,
                // The sound is played asynchronously and PlaySound returns
                //  immediately after beginning the sound. To terminate an
                //  asynchronously played waveform sound, call PlaySound with
                //  pszSound set to NULL.
                SND_ASYNC
                // The pszSound parameter is a file name. If the file cannot be
                //  found, the function plays the default sound unless the
                //  SND_NODEFAULT flag is set.
            |   SND_FILENAME
                // If the driver is busy, return immediately without playing the sound.
            |   SND_NOWAIT // this doesnt work either
                // No default sound event is used
            |   SND_NODEFAULT
        );
}
