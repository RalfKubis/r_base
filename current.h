#pragma once
/* Copyright (C) Ralf Kubis */

#include "r_base/uuid.h"
#include "r_base/time.h"

#include <string>
#include <string_view>


namespace nsBase::current
{

/** \name application info
@{*/
auto application_id_assign(::uuids::uuid const &)           -> void;
auto application_id()                                       -> ::uuids::uuid;

auto application_instance_id_assign(::uuids::uuid const &)  -> void;
auto application_instance_id()                              -> ::uuids::uuid;

auto application_name_assign(::std::string_view)            -> void;
auto application_name()                                     -> ::std::string const &;

auto application_version_assign(::std::string_view)         -> void;
auto application_version()                                  -> ::std::string const &;

auto application_build_time_assign(::std::string_view)      -> void;
auto application_build_time()                               -> ::std::string const &;

auto application_git_commit_id_assign(::std::string_view)   -> void;
auto application_git_commit_id()                            -> ::std::string const &;


void
    application_bulk_assign(
            ::uuids::uuid const & application_id
        ,   ::std::string_view    application_name
        ,   ::std::string_view    application_version
        ,   ::std::string_view    application_build_time
        ,   ::std::string_view    application_git_commit_id
        );
//@}


/** \name threads session id (sessions can migrate to another thread)
@{*/
auto thread_session_id_assign(::uuids::uuid const &) -> void;
auto thread_session_id() -> ::uuids::uuid;
//@}


auto thread() -> ::std::string;
auto time() -> ::nsBase::time::time_point_t;
auto user() -> ::std::string;
auto host() -> ::std::string;

/** Return TRUE if called in the main thread.
*/
auto is_main_thread() -> bool;

}
