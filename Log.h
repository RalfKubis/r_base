#pragma once
/* Copyright (C) Ralf Kubis */

#include "r_base/decl.h"
#include "r_base/dbc.h"
#include "r_base/filesystem.h"
#include "r_base/uuid.h"
#include "r_base/time.h"
#include "r_base/current.h"

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

        "EED119DA-075A-4b3d-B152-2A3651FEB351"_log()
            .info()
            .scope("foobar()")
            ("shader initialisation")
            ("filePath", filepath)
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

    /** This Status is equivalent to ::grpc::StatusCode.
        Using this, components (which are nout coupled to grpc) can
        communicate the status of their operations in a compatible manner.
    */
    public : enum class Status
        {
            /// Not an error; returned on success.
            OK = 0

            /// The operation was cancelled (typically by the caller).
        ,   CANCELLED = 1

            /// Unknown error. An example of where this error may be returned is if a
            /// Status value received from another address space belongs to an error-space
            /// that is not known in this address space. Also errors raised by APIs that
            /// do not return enough error information may be converted to this error.
        ,   UNKNOWN = 2

            /// Client specified an invalid argument. Note that this differs from
            /// FAILED_PRECONDITION. INVALID_ARGUMENT indicates arguments that are
            /// problematic regardless of the state of the system (e.g., a malformed file
            /// name).
        ,   INVALID_ARGUMENT = 3

            /// Deadline expired before operation could complete. For operations that
            /// change the state of the system, this error may be returned even if the
            /// operation has completed successfully. For example, a successful response
            /// from a server could have been delayed long enough for the deadline to
            /// expire.
        ,   DEADLINE_EXCEEDED = 4

            /// Some requested entity (e.g., file or directory) was not found.
        ,   NOT_FOUND = 5

            /// Some entity that we attempted to create (e.g., file or directory) already
            /// exists.
        ,   ALREADY_EXISTS = 6

            /// The caller does not have permission to execute the specified operation.
            /// PERMISSION_DENIED must not be used for rejections caused by exhausting
            /// some resource (use RESOURCE_EXHAUSTED instead for those errors).
            /// PERMISSION_DENIED must not be used if the caller can not be identified
            /// (use UNAUTHENTICATED instead for those errors).
        ,   PERMISSION_DENIED = 7

            /// The request does not have valid authentication credentials for the
            /// operation.
        ,   UNAUTHENTICATED = 16

            /// Some resource has been exhausted, perhaps a per-user quota, or perhaps the
            /// entire file system is out of space.
        ,   RESOURCE_EXHAUSTED = 8

            /// Operation was rejected because the system is not in a state required for
            /// the operation's execution. For example, directory to be deleted may be
            /// non-empty, an rmdir operation is applied to a non-directory, etc.
            ///
            /// A litmus test that may help a service implementor in deciding
            /// between FAILED_PRECONDITION, ABORTED, and UNAVAILABLE:
            ///  (a) Use UNAVAILABLE if the client can retry just the failing call.
            ///  (b) Use ABORTED if the client should retry at a higher-level
            ///      (e.g., restarting a read-modify-write sequence).
            ///  (c) Use FAILED_PRECONDITION if the client should not retry until
            ///      the system state has been explicitly fixed. E.g., if an "rmdir"
            ///      fails because the directory is non-empty, FAILED_PRECONDITION
            ///      should be returned since the client should not retry unless
            ///      they have first fixed up the directory by deleting files from it.
            ///  (d) Use FAILED_PRECONDITION if the client performs conditional
            ///      REST Get/Update/Delete on a resource and the resource on the
            ///      server does not match the condition. E.g., conflicting
            ///      read-modify-write on the same resource.
        ,   FAILED_PRECONDITION = 9

            /// The operation was aborted, typically due to a concurrency issue like
            /// sequencer check failures, transaction aborts, etc.
            ///
            /// See litmus test above for deciding between FAILED_PRECONDITION, ABORTED,
            /// and UNAVAILABLE.
        ,   ABORTED = 10

            /// Operation was attempted past the valid range. E.g., seeking or reading
            /// past end of file.
            ///
            /// Unlike INVALID_ARGUMENT, this error indicates a problem that may be fixed
            /// if the system state changes. For example, a 32-bit file system will
            /// generate INVALID_ARGUMENT if asked to read at an offset that is not in the
            /// range [0,2^32-1], but it will generate OUT_OF_RANGE if asked to read from
            /// an offset past the current file size.
            ///
            /// There is a fair bit of overlap between FAILED_PRECONDITION and
            /// OUT_OF_RANGE. We recommend using OUT_OF_RANGE (the more specific error)
            /// when it applies so that callers who are iterating through a space can
            /// easily look for an OUT_OF_RANGE error to detect when they are done.
        ,   OUT_OF_RANGE = 11

            /// Operation is not implemented or not supported/enabled in this service.
        ,   UNIMPLEMENTED = 12

            /// Internal errors. Means some invariants expected by underlying System has
            /// been broken. If you see one of these errors, Something is very broken.
        ,   INTERNAL = 13

            /// The service is currently unavailable. This is a most likely a transient
            /// condition and may be corrected by retrying with a backoff. Note that it is
            /// not always safe to retry non-idempotent operations.
            ///
            /// \warning Although data MIGHT not have been transmitted when this
            /// status occurs, there is NOT A GUARANTEE that the server has not seen
            /// anything. So in general it is unsafe to retry on this status code
            /// if the call is non-idempotent.
            ///
            /// See litmus test above for deciding between FAILED_PRECONDITION, ABORTED,
            /// and UNAVAILABLE.
        ,   UNAVAILABLE = 14

            /// Unrecoverable data loss or corruption.
        ,   DATA_LOSS = 15
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
    public : ~Log();
    public : Log();
//  public : Log(Log const & src); // defined below
    public : Log(Log && src);
    public : Log & operator=(Log const & src) = delete;
    public : Log & operator=(Log && src);


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


    /** Copy-Constructor is explicit.
        The clone is disarmed (do_broadcast==false).
    */
    public : explicit
        Log(
                Log const &
            );

    /** helper to create a deep copy
    */
    private :
        Log(
                class Log_Impl const &
            );

//@}

    public : Log
        copy() const;

    public : Log &&
        move();

    /** If TRUE, the instance gets broadcasted to the registered consumers prior
        to its destruction or on an explict broadcast attempt.
    */
    public : bool
        do_broadcast() const;
    public : Log &
        do_broadcast_assign(bool val);
    public : Log &
        arm()
            {
                return do_broadcast_assign(true);
            }
    public : Log &
        disarm()
            {
                return do_broadcast_assign(false);
            }

    /** If armed, broadcast and disarm the instance.
    */
    public : void
        broadcast_if();


    /** If applicable, broadcast the instance.
        Clear the instance to its initial state.
        The creator-id gets set to EMPTY.
    */
    public : void
        broadcast_if_and_clear();


    public : [[noreturn]] void
        throw_error();


////////////////////////////////////////////////////////////////////////////////
/** \name Properties
@{*/
    public : ::uuids::uuid const &
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



    public : Log::Status
        status() const;
    public : Log &
        status(Log::Status);
    public : bool
        ok() const
            {
                return status()==Status::OK;
            }

    public : [[noreturn]] void throw_CANCELLED            (){status(Status::CANCELLED          ).throw_error();}
    public : [[noreturn]] void throw_UNKNOWN              (){status(Status::UNKNOWN            ).throw_error();}
    public : [[noreturn]] void throw_INVALID_ARGUMENT     (){status(Status::INVALID_ARGUMENT   ).throw_error();}
    public : [[noreturn]] void throw_DEADLINE_EXCEEDED    (){status(Status::DEADLINE_EXCEEDED  ).throw_error();}
    public : [[noreturn]] void throw_NOT_FOUND            (){status(Status::NOT_FOUND          ).throw_error();}
    public : [[noreturn]] void throw_ALREADY_EXISTS       (){status(Status::ALREADY_EXISTS     ).throw_error();}
    public : [[noreturn]] void throw_PERMISSION_DENIED    (){status(Status::PERMISSION_DENIED  ).throw_error();}
    public : [[noreturn]] void throw_UNAUTHENTICATED      (){status(Status::UNAUTHENTICATED    ).throw_error();}
    public : [[noreturn]] void throw_RESOURCE_EXHAUSTED   (){status(Status::RESOURCE_EXHAUSTED ).throw_error();}
    public : [[noreturn]] void throw_FAILED_PRECONDITION  (){status(Status::FAILED_PRECONDITION).throw_error();}
    public : [[noreturn]] void throw_ABORTED              (){status(Status::ABORTED            ).throw_error();}
    public : [[noreturn]] void throw_OUT_OF_RANGE         (){status(Status::OUT_OF_RANGE       ).throw_error();}
    public : [[noreturn]] void throw_UNIMPLEMENTED        (){status(Status::UNIMPLEMENTED      ).throw_error();}
    public : [[noreturn]] void throw_INTERNAL             (){status(Status::INTERNAL           ).throw_error();}
    public : [[noreturn]] void throw_UNAVAILABLE          (){status(Status::UNAVAILABLE        ).throw_error();}
    public : [[noreturn]] void throw_DATA_LOSS            (){status(Status::DATA_LOSS          ).throw_error();}


    public : Log & CANCELLED            (){return status(Status::CANCELLED          );}
    public : Log & UNKNOWN              (){return status(Status::UNKNOWN            );}
    public : Log & INVALID_ARGUMENT     (){return status(Status::INVALID_ARGUMENT   );}
    public : Log & DEADLINE_EXCEEDED    (){return status(Status::DEADLINE_EXCEEDED  );}
    public : Log & NOT_FOUND            (){return status(Status::NOT_FOUND          );}
    public : Log & ALREADY_EXISTS       (){return status(Status::ALREADY_EXISTS     );}
    public : Log & PERMISSION_DENIED    (){return status(Status::PERMISSION_DENIED  );}
    public : Log & UNAUTHENTICATED      (){return status(Status::UNAUTHENTICATED    );}
    public : Log & RESOURCE_EXHAUSTED   (){return status(Status::RESOURCE_EXHAUSTED );}
    public : Log & FAILED_PRECONDITION  (){return status(Status::FAILED_PRECONDITION);}
    public : Log & ABORTED              (){return status(Status::ABORTED            );}
    public : Log & OUT_OF_RANGE         (){return status(Status::OUT_OF_RANGE       );}
    public : Log & UNIMPLEMENTED        (){return status(Status::UNIMPLEMENTED      );}
    public : Log & INTERNAL             (){return status(Status::INTERNAL           );}
    public : Log & UNAVAILABLE          (){return status(Status::UNAVAILABLE        );}
    public : Log & DATA_LOSS            (){return status(Status::DATA_LOSS          );}

    public : ::uuids::uuid const &
        application() const;
    public : Log &
        application(::uuids::uuid const &);


    public : ::uuids::uuid const &
        application_instance() const;
    public : Log &
        application_instance(::uuids::uuid const &);


    public : ::std::string const &
        version() const;
    public : Log &
        version(::std::string_view const &);


    public : ::uuids::uuid const &
        session() const;
    public : Log &
        session(::uuids::uuid const &);


    public : ::uuids::uuid const &
        creator() const;
    public : Log &
        creator(::uuids::uuid const &);


    public : ::uuids::uuid const &
        event() const;
    public : Log &
        event(::uuids::uuid const &);


    public : ::nsBase::time::time_point_t const &
        time() const;
    public : Log &
        time(::nsBase::time::time_point_t const &);


    public : ::std::string const &
        host() const;
    public : Log &
        host(::std::string_view const &);


    public : ::std::string const &
        user() const;
    public : Log &
        user(::std::string_view const &);


    public : ::std::string const &
        thread() const;
    public : Log &
        thread(::std::string_view const &);


    public : ::std::string const &
        scope() const;
    public : Log &
        scope(::std::string_view const &);

    public : Log &
        operator[](
                ::std::string_view const & s
            )
            {
                return scope(s);
            }

    public : ::std::string const &
        message() const;
    public : Log &
        message(::std::string_view const &);


    public : Log &
        property(
                ::std::string_view const & key
            ,   ::std::string_view const & value
            );

    public : ::std::optional<::std::string>
        property(
                ::std::string_view const & key
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
        trace(::std::string_view const &);
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
    private : Log &
        att_s(
                ::std::string_view const & key
            ,   ::std::string_view const & value
            );

    private : template<typename val_t> Log &
        att(
                ::std::string_view  const & key
            ,   val_t               const & value
            )
            {
                return att_s(key, ::std::to_string(value));
            }

    public : template<typename val_t> Log &
        operator()(
                ::std::string_view     const & key
            ,   ::std::optional<val_t> const & value
            );

    public : template<typename val_t> Log &
        operator()(
                ::std::string_view     const & key
            ,   val_t                  const * value
            );

    public : template<typename val_t> Log &
        operator()(
                ::std::string_view              const & key
            ,   ::std::reference_wrapper<val_t> const & value
            );

    public : Log &
        operator()(
                ::std::string_view const & key
            ,   ::uuids::uuid      const & value
            )
            {
                return operator()(key, to_string(value));
            }

    public : Log &
        operator()(
                ::std::string_view           const & key
            ,   ::nsBase::time::time_point_t const & value
            )
            {
                return operator()(key, to_string_iso_utc(value));
            }

    public : Log &
        operator()(
                ::std::string_view              const & key
            ,   ::nsBase::time::time_duration_t const & value
            )
            {
                return operator()(key, to_string_HH_mm_ss(value, true));
            }

    public : Log &
        operator()(
                ::std::string_view           const & key
            ,   ::nsBase::time::date_t       const & value
            )
            {
                return operator()(key, to_string(value));
            }

    public : template<typename val_t> Log &
        operator()(
                ::std::string_view const & key
            ,   val_t              const & value
            );


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
                ::std::string_view const & s
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
                ::std::string_view const & s
            ,   ::std::string_view const & key
            ,   ::std::string_view const & value
            ) const;
//@}

    public : ::std::map<::std::string,::std::string>
        properties_and_attributes() const;

////////////////////////////////////////////////////////////////////////////////
/** \name Helper for common dynamic attributes
@{*/

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
                ::std::string_view const & inValue
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
                att_s("path", P2S(inValue));
                return *this;
            }
    public : Log &
        path1(
                ::fs::path const & inValue
            )
            {
                att_s("path1", P2S(inValue));
                return *this;
            }


    public : ::std::string
        message_resolved() const
            {
                return resolved(message());
            }


    public : Log &
        operator()(
                ::std::string_view const & inValue
            )
            {
                message(inValue);
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

    /** Serialize the Log into a single-lined JSON object.
    */
    public : ::std::string
        serialize(
                bool pretty = {}
            ) const;

    /** Read a log from a JSON-object.
    */
    public : static ::std::optional<Log>
        deserialize(
                ::std::string_view const & data
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
//  template definitions/specialisations
//
template<> inline
    Log &
        Log::att<::std::string_view>(
                ::std::string_view const & key
            ,   ::std::string_view const & value
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

template<> inline
    Log &
        Log::att<::std::string>(
                ::std::string_view  const & key
            ,   ::std::string       const & value
            )
            {
                return att_s(key, value);
            }



template<typename val_t>
    Log &
        Log::operator()(
                ::std::string_view     const & key
            ,   ::std::optional<val_t> const & value
            )
            {
                if (value)
                    return operator()(key, *value);
                else
                    return operator()(key, "<nullopt>");
            }

template<typename val_t>
    Log &
        Log::operator()(
                ::std::string_view     const & key
            ,   val_t                  const * value
            )
            {
                if (value)
                    return operator()(key, *value);
                else
                    return operator()(key, "<null>");
            }

template<> inline
    Log &
        Log::operator()<char>(
                ::std::string_view     const & key
            ,   char                   const * value
            )
            {
                if (value)
                    return att_s(key, value);
                else
                    return operator()(key, "<null>");
            }

template<typename val_t>
    Log &
        Log::operator()(
                ::std::string_view              const & key
            ,   ::std::reference_wrapper<val_t> const & value
            )
            {
                return operator()(key, value.get());
            }

template<typename val_t>
    Log &
        Log::operator()(
                ::std::string_view const & key
            ,   val_t              const & value
            )
            {
                return att(key, value);
            }


////////////////////////////////////////////////////////////////////////////////
//  inline methods that require template definitions/specialisations
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

/**
    Read Logs from a target file.

    \param path Path of the target file.

    \return The logs read from the target file.
*/
::std::vector<Log>
    log_read(
            ::fs::path const & path
        );

void
    logs_read(
            ::std::vector<Log> & target
        ,   ::fs::path   const & path
        );


::std::string
    to_string(
            Log::Level
        );

::std::optional<Log::Level>
    level_from_string(
            ::std::string_view
        );


::std::string
    to_string(
            Log::Status
        );

::std::optional<Log::Status>
    status_from_string(
            ::std::string_view
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


namespace nsBase
{

struct Log_maker
{
    ::uuids::uuid u;

    ::nsBase::Log
        operator()()
            {
                return {u};
            }


    public : ::nsBase::Log
        operator()(
                ::std::string_view const & message
            )
            {
                auto
                    l = ::nsBase::Log{u};
                    l(message);

                return l;
            }

    public : ::nsBase::Log
        operator[](
                ::std::string_view const & scope
            )
            {
                auto
                    l = ::nsBase::Log{u};
                    l[scope];

                return l;
            }
};


/** Helper to log an application instance execution start and end.
    The function immediately emits the start-log and returns the exit-log for its scoped destruction.
*/
Log
    log_application_execution_span(
            Log && pattern
        );

}

consteval auto
    operator "" _log(
            char        const * data
        ,   ::std::size_t       len
        )
        ->  ::nsBase::Log_maker
        {
            auto is_digit = [](char const c){return (c>='0' && c<='9') || (c>='a' && c<='f');};
            auto is_minus = [](char const c){return  c=='-';};

            // if this fails, the UUID string is wrong sized
            len /= static_cast<::std::size_t>(len==36);
            len /= static_cast<::std::size_t>(
                    is_digit(data[ 0])
                &&  is_digit(data[ 1])
                &&  is_digit(data[ 2])
                &&  is_digit(data[ 3])
                &&  is_digit(data[ 4])
                &&  is_digit(data[ 5])
                &&  is_digit(data[ 6])
                &&  is_digit(data[ 7])

                &&  is_minus(data[ 8])

                &&  is_digit(data[ 9])
                &&  is_digit(data[10])
                &&  is_digit(data[11])
                &&  is_digit(data[12])

                &&  is_minus(data[13])

                &&  is_digit(data[14])
                &&  is_digit(data[15])
                &&  is_digit(data[16])
                &&  is_digit(data[17])

                &&  is_minus(data[18])

                &&  is_digit(data[19])
                &&  is_digit(data[20])
                &&  is_digit(data[21])
                &&  is_digit(data[22])

                &&  is_minus(data[23])

                &&  is_digit(data[24])
                &&  is_digit(data[25])
                &&  is_digit(data[26])
                &&  is_digit(data[27])
                &&  is_digit(data[28])
                &&  is_digit(data[29])
                &&  is_digit(data[30])
                &&  is_digit(data[31])
                &&  is_digit(data[32])
                &&  is_digit(data[33])
                &&  is_digit(data[34])
                &&  is_digit(data[35])
                );

            auto u = ::uuids::uuid::from_string(::std::string_view{data, len});

            // if this fails, the UUID string is otherwise ill-formed
            len /= static_cast<::std::size_t>(u.has_value());

            return {u.value()};
        }
