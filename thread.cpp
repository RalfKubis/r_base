/* Copyright (C) Ralf Kubis */

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#include <processthreadsapi.h>
#endif

#include "r_base/thread.h"
#include "r_base/Log.h"

#include <limits>


namespace nsBase::thread
{

#ifdef WIN32
static ::std::wstring
Utf8ToUtf16(
    ::std::string_view const & utf8
)
{
    // https://docs.microsoft.com/en-us/archive/msdn-magazine/2016/september/c-unicode-encoding-conversions-with-stl-strings-and-win32-apis#converting-from-utf-8-to-utf-16-multibytetowidechar-in-action

    if (utf8.empty())
        return {};

    ::std::wstring
        utf16;

    if (utf8.length() > static_cast<size_t>(::std::numeric_limits<int>::max()))
        "a382b709-2167-4615-b6c4-e3f7646b070f"_log("Input string too long: size_t-length doesn't fit into int.").throw_error();

    // Safely convert from size_t (STL string's length)
    // to int (for Win32 APIs)
    auto const utf8Length = static_cast<int>(utf8.length());

    // Safely fails if an invalid UTF-8 character
    // is encountered in the input string
    constexpr DWORD kFlags = MB_ERR_INVALID_CHARS;

    auto const utf16Length = ::MultiByteToWideChar(
            CP_UTF8         // Source string is in UTF-8
        ,   kFlags          // Conversion flags
        ,   utf8.data()     // Source UTF-8 string pointer
        ,   utf8Length      // Length of the source UTF-8 string, in chars
        ,   {}              // Unused - no conversion done in this step
        ,   0               // Request size of destination buffer, in wchar_ts
        );

    if (utf16Length == 0)
    {
        // Conversion error
        "32cf7e02-8ed7-4d9a-8a19-0ee64ed15b58"_log("Cannot get result string length when converting from UTF-8 to UTF-16 (MultiByteToWideChar failed).")
            .data(::GetLastError())
            .throw_error();
    }

    utf16.resize(utf16Length);

    // Convert from UTF-8 to UTF-16
    auto result = ::MultiByteToWideChar(
            CP_UTF8        // Source string is in UTF-8
        ,   kFlags         // Conversion flags
        ,   utf8.data()    // Source UTF-8 string pointer
        ,   utf8Length     // Length of source UTF-8 string, in chars
        ,   &utf16[0]      // Pointer to destination buffer
        ,   utf16Length    // Size of destination buffer, in wchar_ts
        );

    if (result == 0)
    {
        // Conversion error
        "f2ea20ab-915a-4f2d-8b90-0f772ae85540"_log( "Cannot convert from UTF-8 to UTF-16 (MultiByteToWideChar failed).")
            .data(::GetLastError())
            .throw_error();
    }

    return utf16;
}
#endif


void
set_thread_name(::std::string_view const & s)
{
#ifdef WIN32
    auto ws =  Utf8ToUtf16(s);

    ::SetThreadDescription(::GetCurrentThread(), ws.data());
#else
    pthread_setname_np(pthread_self(), s.data());
#endif
}


}
