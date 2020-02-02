#include <csignal>
#include <string>
#include <map>

#include "r_base/Log.h"
#include "r_base/uuid.h"


namespace nsBase
{
using namespace ::nsBase;
using namespace ::std::string_literals;
using namespace uuids;

namespace
{
using t_handler = void(*)(int);

std::map<int,t_handler>
    default_handlers;

uuid
    signal_to_uuid(
            int sig
        )
        {
            switch(sig)
            {
            case SIGINT  : return "cdf20e3c-2d8b-4698-b4a0-c96953d996de"_uuid;
            case SIGILL  : return "b267efaf-4a5b-4131-9fb7-f91cdcd08eb5"_uuid;
            case SIGFPE  : return "12030a7d-13df-47dd-87a6-a94a27b226e1"_uuid;
            case SIGSEGV : return "d7890fc0-0013-452d-9dbc-7dcc12e5eb13"_uuid;
            case SIGTERM : return "40bca240-47e8-4abe-afed-815dbee78b87"_uuid;
#ifdef SIGBREAK
            case SIGBREAK: return "10cbf4a8-95e0-4ed7-95c1-a53eb896d699"_uuid;
#endif
            case SIGABRT : return "fcf602d6-44ea-404a-abf2-becb86d5f243"_uuid;
            default      : return "9f95977e-8192-4cda-a935-898e7db679cf"_uuid;
            }
        }

std::string
    signal_to_string(
            int sig
        )
        {
            switch(sig)
            {
            case SIGINT  : return "SIGINT"s;
            case SIGILL  : return "SIGILL"s;
            case SIGFPE  : return "SIGFPE"s;
            case SIGSEGV : return "SIGSEGV"s;
            case SIGTERM : return "SIGTERM"s;
#ifdef SIGBREAK
            case SIGBREAK: return "SIGBREAK"s;
#endif
            case SIGABRT : return "SIGABRT"s;
            default      : return "<unknown>"s;
            }
        }

std::string
    signal_to_description(
            int sig
        )
        {
            switch(sig)
            {
            case SIGINT  : return "interrupt"s;
            case SIGILL  : return "illegal instruction - invalid function image"s;
            case SIGFPE  : return "floating point exception"s;
            case SIGSEGV : return "segment violation"s;
            case SIGTERM : return "Software termination signal from kill"s;
#ifdef SIGBREAK
            case SIGBREAK: return "Ctrl-Break sequence"s;
#endif
            case SIGABRT : return "abnormal termination triggered by abort call"s;
            default      : return "<unknown>"s;
            }
        }
}


void
signal_handler(
    int signal
)
{
    Log("c3c67049-cef2-4ba8-b9c3-4f8aa761688d"_uuid)
        .critical()
        .event(signal_to_uuid(signal))
        .message("Received signal [${data}] - ${description}")
        .data(signal_to_string(signal))
        .att("description",signal_to_description(signal))
        ;

    // invoke the default handler
    auto it = default_handlers.find(signal);

    if (it!=default_handlers.end())
    {
        if (it->second)
            it->second(signal);
    }
    else
    {
        ::std::signal(signal, SIG_DFL);
        ::std::raise(signal);
    }
}


void
install_signal_handler()
{
    auto
        add = [](
                int sig
            )
            {
                if(auto old = ::std::signal(sig, signal_handler))
                {
                    default_handlers[sig] = old;
                }
            };

    add(SIGINT  ); // interrupt
    add(SIGILL  ); // illegal instruction - invalid function image
    add(SIGFPE  ); // floating point exception
    add(SIGSEGV ); // segment violation
    add(SIGTERM ); // Software termination signal from kill
#ifdef SIGBREAK
    add(SIGBREAK); // Ctrl-Break sequence
#endif
    add(SIGABRT ); // abnormal termination triggered by abort call
}

}
