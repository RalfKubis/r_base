#pragma once
/* Copyright (C) Ralf Kubis */


namespace nsBase
{

/** helper to create visitors
    https://www.youtube.com/watch?v=EsUmnLgz8QY&t

    Example:

    ::std::visit(
            overloaded
            {
            //--------------------------------------------------------------------------------------
                [&](::std::monostate const & x)
                {
                }
            //--------------------------------------------------------------------------------------
            ,   [&](M::message::call_info_request const & x)
                {
                    *to.mutable_call_info_request() << x;
                }
            //--------------------------------------------------------------------------------------
            ,   [&](M::message::call_info_outcome const & x)
                {
                    *to.mutable_call_info_outcome() << x;
                }
            //--------------------------------------------------------------------------------------
            ,   [&](M::message::call_info_disposal const & x)
                {
                    *to.mutable_call_info_disposal() << x;
                }
            //--------------------------------------------------------------------------------------
            ,   [&](M::message::agent_changed const & x)
                {
                    *to.mutable_agent_changed() << x;
                }
            //--------------------------------------------------------------------------------------
            }
        ,   from
        );
*/
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

}
