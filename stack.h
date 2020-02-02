#pragma once
/* Copyright (C) Ralf Kubis */


#include "quancept/ast_to_model/decl.h"
#include "quancept/model/Context.h"

#include <stack>
#include <vector>


namespace nsBase
{

template<class T>
    class
        Popper
            {
                private : ::std::vector<T> &
                    m_stack;

                private : bool
                    m_pushed = false;

                public :
                    Popper(
                            ::std::vector<T> & stack
                        )
                        :   m_stack(stack)
                        {
                        }

                public : void
                    push(
                            T const & element
                        )
                        {
                            if (m_pushed)
                                throw Error(Log(u8"10349cd0-af2b-4fdb-b6f7-c6c2f8220a74"_uuid));

                            m_stack.push_back(element);
                            m_pushed = true;
                        }

                public :
                    ~Popper()
                        {
                            dispose();
                        }

                public : void
                    dispose()
                        {
                            if (m_pushed)
                            {
                                m_stack.pop_back();
                                m_pushed = false;
                            }
                        }
            };
}
