#include "r_base/commandline/Command_Help.h"


namespace nsBase::commandline
{

namespace
{
auto
sHelpMessageBrief =
"Display documentation text for a specific command."
;

auto
sHelpMessageAttributes =
"       attribute   : command\n"
"       occurrence  : once (required)\n"
"       values      : String\n"
"       default     : \n"
"           With this attribute one can select the command for which the\n"
"           documentation gets displayed.\n"
;
}


command_ref_t
Command_Help::factory()
{
    return command_ref_t(new Command_Help);
}


void
Command_Help::registerMe()
{
    registerFactory("help",factory);
}


::std::string_view
Command_Help::helpMessageAttributes()
{
    return sHelpMessageAttributes;
}


::std::string_view
Command_Help::helpMessageBrief()
{
    return sHelpMessageBrief;
}


void
Command_Help::execute()
{
    ::std::string
        command_name;

    if (auto attCommandName = attribute1("command", false))
        command_name = attCommandName->value();

    usage(command_name, true);
}

}
