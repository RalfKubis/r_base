/* Copyright (C) Ralf Kubis */
#include "r_base/Log.h"

#include "r_base/string.h"
#include "r_base/vector.h"
#include "r_base/Property.h"
#include "r_base/time.h"
#include "r_base/Error.h"

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

#if defined __linux__
#include <unistd.h>
#include <limits.h>
#include <pwd.h>
#endif

#if defined win_x86_32
#include <Windows.h>
#include <Lmcons.h>
#endif

using namespace ::std::string_literals;
using namespace ::uuids;

namespace
{

::std::mutex &
obtain_mutex()
{
    static ::std::atomic<::std::mutex*>
        consumer_mutex(nullptr);

    if (consumer_mutex.load()==nullptr)
    {
        ::std::mutex *
            null = nullptr;

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
                consumers = nullptr;

            if (!consumers)
                consumers = new t_consumers;

            return *consumers;
        }
}


namespace
{

::std::optional<::std::string> &
    current_user_()
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

namespace nsBase
{

::std::string
Log::current_user()
{

    if (!current_user_())
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
                current_user_().emplace(buf);
            else
                current_user_().emplace(u8"?"s);
        }

        // username associated with the effective user ID of the process
        if (false)
        {
            // problem: L_cuserid is too small and unames get truncated
            // got 9 on tara
            char buf[L_cuserid+1];

            if (auto uid=cuserid(nullptr))
                current_user_().emplace(uid);
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
                current_user_().emplace(pw_result->pw_name);
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
            current_user_().emplace(buf.data(),bufSize-1);
        }

#endif
    }

    if (!current_user_())
        current_user_().emplace();

    return *current_user_();
}

}



namespace
{
::std::optional<::std::string> &
    current_host_()
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


namespace nsBase
{

::std::string
Log::current_host()
{
    if (!current_host_())
    {
#if defined __linux__

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif

        char buf[HOST_NAME_MAX];

        if (gethostname(buf, HOST_NAME_MAX)==0)
            current_host_().emplace(buf);

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
            current_host_().emplace(buf.data(),bufSize);
        }
#endif
    }

    if (!current_host_())
        current_host_().emplace();

    return *current_host_();
}

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
            application
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
        ,   ::std::chrono::time_point<::std::chrono::system_clock>
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
    if (!p)
        return;  // content was moved

    if (!do_broadcast())
        return;

    if (time::is_null(time()))
        time(current_time());

    ::std::unique_lock<::std::mutex>
        guard(obtain_mutex());

    // deep copy
    auto
        consumers = obtain_consumers();

    guard.unlock();

    for (auto & c : consumers)
        c.second(*this);
}


Log::Log(
    ::uuids::uuid const & creator_id
)
:   p(::std::make_unique<Log_Impl>())
{
    if (creator_id)
    {
        id(::uuids::uuid_system_generator()());
        application(application_id());
        session(current_thread_session_id());
        creator(creator_id);
        host(current_host());
        user(current_user());
        thread(current_thread());
    }
    else
    {
        do_broadcast_assign(false);
    }
}


Log::Log()
:   Log(::uuids::uuid())
{
}


Log::Log(
    Log && other
)
:   p(::std::move(other.p))
{
}


Log::Log(
    Log const & other
)
:   p(::std::make_unique<Log_Impl>(*other.p)) // deep copy
{
}


void
Log::broadcast_if_and_clear()
{
    operator=(Log());
}


bool
Log::do_broadcast() const
{
    return p->do_broadcast();
}


void
Log::do_broadcast_assign(bool val)
{
    p->do_broadcast_assign(val);
}


namespace
{
::uuids::uuid &
    current_application_id()
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
Log::application_id_assign(
    ::uuids::uuid const & id
)
{
    current_application_id() = id;
}


::uuids::uuid
Log::application_id()
{
    return current_application_id();
}



void
Log::thread_session_id_assign(
    ::uuids::uuid const & id
)
{
    current_thread_session_id_assign(id);
}


::uuids::uuid
Log::thread_session_id()
{
    return current_thread_session_id();
}



::uuids::uuid
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


::uuids::uuid
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


::std::string
Log::version() const
{
    return p->version();
}


Log &
Log::version_(::std::string const & v)
{
    p->version_assign(v);
    return *this;
}


::uuids::uuid
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


::uuids::uuid
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


::uuids::uuid
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


Log::time_t
Log::current_time()
{
    return time::now();
}


Log::time_t
Log::time() const
{
    return p->time();
}


Log &
Log::time(time_t const & v)
{
    p->time_assign(v);
    return *this;
}


::std::string
Log::time_as_string() const
{
    return to_string(time());
}


Log &
Log::time_from_string(::std::string const & s)
{
    time(time::time_from_string(s));
    return *this;
}


::std::string
Log::host() const
{
    return p->host();
}


Log &
Log::host(::std::string const & v)
{
    p->host_assign(v);
    return *this;
}


::std::string
Log::user() const
{
    return p->user();
}


Log &
Log::user(::std::string const & v)
{
    p->user_assign(v);
    return *this;
}


::std::string
Log::current_thread()
{
    ::std::stringstream
        s;

    s << ::std::this_thread::get_id();

    return s.str();
}


::std::string
Log::thread() const
{
    return p->thread();
}


Log &
Log::thread(::std::string const & v)
{
    p->thread_assign(v);
    return *this;
}


::std::string
Log::trace() const
{
    return joined(p->trace(), ",");
}


Log &
Log::trace(::std::string const & v)
{
    for (auto s : split(v,","s))
        trace(::uuids::uuid(s));

    return *this;
}


Log &
Log::trace(::uuids::uuid const & v)
{
    p->trace_mutable().emplace_back(v);
    return *this;
}


std::shared_ptr<Log::ConsumerRegistrationDisposer>
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
    ::std::string const & key
,   ::std::string const & value
)
{
    if (DBC_FAIL(p))
        return *this;  // content was moved

    if (DBC_FAIL(!key.empty()))
        return *this;

    if (DBC_FAIL(key[0]=='_'))
        return *this;

    if (key==u8"_id"s            )  return id(uuid(value));
    if (key==u8"_level"s         )  return level(level_from_string(value).value_or(Level::DEBUG));
    if (key==u8"_id_application"s)  return application(uuid(value));
    if (key==u8"_version"s       )  return version(value);
    if (key==u8"_id_session"s    )  return session(uuid(value));
    if (key==u8"_id_creator"s    )  return creator(uuid(value));
    if (key==u8"_id_event"s      )  return event(uuid(value));
    if (key==u8"_time"s          )  return time_from_string(value);
    if (key==u8"_host"s          )  return host(value);
    if (key==u8"_user"s          )  return user(value);
    if (key==u8"_thread"s        )  return thread(value);
    if (key==u8"_trace"s         )  return trace(value);

    //DBC_ASSERT(false);

    return *this;
}


// serialize
::std::optional<::std::string>
Log::property(
    ::std::string const & key
) const
{
    if (!p)
        return {};  // content was moved

    if (key==u8"_id"s            && id()                  ) return to_string(id());
    if (key==u8"_level"s         && true                  ) return to_string(level());
    if (key==u8"_id_application"s&& application()         ) return to_string(application());
    if (key==u8"_version"s       && !version().empty()    ) return version();
    if (key==u8"_id_session"s    && session()             ) return to_string(session());
    if (key==u8"_id_creator"s    && creator()             ) return to_string(creator());
    if (key==u8"_id_event"s      && event()               ) return to_string(event());
    if (key==u8"_time"s          && !time::is_null(time())) return time_as_string();
    if (key==u8"_host"s          && !host().empty()       ) return host();
    if (key==u8"_user"s          && !user().empty()       ) return user();
    if (key==u8"_thread"s        && true                  ) return thread();
    if (key==u8"_trace"s         && !p->trace().empty()   ) return trace();

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



::std::map<::std::string,std::string>
Log::properties_and_attributes() const
{
    ::std::map<::std::string,std::string>
        m;

    auto
        keys =
            {
                u8"_id"s
            ,   u8"_level"s
            ,   u8"_id_application"s
            ,   u8"_version"s
            ,   u8"_id_session"s
            ,   u8"_id_creator"s
            ,   u8"_id_event"s
            ,   u8"_time"s
            ,   u8"_host"s
            ,   u8"_user"s
            ,   u8"_thread"s
            ,   u8"_trace"s
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
    Format const inFormat
,   int    const inIndent
) const
{
    if (!p)
        return {};  // content was moved

    auto
        indent = ::std::max(inIndent,-1);

    ::std::string
        retVal;

    bool
        doIndent = indent>=0;

    // base indent
    ::std::string
        i0;
    // inner indent
    ::std::string
        i01;
    // end of token
    ::std::string
        eot(" ");
    // comma
    ::std::string
        comma = ", ";

    if ( doIndent )
    {
        i0.resize(indent, ' ');
        i01.resize(4, ' ');
        eot = "\n";
        comma = ",   ";
    }

    auto
        esc = []( ::std::string const & str ) -> ::std::string
            {
                auto a = replaced_all(str, "\\", "\\\\");
                auto b = replaced_all(  a, "\"", "\\\"" );
//                auto c = replaced_all(  b, "\0", "\\0" );
                return b;
            };

    auto
        pairs = properties_and_attributes();

    switch (inFormat)
    {
    ////////////////////////////////////////////////////////////
    case Format::JSON :
        {
            retVal += i0 + "TRACE" + eot;
            retVal += i0 + "{" + eot;

            auto num = 0;
            for (auto & kv : pairs)
            {
                retVal += i0;
                retVal +=  (num==0 ? i01 : comma);
                retVal +=  '"';
                retVal +=  esc(kv.first);
                retVal +=  "\":\"";
                retVal +=  esc(kv.second);
                retVal +=  '"';
                retVal +=  eot;

                num++;
            }

            retVal += i0 + "}" + eot;
        }
        break;


    ////////////////////////////////////////////////////////////
    case Format::XML :
        {
            retVal += i0 + "<TRACE" + eot;

            for (auto & kv : pairs)
            {
                retVal
                    += (i0 + i01)
                    +  esc(kv.first)
                    +  "=\""
                    +  esc(kv.second)
                    +  "\""
                    +  eot
                    ;
            }

            retVal += i0 + "/>" + eot;
        }
        break;


    ////////////////////////////////////////////////////////////
    case Format::TEXT :
        {
            for (auto & kv : pairs)
            {
                retVal
                    += i0
                    +  kv.first
                    +  "="
                    +  kv.second
                    +  "\n"
                    ;
            }
        }
        break;
    ////////////////////////////////////////////////////////////
    }

    return retVal;
}



namespace
{
Status
readString(
    ::std::string & inoutStream
,   ::std::string & outValue
)
{
    Status
        retVal;

    int
        offset = 0;

    auto
        stream_is_empty = [&]()
            {
                return offset >= inoutStream.size();
            };

    do // BLOCK
    {
        if (stream_is_empty())
            FailBreak("ef4538dd-ea03-409d-ad49-62491bcbde26"_uuid);

        outValue.clear();

        // goto next "
        for(;;)
        {
            if (stream_is_empty())
                FailBreak("51bca420-f057-4c85-aa9e-d3face3ccb87"_uuid);

            if (inoutStream[offset]=='"')
                break;

            offset++;
        }
        BreakOnFail;

        // skip "
        offset++;

        // pull string chars
        bool isEscaped = false;
        for(;;)
        {
            if (stream_is_empty())
                FailBreak("6de258c6-c248-4303-989a-5e95e354670a"_uuid);

            if (    !isEscaped
                &&  inoutStream[offset]=='\\'
            )
            {
                isEscaped = true;
                offset++;
                continue;
            }

            // read escaped character
            if (isEscaped)
            {
                isEscaped = false;
                outValue.push_back(inoutStream[offset]);
                offset++;
                continue;
            }

            // ending "
            if (inoutStream[offset]=='"')
            {
                offset++;
                break;
            }

            // pop character
            outValue.push_back(inoutStream[offset]);
            offset++;
        }
        BreakOnFail;
    }
    while(false); // FIN

    inoutStream.erase(0,std::min<int>(offset,inoutStream.size()));

    if (!retVal)
        outValue.clear();

    return retVal;
}
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


Log &
Log::operator=(
    Log const & rhs
)
{
    if (this!=&rhs)
        *p = *rhs.p;

    return *this;
}


::std::optional<Log>
Log::deserialize(
    ::std::string  const & data
,   Format               format
)
{
    ::std::string
        stream = data;

    ::std::optional<Log>
        log;

    log.emplace();

    Status
        retVal;


    switch (format)
    {
    ////////////////////////////////////////////////////////////
    case Format::JSON :

        while (contains(stream, '"'))
        {
            ::std::string
                key;
            ::std::string
                value;

            retVal << readString(stream, key);
            BreakOnFail;

            retVal << readString(stream, value);
            BreakOnFail;

            if (key.empty())
                continue;

            auto
                is_property = key[0] == '_';

            if (is_property)
            {
                log->property(key,value);
            }
            else
            {
                DBC_ASSERT(!log->attribute(key));
                log->att(key,value);
            }
        }
        break;


    ////////////////////////////////////////////////////////////
    case Format::TEXT :
        {
            NYI(u8"042b527d-6dcd-4c22-94dc-9bedfd7b816c"_uuid);
        }
        break;


    ////////////////////////////////////////////////////////////
    case Format::XML :
//        DBC_NYI;
        break;
    ////////////////////////////////////////////////////////////
    }

    if (!retVal)
        return {};

    return ::std::move(log);
}


::std::size_t
Log::attribute_count() const
{
    if (!p)
        return 0;  // content was moved

    return p->mAttributes.size();
}


::std::map<::std::string,std::string> const &
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
    ::std::string const & s
) const
{
    ::std::regex
        regex("\\$\\{([^$]*)\\}");
    ::std::smatch
        match;
    auto
        res = s;

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
    ::std::string const & s
,   ::std::string const & key
,   ::std::string const & value
) const
{
    try
    {
        return ::std::regex_replace(
                s
            ,   ::std::regex("\\$\\{" + key + "\\}") // the resulting regex might become invalid (example: key == "line]")
            ,   value
            );
    }
    catch(...)
    {
        return s;
    }
}


::std::vector<Log>
log_read(
    ::fs::path const & path
)
{
    ::std::vector<Log>
        logs;

    ::std::ifstream
        stream(path.u8string(), ::std::ios::binary);

    ::std::string
        line;

    while (::std::getline(stream,line))
    {
        if (auto log = Log::deserialize(line,Log::Format::JSON) )
        {
            logs.emplace_back(::std::move(*log));
        }
    }

    stream.clear();
    stream.close();

    if (stream.fail())
        throw Error(Log(u8"269ea192-c498-4401-bf0c-8b743398ab2e"_uuid));

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
                    .att("log_limiter_message"               , u8"bandwith limit reached - probably skipping following logs of this consumer"s)
                    .att("log_limiter_duration_milliseconds" , duration.count())
                    .att("log_limiter_duration_max_count"    , max_event_count_per_creator_per_duration)
                    ;
            }

            if (events.size()>max_event_count_per_creator_per_duration)
                return; // skip log due to overload
        }

        events.emplace(now);
    }

    func(log);
}

}
