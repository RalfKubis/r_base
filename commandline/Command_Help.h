#pragma once
// Copyright (C) Ralf Kubis

#include "r_base/commandline/Command.h"

namespace nsBase::commandline
{

class Command_Help
:   public Command
{
    public  : R_DTOR_(Command_Help) = default;
    private : R_CTOR_(Command_Help) = default;
    private : R_CCPY_(Command_Help) = delete;
    private : R_CMOV_(Command_Help) = delete;
    private : R_COPY_(Command_Help) = delete;
    private : R_MOVE_(Command_Help) = delete;

    private : static command_ref_t
        factory();

    public : static void
        registerMe();

////////////////////////////////////////////////////////////////////////////////
/** \name base
@{*/
    public : virtual ::std::string_view
        helpMessageBrief() override;

    public : virtual ::std::string_view
        helpMessageAttributes() override;

    public : virtual void
        execute();

    public : virtual ::std::string
        name() const override
            {
                return "help";
            }
//@}
};

}
