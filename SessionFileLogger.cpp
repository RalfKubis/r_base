// Copyright (C) Ralf Kubis

#include "r_base/SessionFileLogger.h"

#include <mutex>
#include <atomic>
#include <memory>

using namespace ::nsBase;

namespace nsBase
{

namespace
{
std::unique_lock<::std::recursive_mutex>
    lock()
        {
            static ::std::atomic<::std::recursive_mutex*>
                mutex(nullptr);

            if (mutex.load()==nullptr)
            {
                ::std::recursive_mutex *
                    null = nullptr;

                auto m = ::std::make_unique<::std::recursive_mutex>();

                if (mutex.compare_exchange_strong(null, m.get()))
                    m.release();
            }

            return ::std::unique_lock<::std::recursive_mutex>(*mutex.load());
        }
}


SessionFileLogger::~SessionFileLogger()
{
    if (log_file())
        ::std::fclose(log_file());
}


SessionFileLogger::SessionFileLogger(
    SessionFileLogger && rhs
)
{
    *this = ::std::move(rhs);
}


SessionFileLogger &
SessionFileLogger::operator=(
    SessionFileLogger && rhs
)
{
    if (this != &rhs)
    {
        auto guard = lock();

        session_assign(rhs.session());
        log_dir_path_assign(rhs.log_dir_path());

        ::std::swap(*m_log_file, *rhs.m_log_file);
    }
    return *this;
}


namespace
{
fs::path
    eff_path(
            SessionFileLogger const & l
        )
        {
            auto
                non_digit_to_underscore = [](::std::string const & s)
                    {
                        auto res = s;

                        for (auto & c : res)
                        {
                            if (c<'0' || c>'9')
                                c = '-';
                        }

                        return res;
                    };

            auto
                path  = l.log_dir_path();
                path /= non_digit_to_underscore(l.time()) + "." + to_string(l.session()) + '.' + l.extension();

            return ::fs::absolute(path);
        }
}


void
SessionFileLogger::operator()(
    Log & log
)
{
    if (log.session()!=session())
        return;

    auto
        guard = lock();

    if (log_file_path().empty())
    {
        if (time().empty())
            time_assign(log.time_as_string());

        log_file_path_assign(eff_path(*this));

        auto
            log_dir_abs = log_file_path().parent_path();
        if (!log_dir_abs.empty() && !::fs::exists(log_dir_abs))
            ::fs::create_directories(log_dir_abs);

        log_file_assign(::std::fopen(log_file_path().string().c_str(), "ab"));
    }

    if (!log_file())
        return;

    auto
        line = log.serialize(Log::Format::JSON) + "\n";

    ::std::fwrite(line.c_str(), line.size(), 1, log_file());
    ::std::fflush(log_file());
}


void
SessionFileLogger::rename_if()
{
    auto
        guard = lock();

    auto
        new_path = eff_path(*this);

    if (new_path==log_file_path())
        return;

    if (log_file())
    {
        ::std::fclose(log_file());
        log_file_clear();

        ::std::error_code
            err;

        ::fs::rename(
                log_file_path()
            ,   new_path
            ,   err
            );
    }

    log_file_path_clear();
}


}
