/* Copyright (C) Ralf Kubis */

#include "r_base/debug.h"

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


namespace nsBase::debug
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


#ifdef WIN32
#pragma comment(linker, "/DEFAULTLIB:Winmm.lib")
#endif

static ::std::atomic<bool>
    beep {false};


static void
tick_executor()
{
    do
    {
        ::std::this_thread::sleep_for(1ms);

        if (beep)
        {
            auto fn = X_SOURCE_ROOT "/.wav/tick.wav";

#ifdef WIN32
            // stop async playing sound
            PlaySoundA(
                    fn // pszSound
                ,   0
                ,   0
                );

            PlaySoundA(
                    fn
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
#endif

            beep = false;
        }
    }
    while(true);
}


static ::std::thread &
tick_thread()
{
    static ::std::thread t{tick_executor};
    return t;
}


void
tick()
{
    tick_thread();

    beep = true;
    ::fmt::print(".");
    ::std::fflush(stdout);
}

}
