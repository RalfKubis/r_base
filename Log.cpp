/* Copyright (C) Ralf Kubis */
#include "r_base/Log.h"

#include "r_base/string.h"
#include "r_base/vector.h"
#include "r_base/Property.h"
#include "r_base/time.h"
#include "r_base/Error.h"
#include "r_base/current.h"

#include <optional>
#include <mutex>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <memory>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <thread>
#include <string>
#include <atomic>

#include <fmt/format.h>

#include <nlohmann/json/json.hpp>

#if defined __linux__
#include <unistd.h>
#include <limits.h>
#include <pwd.h>
#endif

#ifdef _WIN32
#include <Windows.h>
#include <Lmcons.h>
#endif


using namespace ::std::string_literals;

namespace
{

::std::mutex &
obtain_mutex()
{
    static ::std::atomic<::std::mutex*>
        consumer_mutex({});

    if (!consumer_mutex.load())
    {
        ::std::mutex *
            null {};

        auto m = ::std::make_unique<::std::mutex>();

        if (consumer_mutex.compare_exchange_strong(null, m.get()))
            m.release();
    }

    return *consumer_mutex.load();
}

unsigned int
    s_consumer_next_id;

using
    t_consumers = ::std::map<
            unsigned int
        ,   ::std::function<void(nsBase::Log &)>
        >;

// caller must lock mutex
t_consumers &
    obtain_consumers()
        {
            static t_consumers *
                consumers {};

            if (!consumers)
                consumers = new t_consumers;

            return *consumers;
        }
}


namespace nsBase
{

class Log_Impl
{
////////////////////////////////////////////////////////////////////////////////
/** \name Standard Attributes
@{*/
    R_PROPERTY_(
            id
        ,   ::uuids::uuid
        )

    R_PROPERTY_D(
            level
        ,   Log::Level
        ,   Log::Level::DEBUG
        )

    R_PROPERTY_(
            status
        ,   Log::Status
        )

    R_PROPERTY_(
            application
        ,   ::uuids::uuid
        )

    R_PROPERTY_(
            application_instance
        ,   ::uuids::uuid
        )

    R_PROPERTY_(
            version
        ,   ::std::string
        )

    R_PROPERTY_(
            session
        ,   ::uuids::uuid
        )

    R_PROPERTY_(
            task
        ,   ::uuids::uuid
        )

    R_PROPERTY_(
            creator
        ,   ::uuids::uuid
        )

    R_PROPERTY_(
            event
        ,   ::uuids::uuid
        )

    R_PROPERTY_(
            time
        ,   ::nsBase::time::time_point_t
        )

    R_PROPERTY_(
            host
        ,   ::std::string
        )

    R_PROPERTY_(
            user
        ,   ::std::string
        )

    R_PROPERTY_(
            thread
        ,   ::std::string
        )

    R_PROPERTY_3M(
            trace
        ,   ::std::vector<::uuids::uuid>
        )

    R_PROPERTY_3M(
            scope
        ,   ::std::string
        )

    R_PROPERTY_3M(
            message
        ,   ::std::string
        )
//@}


    R_PROPERTY_D(
            do_broadcast
        ,   bool
        ,   true
        )

    /**
        Attributes
    */
    public : ::std::map<::std::string, ::std::string>
        mAttributes;
};


Log::~Log()
{
    broadcast_if();
}


Log::Log(
    ::uuids::uuid const & creator_id
)
:   p {::std::make_unique<Log_Impl>()}
{
    if (!creator_id.is_nil())
    {
        id(::uuids::uuid_system_generator{}());
        application(current::application_id());
        application_instance(current::application_instance_id());
        session(current::thread_session_id());
        creator(creator_id);
        host(current::host());
        user(current::user());
        thread(current::thread());
    }
    else
    {
        do_broadcast_assign(false);
    }
}


Log::Log()
:   Log {::uuids::uuid{}}
{
}


Log::Log(
    Log && other
)
:   p {::std::move(other.p)}
{
}


Log::Log(
    Log const & other
)
{
    if (other.p)
        p = ::std::make_unique<Log_Impl>(*other.p); // deep copy

    do_broadcast_assign(false);
}


Log::Log(
    Log_Impl const & other
)
:   p {::std::make_unique<Log_Impl>(other)} // deep copy
{
}


Log
Log::copy() const
{
    if (!p)
        "83eacc55-9970-4c2e-bc31-5a9c369cfcce"_log().throw_error();

    return *p.get();
}


Log &&
Log::move()
{
    return ::std::move(*this);
}


void
Log::broadcast_if()
{
    if (!p)
        return;  // content was moved

    if (!do_broadcast())
        return;

    disarm();

    if (time::is_null(time()))
        time(current::time());

    ::std::unique_lock<::std::mutex>
        guard(obtain_mutex());

    // deep copy
    auto
        consumers = obtain_consumers();

    guard.unlock();

    for (auto & [consumer_id, consumer_func] : consumers)
        consumer_func(*this);
}


void
Log::broadcast_if_and_clear()
{
    operator=(Log{});
}


void
Log::throw_error()
{
    if (time::is_null(time()))
        time(current::time());

    throw Error{move()};
}


bool
Log::do_broadcast() const
{
    return p->do_broadcast();
}


Log &
Log::do_broadcast_assign(bool val)
{
    p->do_broadcast_assign(val);
    return *this;
}


::uuids::uuid const &
Log::id() const
{
    return p->id();
}


Log &
Log::id(::uuids::uuid const & v)
{
    p->id_assign(v);
    return *this;
}


Log::Level
Log::level() const
{
    return p->level();
}


Log &
Log::level(Log::Level v)
{
    p->level_assign(v);
    return *this;
}


Log &
Log::level_raise_to(
    Level minimum
)
{
    auto
        curr = level();

    if (int(curr)<int(minimum))
        return level(minimum);

    return *this;
}


Log::Status
Log::status() const
{
    return p->status();
}


Log &
Log::status(Log::Status v)
{
    p->status_assign(v);
    return *this;
}


::uuids::uuid const &
Log::application() const
{
    return p->application();
}


Log &
Log::application(::uuids::uuid const & v)
{
    p->application_assign(v);
    return *this;
}


::uuids::uuid const &
Log::application_instance() const
{
    return p->application_instance();
}


Log &
Log::application_instance(::uuids::uuid const & v)
{
    p->application_instance_assign(v);
    return *this;
}


::std::string const &
Log::version() const
{
    return p->version();
}


Log &
Log::version(
    ::std::string_view const & v
)
{
    p->version_assign(::std::string{v});
    return *this;
}


::uuids::uuid const &
Log::session() const
{
    return p->session();
}


Log &
Log::session(::uuids::uuid const & v)
{
    p->session_assign(v);
    return *this;
}


::uuids::uuid const &
Log::creator() const
{
    return p->creator();
}


Log &
Log::creator(::uuids::uuid const & v)
{
    p->creator_assign(v);
    return *this;
}


::uuids::uuid const &
Log::event() const
{
    return p->event();
}


Log &
Log::event(::uuids::uuid const & v)
{
    p->event_assign(v);
    return *this;
}


::nsBase::time::time_point_t const &
Log::time() const
{
    return p->time();
}


Log &
Log::time(::nsBase::time::time_point_t const & v)
{
    p->time_assign(v);
    return *this;
}


::std::string const &
Log::host() const
{
    return p->host();
}


Log &
Log::host(
    ::std::string_view const & v
)
{
    p->host_assign(::std::string{v});
    return *this;
}


::std::string const &
Log::scope() const
{
    return p->scope();
}


Log &
Log::scope(::std::string_view const & v)
{
    p->scope_mutable() = v;
    return *this;
}


::std::string const &
Log::message() const
{
    return p->message();
}


Log &
Log::message(
    ::std::string_view const & v
)
{
    p->message_mutable() = v;
    return *this;
}


::std::string const &
Log::user() const
{
    return p->user();
}


Log &
Log::user(
    ::std::string_view const & v
)
{
    p->user_assign(::std::string{v});
    return *this;
}


::std::string const &
Log::thread() const
{
    return p->thread();
}


Log &
Log::thread(
    ::std::string_view const & v
)
{
    p->thread_assign(::std::string{v});
    return *this;
}


::std::string
Log::trace() const
{
    return joined(p->trace(), ",");
}


Log &
Log::trace(
    ::std::string_view const & v
)
{
    for (auto s : split(::std::string{v},","s))
        if (auto u = ::uuids::uuid::from_string(s))
            trace(*u);

    return *this;
}


Log &
Log::trace(::uuids::uuid const & v)
{
    p->trace_mutable().emplace_back(v);
    return *this;
}


::std::shared_ptr<Log::ConsumerRegistrationDisposer>
Log::consumer_register(
    ::std::function<void(Log &)>  const & func
)
{
    ::std::lock_guard<::std::mutex>
        guard(obtain_mutex());

    auto
        id = s_consumer_next_id++;

    obtain_consumers()[id] = func;

    ::std::shared_ptr<Log::ConsumerRegistrationDisposer>
        ret;

    ret.reset(new ConsumerRegistrationDisposer(id));

    return ret;
}


// de-serialize
Log &
Log::property(
    ::std::string_view const & key
,   ::std::string_view const & value
)
{
    if (DBC_FAIL(p))
        return *this;  // content was moved

    if (DBC_FAIL(!key.empty()))
        return *this;

    if (DBC_FAIL(key[0] == '_' || key=="scope" || key=="message"))
        return *this;

    auto as_uuid = [&](){return ::nsBase::uuids::from_string_with_empty_to_NIL(value);};

    if (key=="_id"s                     )  return id(as_uuid());
    if (key=="_level"s                  )  return level(level_from_string(value).value_or(Level::DEBUG));
    if (key=="_status"s                 )  return status(status_from_string(value).value_or(Status::OK));
    if (key=="_id_application"s         )  return application(as_uuid());
    if (key=="_id_application_instance"s)  return application_instance(as_uuid());
    if (key=="_version"s                )  return version(value);
    if (key=="_id_session"s             )  return session(as_uuid());
    if (key=="_id_creator"s             )  return creator(as_uuid());
    if (key=="_id_event"s               )  return event(as_uuid());
    if (key=="_time"s                   )  return time(time::time_from_string_utc_YYYY_MM_DD_HH_mm_ss_mmm(value).value_or(time::time_point_t{}));
    if (key=="_host"s                   )  return host(value);
    if (key=="_user"s                   )  return user(value);
    if (key=="_thread"s                 )  return thread(value);
    if (key=="_trace"s                  )  return trace(value);
    if (key=="scope"s                   )  return scope(value);
    if (key=="message"s                 )  return message(value);

    return *this;
}


// serialize
::std::optional<::std::string>
Log::property(
    ::std::string_view const & key
) const
{
    if (!p)
        return {};  // content was moved

    if (key=="_id"s                         && !id().is_nil()                   ) return to_string(id());
    if (key=="_level"s                      && true                             ) return to_string(level());
    if (key=="_status"s                     && true                             ) return to_string(status());
    if (key=="_id_application"s             && !application().is_nil()          ) return to_string(application());
    if (key=="_id_application_instance"s    && !application_instance().is_nil() ) return to_string(application_instance());
    if (key=="_version"s                    && !version().empty()               ) return version();
    if (key=="_id_session"s                 && !session().is_nil()              ) return to_string(session());
    if (key=="_id_creator"s                 && !creator().is_nil()              ) return to_string(creator());
    if (key=="_id_event"s                   && !event().is_nil()                ) return to_string(event());
    if (key=="_time"s                       && !time::is_null(time())           ) return to_string_iso_utc(time());
    if (key=="_host"s                       && !host().empty()                  ) return host();
    if (key=="_user"s                       && !user().empty()                  ) return user();
    if (key=="_thread"s                     && true                             ) return thread();
    if (key=="_trace"s                      && !p->trace().empty()              ) return trace();
    if (key=="scope"s                       && !scope().empty()                 ) return scope();
    if (key=="message"s                     && !message().empty()               ) return message();

    return {};
}


void
Log::consumers_force_dispose_all()
{
    ::std::lock_guard<::std::mutex>
        guard(obtain_mutex());

    obtain_consumers().clear();
}


void
Log::ConsumerRegistrationDisposer::dispose()
{
    if (!m_id)
        return;

    ::std::lock_guard<::std::mutex>
        guard(obtain_mutex());

    obtain_consumers().erase(*m_id);

    m_id.reset();
}



Log::ConsumerRegistrationDisposer::ConsumerRegistrationDisposer(
    unsigned int id
)
:   m_id(id)
{
}



Log::ConsumerRegistrationDisposer::~ConsumerRegistrationDisposer()
{
    dispose();
}



::std::string
to_string(
    Log::Level level
)
{
    switch (level)
    {
    case Log::Level::DEBUG    : return "DEBUG";
    case Log::Level::INFO     : return "INFO";
    case Log::Level::WARNING  : return "WARNING";
    case Log::Level::FAILURE  : return "ERROR";
    case Log::Level::CRITICAL : return "CRITICAL";
    default:
        return "<error>";
    }
}



::std::optional<Log::Level>
level_from_string(
    ::std::string_view s
)
{
    if (s=="DEBUG"   ) return Log::Level::DEBUG   ;
    if (s=="INFO"    ) return Log::Level::INFO    ;
    if (s=="WARNING" ) return Log::Level::WARNING ;
    if (s=="ERROR"   ) return Log::Level::FAILURE ;
    if (s=="CRITICAL") return Log::Level::CRITICAL;

    return {};
}



::std::string
to_string(
    Log::Status s
)
{
    switch (s)
    {
    case Log::Status::OK                  : return "OK";
    case Log::Status::CANCELLED           : return "CANCELLED";
    case Log::Status::UNKNOWN             : return "UNKNOWN";
    case Log::Status::INVALID_ARGUMENT    : return "INVALID_ARGUMENT";
    case Log::Status::DEADLINE_EXCEEDED   : return "DEADLINE_EXCEEDED";
    case Log::Status::NOT_FOUND           : return "NOT_FOUND";
    case Log::Status::ALREADY_EXISTS      : return "ALREADY_EXISTS";
    case Log::Status::PERMISSION_DENIED   : return "PERMISSION_DENIED";
    case Log::Status::UNAUTHENTICATED     : return "UNAUTHENTICATED";
    case Log::Status::RESOURCE_EXHAUSTED  : return "RESOURCE_EXHAUSTED";
    case Log::Status::FAILED_PRECONDITION : return "FAILED_PRECONDITION";
    case Log::Status::ABORTED             : return "ABORTED";
    case Log::Status::OUT_OF_RANGE        : return "OUT_OF_RANGE";
    case Log::Status::UNIMPLEMENTED       : return "UNIMPLEMENTED";
    case Log::Status::INTERNAL            : return "INTERNAL";
    case Log::Status::UNAVAILABLE         : return "UNAVAILABLE";
    case Log::Status::DATA_LOSS           : return "DATA_LOSS";
    default                               : return "<error>";
    }
}


::std::optional<Log::Status>
status_from_string(
    ::std::string_view s
)
{
    if (s=="OK"                 ) return Log::Status::OK                 ;
    if (s=="CANCELLED"          ) return Log::Status::CANCELLED          ;
    if (s=="UNKNOWN"            ) return Log::Status::UNKNOWN            ;
    if (s=="INVALID_ARGUMENT"   ) return Log::Status::INVALID_ARGUMENT   ;
    if (s=="DEADLINE_EXCEEDED"  ) return Log::Status::DEADLINE_EXCEEDED  ;
    if (s=="NOT_FOUND"          ) return Log::Status::NOT_FOUND          ;
    if (s=="ALREADY_EXISTS"     ) return Log::Status::ALREADY_EXISTS     ;
    if (s=="PERMISSION_DENIED"  ) return Log::Status::PERMISSION_DENIED  ;
    if (s=="UNAUTHENTICATED"    ) return Log::Status::UNAUTHENTICATED    ;
    if (s=="RESOURCE_EXHAUSTED" ) return Log::Status::RESOURCE_EXHAUSTED ;
    if (s=="FAILED_PRECONDITION") return Log::Status::FAILED_PRECONDITION;
    if (s=="ABORTED"            ) return Log::Status::ABORTED            ;
    if (s=="OUT_OF_RANGE"       ) return Log::Status::OUT_OF_RANGE       ;
    if (s=="UNIMPLEMENTED"      ) return Log::Status::UNIMPLEMENTED      ;
    if (s=="INTERNAL"           ) return Log::Status::INTERNAL           ;
    if (s=="UNAVAILABLE"        ) return Log::Status::UNAVAILABLE        ;
    if (s=="DATA_LOSS"          ) return Log::Status::DATA_LOSS          ;

    return {};
}



::std::map<::std::string,::std::string>
Log::properties_and_attributes() const
{
    ::std::map<::std::string,::std::string>
        m;

    static const auto
        keys =
            {
                "_id"s
            ,   "_level"s
            ,   "_status"s
            ,   "_id_application"s
            ,   "_id_application_instance"s
            ,   "_version"s
            ,   "_id_session"s
            ,   "_id_creator"s
            ,   "_id_event"s
            ,   "_time"s
            ,   "_host"s
            ,   "_user"s
            ,   "_thread"s
            ,   "_trace"s
            ,   "scope"s
            ,   "message"s
            };

    for (auto & key : keys)
    {
        if (auto value = property(key))
            m.emplace(key, *value);
    }

    m.insert(
            p->mAttributes.begin()
        ,   p->mAttributes.end()
        );

    return m;
}


::std::string
Log::serialize(
    bool pretty
) const
{
    if (!p)
        return {};  // content was moved

    ::nlohmann::json
        j;
        for (auto & [k,v] : properties_and_attributes()) j[k] = v;

    return j.dump(
            pretty ? 4 : -1 // int indent = -1
        );
}


Log &
Log::operator=(
    Log && rhs
)
{
    if (this!=&rhs)
        ::std::swap(p,rhs.p);

    return *this;
}


::std::optional<Log>
Log::deserialize(
    ::std::string_view  const & data
)
{
    ::std::optional<Log>
        log;

    auto stream = data;

    // crop optional prefix up to the first '{'-character
    if (auto i=data.find_first_of('{'); i!=::std::string::npos)
        stream = ::std::string_view{data.data()+i, data.size()-i};

    try
    {
        auto json = ::nlohmann::json::parse(stream);

        log.emplace();

        for (auto const & [k_,v_] : json.items())
        {
            auto k = ::std::string_view{k_};
            auto v = v_.is_string() ? ::std::string{v_} : to_string(v_);

            auto
                is_property = k[0] == '_' || k=="scope" || k=="message";

            if (is_property)
                log->property(k, v);
            else
                log->att(k, v);
        }
    }
    catch(...)
    {
        log.reset();
    }

    return log;
}


::std::size_t
Log::attribute_count() const
{
    if (!p)
        return 0;  // content was moved

    return p->mAttributes.size();
}


::std::map<::std::string,::std::string> const &
Log::attributes() const
{
    DBC_PRE(p);

    return p->mAttributes;
}


::std::optional<::std::string>
Log::attribute(
    ::std::string const & key
) const
{
    if (!p)
        return {};  // content was moved

    auto
        it = p->mAttributes.find(key);

    if (it==p->mAttributes.end())
        return {};

    return it->second;
}


Log &
Log::att_s(
    ::std::string_view const & key
,   ::std::string_view const & value
)
{
    if (!p)
        return *this;  // content was moved

    if (key.empty())
        return *this;

    p->mAttributes[::std::string(key)] = value;

    return *this;
}


void
Log::attributeRemoveAll()
{
    p->mAttributes.clear();
}


::std::string
Log::resolved(
    ::std::string_view const & s
) const
{
    ::std::regex
        regex("\\$\\{([^$]*)\\}");
    ::std::smatch
        match;
    auto
        res = ::std::string{s};

    while (::std::regex_search(res,match,regex))
    {
        if (match.size()<2)
            break;
        auto
            key = match[1].str();
        auto
            value = attribute(key);

        if (!value)
            value = property(key);

        if (!value)
            value = "<" + key + ">";

        auto
            new_res = resolved(res, key, *value);

        if (res==new_res)
            break; // break endless loops

        res = ::std::move(new_res);
    }

    return res;
}


::std::string
Log::resolved(
    ::std::string_view const & s
,   ::std::string_view const & key
,   ::std::string_view const & value
) const
{
    try
    {
        ::std::string res;

        ::std::regex_replace(
                ::std::back_inserter(res)
            ,   s.begin()
            ,   s.end()
            ,   ::std::regex("\\$\\{" + ::std::string{key} + "\\}") // the resulting regex might become invalid (example: key == "line]")
            ,   ::std::string{value}
            );

        return res;
    }
    catch(...)
    {
        return ::std::string{s};
    }
}


void
logs_read(
    ::std::vector<Log> & logs
,   ::fs::path   const & path
)
{
    ::std::ifstream
        stream {path, ::std::ios::in | ::std::ios::binary};

    ::std::string
        line;

    while (::std::getline(stream,line))
        if (auto log = Log::deserialize(line))
            logs.emplace_back(::std::move(*log));

    stream.clear();
    stream.close();

    if (stream.fail())
        "269ea192-c498-4401-bf0c-8b743398ab2e"_log().throw_error();
}


::std::vector<Log>
log_read(
    ::fs::path const & path
)
{
    ::std::vector<Log>
        logs;

    logs_read(logs, path);

    return logs;
}



void
log_filter_anti_flood(
    ::std::function<void(Log &)>  const & func
,   int                                   max_event_count_per_creator_per_duration
,   ::std::chrono::milliseconds           duration
,   Log                                 & log
)
{
    if (!func)
        return;

    using tp_t = ::std::chrono::system_clock::time_point;

    // creator-id -> time_points
    static ::std::map<::uuids::uuid, ::std::set<tp_t>>
        history;

    static ::std::mutex
        mutex; // to secure 'history'

    {
        ::std::unique_lock<::std::mutex>
            guard(mutex);

        auto &
            events = history[log.creator()];

        auto now = ::std::chrono::system_clock::now();

        if (events.size()>=max_event_count_per_creator_per_duration)
        {
            erase_if(events, [&](tp_t const & t){return now-t > duration;});

            if (events.size()==max_event_count_per_creator_per_duration)
            {
                log
                    ("log_limiter_message"               , "bandwith limit reached - probably skipping following logs of this consumer"s)
                    ("log_limiter_duration_milliseconds" , duration.count())
                    ("log_limiter_duration_max_count"    , max_event_count_per_creator_per_duration)
                    ;
            }

            if (events.size()>max_event_count_per_creator_per_duration)
                return; // skip log due to overload
        }

        events.emplace(now);
    }

    func(log);
}


Log
log_application_execution_span(
    Log && pattern
)
{
    auto
        begin_log = ::std::move(pattern);
        begin_log
            ("${service.name} ${_version} [${build_time}] git[${git_commit}] instance[${_id_application_instance}]")
            .info()
            .event("70ae06d0-9e8d-4af0-9083-107e17a11a02"_uuid) // event_id(EventID::session_begin))
            .version(current::application_version())
            ("service.name", current::application_name())
            ("build_time"  , current::application_build_time())
            ("git_commit"  , current::application_git_commit_id())
            ;

    auto
        exit_log = Log{begin_log}; // deep copy
        exit_log("exiting instance[${_id_application_instance}]")
        .event("4f62852d-31fa-47dd-b064-356dad92fc64"_uuid) // event_id(EventID::session_end))
        .do_broadcast_assign(true)
        ;

    begin_log = {}; // explicit emission

    return exit_log;
}

}
