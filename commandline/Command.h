#pragma once
// Copyright (C) Ralf Kubis

#include "r_base/language_tools.h"
#include "r_base/string.h"
#include <optional>

#include <memory>
#include <vector>
#include <functional>
#include <string_view>


TODO("probably better use boost.program_options")

namespace nsBase::commandline
{

class Command
{
    R_VTOR(Command) = default;
    R_CTOR(Command) = default;
    R_CCPY(Command) = delete;
    R_CMOV(Command) = delete;
    R_COPY(Command) = delete;
    R_MOVE(Command) = delete;


/** \name Command Name
@{*/
    public : virtual ::std::string
        name() const = 0;
//@}


/** \name Attributes
@{*/
    public : class
        Attribute
            {
                private : ::std::string
                    m_name;

                public : ::std::string
                    name() const;

                public : void
                    name_assign(
                            ::std::string const & v
                        );


                private : ::std::string
                    m_value;

                public : ::std::string
                    value() const;

                public : void
                    value_assign(
                            ::std::string const & v
                        );
            };

    public : using attribute_t      = Attribute;
    public : using attribute_ref_t  = ::std::shared_ptr<attribute_t>;
    public : using attribute_refs_t = ::std::vector<attribute_ref_t>;

    /**
        Lookup all attributes with the given name.

        \param  attribute_name  Name of the attribute to lookup. If empty,
            all attributes are returned.

    */
    public : attribute_refs_t
        attributes(
                ::std::optional<::std::string> const & attribute_name = {}
            ) const;

    /**
        Lookup an attribute value that is allowed to occur only once.

        \param  attribute_name  Name of the attribute to lookup.
        \param  is_required     If TRUE, the attribute is required.

        \return
        OK if the attribute occurs not more than once or exactly once in case
        it is required, BAD otherwise.
    */
    public : attribute_ref_t
        attribute1(
                ::std::string const & attribute_name
            ,   bool                  is_required = true
            ) const;

    /**
        Append an attribute.
    */
    public : void
        attributeAdd(
                ::std::string const & attribute_name
            ,   ::std::string const & attribute_value
            );

    private : attribute_refs_t
        m_attributes;
//@}


/** \name Usage Message
@{*/
    public : virtual ::std::string_view
        helpMessageBrief() = 0;

    public : virtual ::std::string_view
        helpMessageAttributes() = 0;
//@}


/** \name Execution
@{*/
    /**
        This method is to be invoked after the command was completely set up.
        For example the command line parser is going to call this function.
        But also on manual command-injection this method is invoked.
        This gives a command the chance to perform early actions which might
        influence the main execution of the commands even of other commands
        that are specified earlier in the command stream.
        One possible action to be performed in this pass is to register commands
        that come with the plugin activated by the command 'plugin' which
        overrides this method.
        Then this "lazy-registered" commands are recognized by the parser
        when parsing of the remaining command stream.
    */
    public : virtual void
        execute_early()
            {
            }

    /**
        Perform the main action of the command.
    */
    public : virtual void
        execute() = 0;
//@}
};


using
    command_factory_t = ::std::function<::std::shared_ptr<Command>()>;
using
    command_resolve_t = ::std::function<::std::string(::std::string const &)>;
using
    command_ref_t = ::std::shared_ptr<Command>;
using
    commands_t = ::std::vector<command_ref_t>;


    /** Factory to make a command by its name.
        If no command producer is registered for this name, an exception is thrown.
    */
command_ref_t
    create(
            ::std::string const & command_name
        );

void
    registerFactory(
            ::std::string     const & command_name
        ,   command_factory_t         factory
        );

void
    registerResolver(
            command_resolve_t resolver
        );

::std::string
    resolved(
            ::std::string const & inoutExpression
        );

::std::string
    usage(
            ::std::string const & command_name
        ,   bool                  stream_to_stdout
        );

/**
    Parse a command line.
*/
commands_t
    parse(
            ::std::vector<::std::string> const & args
        );

}
