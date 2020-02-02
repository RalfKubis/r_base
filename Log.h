#pragma once
/* Copyright (C) Ralf Kubis */

#include "r_base/language_tools.h"
#include "r_base/decl.h"
#include "r_base/dbc.h"
#include "r_base/filesystem.h"
#include "r_base/uuid.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <cstddef>
#include <utility>
#include <cstddef>
#include <chrono>
#include <thread>


namespace nsBase
{
/**
    An instance of this class is used to collect attributes.
    The serialized attributes are intended to be machine readeable, i.e. easily
    parseable. They follow a grammar which is currently JSON, XML or PlainText.

    An attribute is a key-value pair. Keys are strings. Values ar of type string
    or Log. The latter allows hierarchical structures.
    When an instance is destroyed, the destructor prints, if desired, all
    attributes that got appended.
    There are attributes that are automatically added on construction like
    'id', 'time' and 'thread'.
    There can't be multiple attribute values with the same key.

    There are some pre-defined attributes that use to occur very often.
    These attributes can be updated by using convenience methods that do not
    require a key parameter. This helps to harmonize the logss and makes
    is easier to analyse Logs.

    In this example a single Log is emitted. Additionally to the automatic
    attributes, the pre-defined attributes 'scope' and 'message' are added.
    Finally the attribute 'requestId' is added. Since in this case the instance
    is not bound to a variable, the instance is immediately destroyed and
    therefore serialized and log-ed by its destructor.

        ...

        Log("EED119DA-075A-4b3d-B152-2A3651FEB351"_uuid)
            .info()
            .scope("foobar()")
            .message("shader initialisation")
            .att("filePath", filepath)
            ;

        ...
*/
class  Log
:   public ::std::enable_shared_from_this<Log>
{
    private : ::std::unique_ptr<class Log_Impl>
        p;


    public : enum class Level
        {
            DEBUG       = 0
        ,   INFO        = 1
        ,   WARNING     = 2
        ,   FAILURE     = 3
        ,   CRITICAL    = 4
        };

////////////////////////////////////////////////////////////////////////////////
/** \name Standard Functions
@{*/
    /**
        If the property 'do_broadcast' is set to TRUE, the Log is serialized and
        printed to the log by the destructor.
        This attribute gets set to TRUE by the constructor if a non-empty
        creator_id was provided.
    */
    R_DTOR(Log);
    R_CTOR(Log);
    R_CCPY(Log);
    R_CMOV(Log);
    R_COPY(Log);
    R_MOVE(Log);

    /**
        Construct a new instance. Automatic attributes get added.

        \param id_code If non-empty, it is the value of the attribute '_id_creator'
                       and automatic attributes are added and the instance
                       will get broadcasted to the consumers on its
                       destruction. If empty, no automatic attributes are
                       added and the instance will not be broadcasted.
    */
    public :
        Log(
                ::uuids::uuid const & id_creator
            );
//@}

    /**
        If TRUE, the instance gets broadcasted to the registered consumers prior
        to its destruction.
    */
    public : bool
        do_broadcast() const;
    public : void
        do_broadcast_assign(bool val);

    /**
        If applicable, broadcast the instance.
        Clear the instance to its initial state.
        The creator-id gets set to EMPTY.
    */
    public : void
        broadcast_if_and_clear();


////////////////////////////////////////////////////////////////////////////////
/** \name Properties
@{*/
    public : static void
        application_id_assign(
                ::uuids::uuid const & id
            );

    public : static ::uuids::uuid
        application_id();



    public : static void
        thread_session_id_assign(
                ::uuids::uuid const & id
            );

    public : static ::uuids::uuid
        thread_session_id();



    public : ::uuids::uuid
        id() const;
    public : Log &
        id(::uuids::uuid const &);


    public : Log::Level
        level() const;
    public : Log &
        level(Log::Level);
    /**
        Update the pre-defined attribute '_level' to DEBUG.
    */
    public : Log &
        debug()
            {
                return level(Level::DEBUG);
            }


    /**
        Update the pre-defined attribute '_level' to INFO.
    */
    public : Log &
        info()
            {
                return level(Level::INFO);
            }

    /**
        Update the pre-defined attribute '_level' to WARNING.
    */
    public : Log &
        warning()
            {
                return level(Level::WARNING);
            }

    /**
        Update the pre-defined attribute '_level' to FAILURE.
    */
    public : Log &
        error()
            {
                return level(Level::FAILURE);
            }

    /**
        Update the pre-defined attribute '_level' to CRITICAL.
    */
    public : Log &
        critical()
            {
                return level(Level::CRITICAL);
            }
    /**
        Update the pre-defined attribute '_level' to the target level if the
        current value is less severe than the new value.
    */
    public : Log &
        level_raise_to(
                Level minimum
            );

    public : ::uuids::uuid
        application() const;
    public : Log &
        application(::uuids::uuid const &);


    public : ::std::string
        version() const;
    public : Log &
        version_(::std::string const &);

    public : template<typename V>
        Log &
            version(
                    V const & v
                )
                {
                    return version_(to_string(v));
                }


    public : ::uuids::uuid
        session() const;
    public : Log &
        session(::uuids::uuid const &);


    public : ::uuids::uuid
        creator() const;
    public : Log &
        creator(::uuids::uuid const &);


    public : ::uuids::uuid
        event() const;
    public : Log &
        event(::uuids::uuid const &);

    public : using
        time_t = ::std::chrono::time_point<::std::chrono::system_clock>;

    public : static time_t
        current_time();
    public : time_t
        time() const;
    public : ::std::string
        time_as_string() const;
    public : Log &
        time_from_string(::std::string const &);
    public : Log &
        time(time_t const &);


    public : ::std::string
        host() const;
    public : Log &
        host(::std::string const &);
    public : static ::std::string
        current_host();


    public : ::std::string
        user() const;
    public : Log &
        user(::std::string const &);
    public : static ::std::string
        current_user();


    public : ::std::string
        thread() const;
    public : Log &
        thread(::std::string const &);
    public : static ::std::string
        current_thread();


    public : Log &
        property(
                ::std::string const & key
            ,   ::std::string const & value
            );

    public : ::std::optional<::std::string>
        property(
                ::std::string const & key
            ) const;

    /** A sequence of Creator-IDs that allows it to reconstruct the path a log
        was routed along.
        If for example a Log is thrown as an exception value, probably enclosed
        in an object of class Error, the position where the Log gets re-thrown
        or placed into an object like ::tl::expected<_,Error>, this hop should
        be added to the trace for later diagnostics.
    */

    // serialize
    public : ::std::string
        trace() const;
    // de-serialize
    public : Log &
        trace(::std::string const &);
    /// append a hop to the trace
    public : Log &
        trace(::uuids::uuid const &);
//@}


////////////////////////////////////////////////////////////////////////////////
/** \name Dynamic Attributes
@{*/
    /**
        Query the Log value of the attribute with the target key.
    */

    public : ::std::size_t
        attribute_count() const;

    /**
        Query the string value of the attribute with the target key.
    */
    public : ::std::optional<::std::string>
        attribute(
                ::std::string const & key
            ) const;

    //public : ::std::optional<::std::pair<::std::string,::std::string>>
    //    attribute(
    //            ::std::size_t attribute_index
    //        ) const;

    //private : ::std::optional<::std::size_t>
    //    attribute_index(
    //            ::std::string const & key
    //        ) const;


    /**
        Update an attribute. If the target key was not registered
        before, a new attribute is added. If the target key was registered
        before, the attribute value ges set to the target value.

        \param  key   The target key.
        \param  value The target value.

        \return
        Reference to THIS. This allows chaining.
    */
    public : Log &
        att_s(
                ::std::string_view const & key
            ,   ::std::string_view const & value
            );

    public : template<typename val_t> Log &
        att(
                ::std::string_view  const & key
            ,   val_t               const & value
            )
            {
                return att_s(key, ::std::to_string(value));
            }


    public : ::std::map<::std::string,::std::string> const &
        attributes() const;

    /**
        Remove all attributes.
    */
    public : void
        attributeRemoveAll();


    /**
        Substitute all occurences of the pattern ${attribute_name} by the
        actual values of the addressed attribute. If the attribute is not
        present, the string  <attribute_name> gets inserted.

        \param s          The source string.

        \return A new string with the patterns replaced accordingly.
    */
    public : ::std::string
        resolved(
                ::std::string const & s
            ) const;

    /**
        Substitute all occurences of the pattern ${key} by the given value.

        \param s     The source string
        \param key   The key to address. This is a regular expression.
        \param value The replacement value.

        \return A new string with the keys replaced.
    */
    public : ::std::string
        resolved(
                ::std::string const & s
            ,   ::std::string const & key
            ,   ::std::string const & value
            ) const;
//@}

    public : ::std::map<::std::string,::std::string>
        properties_and_attributes() const;

////////////////////////////////////////////////////////////////////////////////
/** \name Helper for common dynamic attributes
@{*/

    /**
        Update the pre-defined attribute 'scope'.
    */
    public : Log &
        scope(
                ::std::string_view const & inValue
            )
            {
                att_s("scope", inValue);
                return *this;
            }

    /**
        Update the pre-defined attribute 'action'.
    */
    public : Log &
        action(
                ::std::string_view const & inValue
            )
            {
                att_s("action", inValue);
                return *this;
            }

    /**
        Update the pre-defined attribute 'object'.
    */
    public : Log &
        object(
                ::std::string_view const & inValue
            )
            {
                att_s("object", inValue);
                return *this;
            }

    /**
        Update the pre-defined attribute 'count'.
    */
    public : inline Log &
        count(
                ::std::int64_t const & value
            );
    /**
        Update the pre-defined attribute 'count2'.
    */
    public : inline Log &
        count1(
                ::std::int64_t const & value
            );

    /**
        Update the pre-defined attribute 'source'.
    */
    public : Log &
        source(
                ::std::string_view const & inValue
            )
            {
                att_s("source", inValue);
                return *this;
            }

    /**
        Update the pre-defined attribute 'target'.
    */
    public : Log &
        target(
                ::std::string_view const & inValue
            )
            {
                att_s("target", inValue);
                return *this;
            }

    /**
        Update the pre-defined attribute 'key'.
    */
    public : Log &
        key(
                ::std::string_view const & inValue
            )
            {
                att_s("key", inValue);
                return *this;
            }

    /**
        Update the pre-defined attribute 'value'.
    */
    public : Log &
        value(
                ::std::string_view const & inValue
            )
            {
                att_s("value", inValue);
                return *this;
            }

    /**
        Update the pre-defined attribute 'data'.
    */
    public : Log &
        data(
                ::std::string_view const & inValue
            )
            {
                att_s("data", inValue);
                return *this;
            }

    /**
        Update the pre-defined attribute 'data'.
    */
    public : inline Log &
        data(
                ::std::int64_t value
            );

    /**
        Update the pre-defined attribute 'data1'.
    */
    public : Log &
        data1(
                ::std::string const & inValue
            )
            {
                att_s("data1", inValue);
                return *this;
            }

    /**
        Update the pre-defined attribute 'file'.
    */
    public : Log &
        file(
                ::std::string_view const & inValue
            )
            {
                att_s("file", inValue);
                return *this;
            }

    /**
        Update the pre-defined attribute 'path'.
    */
    public : Log &
        path(
                ::fs::path const & inValue
            )
            {
                att_s("path", inValue.u8string());
                return *this;
            }
    public : Log &
        path1(
                ::fs::path const & inValue
            )
            {
                att_s("path1", inValue.u8string());
                return *this;
            }


    public : ::std::string
        message() const
            {
                return attribute("message").value_or(::std::string());
            }

    public : ::std::string
        message_resolved() const
            {
                return resolved(attribute("message").value_or(::std::string()));
            }

    /**
        Update the pre-defined attribute 'message'.
    */
    public : Log &
        message(
                ::std::string_view const & inValue
            )
            {
                att_s("message", inValue);
                return *this;
            }

    /**
        Update the pre-defined attribute 'code_file'.
    */
    public : Log &
        code_file(
                ::std::string_view const & inValue
            )
            {
                att_s("code_file", inValue);
                return *this;
            }
    /**
        Update the pre-defined attribute 'code_line'.
    */
    public : inline Log &
        code_line(
                int line
            );

    /**
        Update the pre-defined attribute 'code_function'.
    */
    public : Log &
        code_function(
                ::std::string_view const & inValue
            )
            {
                att_s("code_function", inValue);
                return *this;
            }
    /**
        Update the pre-defined attribute 'code_expression'.
    */
    public : Log &
        code_expression(
                ::std::string_view const & inValue
            )
            {
                att_s("code_expression", inValue);
                return *this;
            }
//@}


////////////////////////////////////////////////////////////////////////////////
/** \name Serialisation
@{*/

    public : enum class
        Format
            {
                TEXT
            ,   JSON
            ,   XML
            };

    /**
        Serialize the Log.

        \param  format    The format to convert to.
        \param  indent    (Optional) If -1, no newline characters or
            indending whitespaces are inserted. If >=0, the output gets
            formatted with the given base indent.

        \return
        The resulting text.
    */
    public : ::std::string
        serialize(
                Format  format
            ,   int     indent = -1
            ) const;

    /**
        Remove all attributes and add the attributes that are stored in the
        given text.

        \param  inStream    The text to parse.
        \param  inFormat    The format of the text.

        \return
        The success status.
    */
    public : static ::std::optional<Log>
        deserialize(
                ::std::string  const & data
            ,   Format                 format
            );
//@}


////////////////////////////////////////////////////////////////////////////////
/** \name Consumer
@{*/
    /**
        Objects of this class own the registration of a log consumer and
        get returned by consumer_register() for later disposal.
        \see https://en.wikipedia.org/wiki/Dispose_pattern
    */
    public : class
        ConsumerRegistrationDisposer
            {
                friend class Log;

                private :
                    ConsumerRegistrationDisposer(
                            unsigned int id
                        );
                public :
                    ~ConsumerRegistrationDisposer();

                private : ::std::optional<int>
                    m_id;
                public : void
                    dispose();
            };

    public : using
        consumer_guard_t = ::std::shared_ptr<ConsumerRegistrationDisposer>;

    public : static consumer_guard_t
        consumer_register(
                ::std::function<void(Log &)> const & func
            );

    public : static void
        consumers_force_dispose_all();
//@}
};


////////////////////////////////////////////////////////////////////////////////
//  template specialisations
//
template<> inline
    Log &
        Log::version<::std::string>(
                ::std::string const & v
            )
            {
                return version_(v);
            }

template<> inline
    Log &
        Log::att<::std::string>(
                ::std::string_view const & key
            ,   ::std::string      const & value
            )
            {
                return att_s(key, value);
            }

template<> inline
    Log &
        Log::att<char const *>(
                ::std::string_view  const & key
            ,   char const        * const & value
            )
            {
                return att_s(key, value);
            }

template<> inline
    Log &
        Log::att<char *>(
                ::std::string_view  const & key
            ,   char              * const & value
            )
            {
                return att_s(key, value);
            }

////////////////////////////////////////////////////////////////////////////////
//  inline methods that require template specialisations
//

    inline Log &
        Log::count(
                ::std::int64_t const & value
            )
            {
                att("count", value);
                return *this;
            }

    inline Log &
        Log::count1(
                ::std::int64_t const & value
            )
            {
                att("count1", value);
                return *this;
            }

    inline Log &
        Log::data(
                ::std::int64_t value
            )
            {
                att("data", value);
                return *this;
            }

    inline Log &
        Log::code_line(
                int line
            )
            {
                att("code_line", line);
                return *this;
            }


////////////////////////////////////////////////////////////////////////////////

#if !(defined(__GNUG__) && defined WIN32)

/**
    Read Logs from a target file.

    \param path Path of the target file.

    \return The logs read from the target file.
*/
extern ::std::vector<Log>
    log_read(
            ::fs::path const & path
        );

#endif


::std::string
    to_string(
            Log::Level level
        );

::std::optional<Log::Level>
    level_from_string(
            ::std::string_view str
        );

/** Flood prevention filter.
    This function routes Logs to a log consumer function.
    If some bandwith limit is reached the Logs of the affected creator are no
    longer routed but ignored until bandwith is available again.

    \see <issue915> telemetry flood prevention

    example use:

    auto
        logDisposer_network = Log::consumer_register(
                ::std::bind(
                        ::nsBase::log_filter_anti_flood
                    ,   ::telemetry::broadcast::send
                    ,   10   // report up to this count events
                    ,   60s  // per this duration
                    ,   _1
                    )
            );
*/
void
    log_filter_anti_flood(
            ::std::function<void(Log &)>  const & log_consumer
        ,   int                                   max_event_count_per_creator_per_duration
        ,   ::std::chrono::milliseconds           duration
        ,   Log                                 & log
        );
}
