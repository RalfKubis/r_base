/* Copyright (C) Ralf Kubis */
#include "r_base/current.h"

#include "r_base/string.h"
#include "r_base/time.h"

#include <atomic>
#include <optional>
#include <mutex>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <memory>
#include <iomanip>
#include <thread>
#include <string>

#include <fmt/format.h>

#if defined __linux__
#include "unix-common/my_unistd.h"
#include <limits.h>
#include <pwd.h>
#endif

#if defined win_x86_32
#include <Windows.h>
#include <Lmcons.h>
#endif


namespace nsBase{namespace current
{

using namespace ::std::string_literals;
using namespace ::uuids;


namespace
{

::std::mutex &
obtain_mutex()
{
    static ::std::atomic<::std::mutex*>
        mutex{nullptr};

    if (mutex.load()==nullptr)
    {
        ::std::mutex *
            null = nullptr;

        auto m = ::std::make_unique<::std::mutex>();

        if (mutex.compare_exchange_strong(null, m.get()))
            m.release();
    }

    return *mutex.load();
}


::std::optional<::std::string> &
    user_()
        {
            static ::std::optional<::std::string> *
                current_user = nullptr;

            ::std::unique_lock<::std::mutex>
                guard(obtain_mutex());

            if (!current_user)
                current_user = new ::std::optional<::std::string>;

            return *current_user;
        }

}

::std::string
user()
{

    if (!user_())
    {
#if defined __linux__

#ifndef LOGIN_NAME_MAX
#define LOGIN_NAME_MAX 256
#endif

        // user logged in on the controlling terminal of the process
        if (false)
        {
            char buf[LOGIN_NAME_MAX];

            if (getlogin_r(buf,LOGIN_NAME_MAX)==0)
                user_().emplace(buf);
            else
                user_().emplace(u8"?"s);
        }

        // username associated with the effective user ID of the process
        if (false)
        {
            // problem: L_cuserid is too small and unames get truncated
            // got 9 on tara
            char buf[L_cuserid+1];

            if (auto uid=cuserid(nullptr))
                user_().emplace(uid);
        }

        if (true)
        {
            auto
                uid = geteuid();
            passwd
                pw_buffer{};
            ::std::array<char,200>
                buffer;
            passwd *
                pw_result{};

            getpwuid_r(
                    uid
                ,   &pw_buffer
                ,   buffer.data()
                ,   buffer.size()
                ,   &pw_result
                );

            if (pw_result && pw_result->pw_name)
                user_().emplace(pw_result->pw_name);
        }
#endif

#if defined win_x86_32

        ::std::array<char,UNLEN+1>
            buf;
            buf.fill(0);

        DWORD
            bufSize = buf.size();

        if (    ::GetUserNameA(buf.data(), &bufSize)
            &&  bufSize>0
        )
        {
            user_().emplace(buf.data(),bufSize-1);
        }

#endif
    }

    if (!user_())
        user_().emplace();

    return *user_();
}



namespace
{
::std::optional<::std::string> &
    host_()
        {
            static ::std::optional<::std::string> *
                current_host = nullptr;

            ::std::unique_lock<::std::mutex>
                guard(obtain_mutex());

            if (!current_host)
                current_host = new ::std::optional<::std::string>;

            return *current_host;
        }
}


::std::string
host()
{
    if (!host_())
    {
#if defined __linux__

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif

        char buf[HOST_NAME_MAX];

        if (gethostname(buf, HOST_NAME_MAX)==0)
            host_().emplace(buf);

#endif

#if defined win_x86_32

        ::std::array<char,UNCLEN+1>
            buf;
            buf.fill(0);

        DWORD
            bufSize = buf.size();

        if (    ::GetComputerNameA(buf.data(), &bufSize)
            &&  bufSize>0
        )
        {
            host_().emplace(buf.data(),bufSize);
        }
#endif
    }

    if (!host_())
        host_().emplace();

    return *host_();
}


namespace
{
::std::uint8_t
    session_id_global[16];

thread_local ::std::uint8_t
    session_id_thread[16];

void
    copy(::std::uint8_t const src[], ::uuids::uuid & dst)
        {
            for (int i=0 ; i<16 ; i++)
                *(dst.begin() + i) = src[i];
        }

void
    copy(::uuids::uuid const & src, ::std::uint8_t dst[])
        {
            for (int i=0 ; i<16 ; i++)
                dst[i] = *(src.begin() + i);
        }

void
    copy(::std::uint8_t const src[], ::std::uint8_t dst[])
        {
            for (int i=0 ; i<16 ; i++)
                dst[i] = src[i];
        }

bool
    empty(::std::uint8_t const src[])
        {
            for (auto i=0 ; i<16 ; i++)
                if (src[i])
                    return false;
            return true;
        }

::uuids::uuid
    current_thread_session_id()
        {
            ::std::unique_lock<::std::mutex>
                guard(obtain_mutex());

            if (empty(session_id_global))
                copy(::uuids::uuid_system_generator()(), session_id_global);

            if (empty(session_id_thread))
                copy(session_id_global,session_id_thread);

            ::uuids::uuid
                u;

            copy(session_id_thread,u);

            return u;
        }

void
    current_thread_session_id_assign(
            ::uuids::uuid const & uid
        )
        {
            ::std::unique_lock<::std::mutex>
                guard(obtain_mutex());

            if (empty(session_id_global))
                copy(uid, session_id_global);

            copy(uid ,session_id_thread);
        }
}


void
thread_session_id_assign(
    ::uuids::uuid const & id
)
{
    current_thread_session_id_assign(id);
}


::uuids::uuid
thread_session_id()
{
    return current_thread_session_id();
}



namespace
{
::uuids::uuid &
    application_id_()
        {
            static ::uuids::uuid *
                id = nullptr;

            ::std::unique_lock<::std::mutex>
                guard(obtain_mutex());

            if (!id)
                id = new ::uuids::uuid;

            return *id;
        }
}

void
application_id_assign(
    ::uuids::uuid const & id
)
{
    application_id_() = id;
}


::uuids::uuid
application_id()
{
    return application_id_();
}




::std::string
thread()
{
    ::std::stringstream
        s;

    s << ::std::this_thread::get_id();

    return s.str();
}


::nsBase::time::time_point_t
time()
{
    return time::now();
}

}}
