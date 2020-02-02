#include "r_base/commandline/Command.h"

#include "r_base/vector.h"
#include "r_base/Error.h"

#include <fmt/format.h>

#include <map>


namespace nsBase::commandline
{

using namespace ::std::string_literals;


namespace
{
// [command name]->[factory]
::std::map<::std::string, command_factory_t>
    command_factories;

command_resolve_t
    resolver;
}



void
registerFactory(
    ::std::string     const & command_name
,   command_factory_t         factory
)
{
    DBC_PRE(!map_contains(command_factories,command_name));
    DBC_PRE(!command_name.empty());
    DBC_PRE(factory);

    command_factories.emplace(command_name, ::std::move(factory));
}


void
registerResolver(
    command_resolve_t resolver_
)
{
    DBC_PRE(!resolver);
    resolver = resolver_;
}


::std::string
resolved(
    ::std::string const & v
)
{
    if (resolver)
        return resolver(v);
    return v;
}



command_ref_t
create(
    ::std::string const & command_name
)
{
    auto it = command_factories.find(command_name);

    if (it==command_factories.end())
    {
        throw Error(Log(u8"550f5815-4137-4199-a59d-811b38759bb5"_uuid)
            .message("Unknown command '${data}'")
            .data(command_name)
            );
    }

    if (!it->second)
    {
        throw Error(Log(u8"72a8c87f-4af8-4d5c-94a0-06eef5c4f98d"_uuid)
            .message("Have no handler for command '${data}'")
            .data(command_name)
            );
    }

    if (auto cmd=it->second())
        return cmd;

    throw Error(Log(u8"8fd5f9a6-2522-4190-8d7f-876d211e8d11"_uuid)
        .message("Command '${data}' can't get handled")
        .data(command_name)
        );
}



Command::attribute_refs_t
Command::attributes(
    ::std::optional<::std::string> const & attribute_name
) const
{
    attribute_refs_t
        aaa;

    ::std::copy_if(
            m_attributes.begin()
        ,   m_attributes.end()
        ,   ::std::back_inserter(aaa)
        ,   [&](auto & att){return !attribute_name || *attribute_name==att->name();}
        );

    return aaa;
}


Command::attribute_ref_t
Command::attribute1(
    ::std::string const & attribute_name
,   bool                  is_required
) const
{
    auto
        aaa = attributes(attribute_name);

    if (is_required)
    {
        if (aaa.size()!=1)
            throw Error(Log(u8"44f2fa52-ac08-4af9-908c-3e8a8b5d5815"_uuid)
                .message("The attribute '${data}' must occur exactly once.")
                .data(attribute_name)
                );
    }
    else
    {
        if (aaa.size()>1)
            throw Error(Log(u8"633b6592-c7d4-4a4c-a11e-455f534f1d45"_uuid)
                .message("The attribute '${data}' must not occur more than once.")
                .data(attribute_name)
                );
    }

    if (aaa.empty())
        return {};

    return aaa.front();
}


void
Command::attributeAdd(
    ::std::string const & attribute_name
,   ::std::string const & attribute_value
)
{
    auto
        a = ::std::make_shared<Attribute>();
        a->name_assign(attribute_name);
        a->value_assign(attribute_value);

    m_attributes.emplace_back(::std::move(a));
}


::std::string
usage(
    ::std::string const & command_name
,   bool                  stream_to_stdout
)
{
    ::std::string
        retVal;

    BLOCK
    {
        if (auto cmd = create(command_name))
        {
            retVal = ::fmt::format(
                "\n"
                "--{}\n"
                "\n"
                "    {}\n"
                "\n"
                "{}\n"
                ,   command_name
                ,   cmd->helpMessageBrief()
                ,   cmd->helpMessageAttributes()
                );

            LEAVE;
        }

        // default message
        ::std::string cmdList;
        ::std::string cmdListDetails;

        for (auto const & [name,factory] : command_factories)
        {
            if (!cmdList.empty())
                cmdList += u8" | "s;
            cmdList += name;

            auto cmd = factory();

            cmdListDetails += ::fmt::format(
                    "\n"
                    "   --{}\n"
                    "\n"
                    "       {}\n"
                ,   name
                ,   cmd->helpMessageBrief()
                );
        }

        retVal = ::fmt::format(
                "\n"
                "Usage: {0} { '--'Command {Attribute} }\n"
                "\n"
                "       Command : {1}\n"
                "\n"
                "       Attribute : [\"][attributeName'=']attributeValue[\"]\n"
                "\n"
                "           If the values contain whitespaces, the whole attribute must\n"
                "           be enclosed with quotes.\n"
                "           Quotes in the value must be escaped with a backslash ( \\\" ).\n"
                "\n"
                "\n"
                "   Possible commands are:\n"
                "{2}"
                "\n"
                "\n"
                "   To get detailed information for a command (like for example 'mycommand')\n"
                "   use:\n"
                "\n"
                "       > {0} --help command=mycommand\n"
            ,   "program.exe"
            ,   cmdList
            ,   cmdListDetails
            );
    }
    FIN

    if (stream_to_stdout)
        ::fmt::print("\n{}\n", retVal);

    return retVal;
}


commands_t
parse(
    ::std::vector<::std::string> const & args_
)
{
    commands_t
        ccc;

    auto
        args = args_;

    // trim / un-quote / un-escape
    for (auto & a : args)
    {
        trim_in_place(a);

        auto isStillQuoted =
                a.size()>=2
            &&  a.front()=='"'
            &&  a.back()=='"';
            ;

        if (!isStillQuoted)
            continue;

        // remove quotes
        a = a.substr(1,a.size()-2);

        // un-escape
        a = replaced_all(a,"\\\"","\"");
    }

    ::std::reverse(args.begin(), args.end());

    while (!args.empty())
    {
        auto
            command_name(::std::move(args.back()));

        args.pop_back();

        // parse command name
        if (command_name.find_first_of("--")!=0)
        {
            throw Error(Log(u8"b3794764-732c-45e3-bfda-94a2d454b00b"_uuid)
                .message("parse error near '${data}'")
                .data(command_name)
                );
        }

        // cut away "--"
        command_name = command_name.substr(2);

        // new command
        auto
            command = create(command_name);

        ccc.push_back(command);

        // parse attributes
        while (
                !args.empty()
            &&  args.back().find_first_of("--")!=0
        )
        {
            auto
                attribute(::std::move(args.back()));
            args.pop_back();

            auto
                posOfEqualSign = attribute.find_first_of('=');

            if (posOfEqualSign==0 || posOfEqualSign==::std::string::npos)
                throw Error(Log(u8"fc5fd1f9-272f-4853-8384-9c99ff822f35"_uuid)
                    .message("parse error near '${data}'")
                    .data(attribute)
                    );

            command->attributeAdd(
                    attribute.substr(0,posOfEqualSign)
                ,   attribute.substr(posOfEqualSign+1)
                );
        }

        // execute early pass
        command->execute_early();

    } // while

    return ccc;
}


::std::string
Command::Attribute::name() const
{
    return resolved(m_name);
}


void
Command::Attribute::name_assign(
    ::std::string const & v
)
{
    m_name = v;
}


::std::string
Command::Attribute::value() const
{
    return resolved(m_value);
}


void
Command::Attribute::value_assign(
    ::std::string const & v
)
{
    m_value = v;
}

}
