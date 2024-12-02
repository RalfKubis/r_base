#include "r_base/log_consumer_console.h"

#include "r_base/time.h"

#include <fmt/format.h>

#include <cstdlib>

#ifdef _WIN32
#include <Windows.h>
#include <winuser.h>
#endif


namespace nsBase
{
using namespace ::std::string_literals;


bool
    log_consumer_console_be_verbose {true};

::std::optional<bool>
    log_consumer_console_dump_in_local_time;

::std::optional<bool>
    log_consumer_console_dump_date;


namespace
{

struct
ConsoleTweaker
{
    ConsoleTweaker()
    {
        // requirement.74c1daf7-94be-417a-bea0-a9ec90d64f71
        #ifdef _WIN32
        if (::GetConsoleWindow()) // only if the app has a console
        {
            if (auto ok = ::SetConsoleCP(65001); !ok)
            {
                auto ec = ::GetLastError();
                auto v  = ::IsValidCodePage(65001);
                auto w  = ::GetConsoleWindow();

                auto msg = ::std::format("[19346fb1-0d5a-424a-b228-a4568783ca42] failed to set code page 65001 (ec:{} v:{} w:{})", ec, v, w!=nullptr);

                MessageBox(
                        {}          // [in, optional] HWND    hWnd
                    ,   msg.c_str() // [in, optional] LPCTSTR lpText
                    ,   "error"     // [in, optional] LPCTSTR lpCaption
                    ,   MB_OK       // [in]           UINT    uType
                    );
            }
        }
        ::std::setlocale(LC_ALL, "en_US.UTF-8");
        #endif
    }
};


struct
Initializer
{
    Initializer()
    {
        if (!log_consumer_console_dump_in_local_time.has_value())
        {
            auto s = ::std::getenv("log_in_local_time");
            log_consumer_console_dump_in_local_time = s ? ::std::string_view{s} == "1" :
                #ifdef NDEBUG
                    false
                #else
                    true
                #endif
                    ;
        }

        if (!log_consumer_console_dump_date.has_value())
        {
            auto s = ::std::getenv("log_date");
            log_consumer_console_dump_date = s ? ::std::string_view{s} == "1" :
                #ifdef NDEBUG
                    true
                #else
                    false
                #endif
                    ;
        }
    }
};

} // ns


void
log_consumer_console(
    Log & log
)
{
    static ConsoleTweaker
        console_tweaker;

    static Initializer
        initializer;


    auto level = log.level();

    if (level>=Log::Level::FAILURE)
        ::fmt::print(""); // put your breakpoint here to break on failures

    if (   !log_consumer_console_be_verbose
        &&  level<Log::Level::INFO
    )
        return;

    //  fetch relevant attributes
    auto att_message = log.message_resolved();

    if (att_message.empty())
        att_message = "no message"s;

    //  console output
    auto text = ""s;

    if (log_consumer_console_be_verbose)
        text += "{"s + to_string8_with_empty_to_NIL(log.creator()) + "} "s;

    auto as_utc    = !log_consumer_console_dump_in_local_time.value_or(false);
    auto with_date =  log_consumer_console_dump_date.value_or(true);

    text += to_string(
            log.time()
        ,   as_utc
        ,   true // with_millis
        ,   with_date ? (as_utc ? "%Y-%m-%dT%H:%M:%S" : "%Y-%m-%d %H:%M:%S") : "%H:%M:%S" // format
        ,   true // with_micros
        );

    if (as_utc)
        text += "Z";

    // the 'debug' level is printed as empty string to make it easier to percept the other levels in the output
    auto lvl = to_lower(level==Log::Level::DEBUG ? ""s : to_string(level));
    auto lvl_padding_size = 8 - lvl.size();

    text += " " + lvl + ::std::string(lvl_padding_size, ' ') + ": ";

    if (auto & s = log.scope(); !s.empty())
        text += "["s + s + "] "s;

    text += att_message;

    // requirement.74c1daf7-94be-417a-bea0-a9ec90d64f71
    auto fd = level<Log::Level::FAILURE ? stdout : stderr;
    ::fmt::print(fd, "{}\n", text);
    ::std::fflush(fd);
}

}
