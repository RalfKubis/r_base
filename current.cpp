/* Copyright (C) Ralf Kubis */
#include "r_base/current.h"

#include "r_base/string.h"
#include "r_base/time.h"
#include "r_base/utf.h"
#include "r_base/thread.h"
#include "r_base/Log.h"

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
#include <unistd.h>
#include <limits.h>
#include <pwd.h>
#endif

#ifdef _WIN32
#include <Windows.h>
#include <Lmcons.h>
#endif


namespace nsBase::current
{
using namespace ::std::string_literals;
using namespace ::uuids;


static ::std::mutex &
obtain_mutex()
{
    static ::std::atomic<::std::mutex*>
        mutex{{}};

    if (!mutex.load())
    {
        ::std::mutex *
            null {};

        auto m = ::std::make_unique<::std::mutex>();

        if (mutex.compare_exchange_strong(null, m.get()))
            m.release();
    }

    return *mutex.load();
}


static ::std::optional<::std::string> &
    user_()
        {
            static ::std::optional<::std::string> *
                current_user {};

            ::std::unique_lock<::std::mutex>
                guard(obtain_mutex());

            if (!current_user)
                current_user = new ::std::optional<::std::string>;

            return *current_user;
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
                user_().emplace("?"s);
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

#ifdef _WIN32

        ::std::wstring
            buf;
            buf.resize(1024);

        DWORD
            bufSize = buf.size();

        if (    ::GetUserNameW(buf.data(), &bufSize)
            &&  bufSize>0
        )
        {
            buf.resize(bufSize-1);
            user_().emplace(wstring_to_utf8(buf));
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
                current_host {};

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

#ifdef _WIN32

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
    copy(::std::uint8_t(&src)[16], ::uuids::uuid & dst)
        {
            dst = ::uuids::uuid{src};
        }

void
    copy(::uuids::uuid const & src, ::std::uint8_t dst[])
        {
            auto span = src.as_bytes();

            for (int i=0 ; i<16 ; i++)
                dst[i] = static_cast<::std::uint8_t>(span[i]);
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


////////////////////////////////////////////////////////////////////////////////
static ::uuids::uuid &
application_id_storage()
{
    static ::uuids::uuid
        id;

    return id;
}


void
application_id_assign(
    ::uuids::uuid const & id
)
{
    application_id_storage() = id;
}


::uuids::uuid
application_id()
{
    return application_id_storage();
}


////////////////////////////////////////////////////////////////////////////////
static ::uuids::uuid &
application_instance_id_storage()
{
    static ::uuids::uuid
        id;

    return id;
}


void
application_instance_id_assign(
    ::uuids::uuid const & id
)
{
    application_instance_id_storage() = id;
}


::uuids::uuid
application_instance_id()
{
    return application_instance_id_storage();
}


////////////////////////////////////////////////////////////////////////////////
static ::std::string &
application_name_storage()
{
    static ::std::string
        s;

    return s;
}


void
application_name_assign(
    ::std::string_view s
)
{
    application_name_storage() = s;
}


::std::string const &
application_name()
{
    return application_name_storage();
}


////////////////////////////////////////////////////////////////////////////////
static ::std::string &
application_version_storage()
{
    static ::std::string
        s;

    return s;
}


void
application_version_assign(
    ::std::string_view s
)
{
    application_version_storage() = s;
}


::std::string const &
application_version()
{
    return application_version_storage();
}


////////////////////////////////////////////////////////////////////////////////
static ::std::string &
application_build_time_storage()
{
    static ::std::string
        s;

    return s;
}


void
application_build_time_assign(
    ::std::string_view s
)
{
    application_build_time_storage() = s;
}


::std::string const &
application_build_time()
{
    return application_build_time_storage();
}


////////////////////////////////////////////////////////////////////////////////
static ::std::string &
application_git_commit_id_storage()
{
    static ::std::string
        s;

    return s;
}


void
application_git_commit_id_assign(
    ::std::string_view s
)
{
    application_git_commit_id_storage() = s;
}


::std::string const &
application_git_commit_id()
{
    return application_git_commit_id_storage();
}


////////////////////////////////////////////////////////////////////////////////
void
application_bulk_assign(
    ::uuids::uuid const & application_id
,   ::std::string_view    application_name
,   ::std::string_view    application_version
,   ::std::string_view    application_build_time
,   ::std::string_view    application_git_commit_id
)
{
    // <issue#4018> stdout/stderr runs full if no console is attached -> route to "NUL", which is the WIN equivalent to "/dev/null"
#ifdef _WIN32
    if (!::GetStdHandle(STD_OUTPUT_HANDLE)) if (!::std::freopen("NUL", "w", stdout)) "1edca77b-21d8-498a-8518-4cdb36216e4f"_log["r_base/current"].throw_INTERNAL();
    if (!::GetStdHandle(STD_ERROR_HANDLE )) if (!::std::freopen("NUL", "w", stderr)) "f16866a4-9441-4fce-95a9-460af89a80f5"_log["r_base/current"].throw_INTERNAL();
#endif

    current::application_id_assign(             application_id);
    current::application_instance_id_assign(    ::uuids::uuid_system_generator{}());
    current::application_name_assign(           application_name);
    current::application_version_assign(        application_version);
    current::application_build_time_assign(     application_build_time);
    current::application_git_commit_id_assign(  application_git_commit_id);

    // <issue#3043> gRPC DNS resolution fail
    putenv("GRPC_DNS_RESOLVER=native");

    ::nsBase::thread::set_thread_name("main");
}


////////////////////////////////////////////////////////////////////////////////
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


static auto
    main_thread_id = ::std::this_thread::get_id();

bool
is_main_thread()
{
    return ::std::this_thread::get_id() == main_thread_id;
}

}
