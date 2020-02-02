#pragma once
/* Copyright (C) Ralf Kubis */

#include <chrono>


class Watch
{
    public :
        Watch()
            {
                start();
            }

    public : void
        start()
            {
                m_time = ::std::chrono::system_clock::now();
            }

    public : long double
        restart()
            {
                long double us = stop();
                start();
                return us;
            }

    public : auto
        duration() const
            {
                auto d = ::std::chrono::system_clock::now() - m_time;
                return d;
            }

    public : long double
        stop()
            {
                auto delta = duration();
                long double us = ::std::chrono::duration_cast<::std::chrono::duration<long double, ::std::micro>>(delta).count();
                return us;
            }

    private : ::std::chrono::time_point<::std::chrono::system_clock>
        m_time;
};
