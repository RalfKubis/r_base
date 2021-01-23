#ifdef _WIN32
#include <Windows.h>
#endif

#include "r_base/Error.h"
#include "r_base/string.h"


namespace nsBase
{

void
throw_on_error_win(
    bool              is_ok
,   ::uuids::uuid     trace_id
)
{
    if (is_ok)
        return;

#ifdef _WIN32

    // there is an error -> get WIN error message
    LPTSTR lpBuffer;

    auto errNum = ::GetLastError();

    auto n = ::FormatMessage(
                0               // __in      DWORD dwFlags
            |   FORMAT_MESSAGE_FROM_SYSTEM
            |   FORMAT_MESSAGE_ALLOCATE_BUFFER
        ,   NULL                // __in_opt  LPCVOID lpSource
        ,   errNum              // __in      DWORD dwMessageId
        ,   0                   // __in      DWORD dwLanguageId
        ,   (LPTSTR)&lpBuffer   // __out     LPTSTR lpBuffer
        ,   0                   // __in      DWORD nSize
        ,   NULL                // __in_opt  va_list *Arguments
        );

    ::std::string
        msg(lpBuffer);
    ::LocalFree(lpBuffer);

    throw Error{Log{u8"de7708f7-a526-4f1f-bcf3-7f2e68fd1da3"_uuid}
        .message("Windows API")
        .att("api_message", msg)
        };

#else

    throw Error{u8"de7708f7-a526-4f1f-bcf3-7f2e68fd1da3"_uuid, "This code should only execute on Windows."};

#endif
}

}
