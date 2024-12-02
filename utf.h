#pragma once
/* Copyright (C) Ralf Kubis */

#include <string>
#include <locale>

namespace nsBase
{

::std::string   wstring_to_utf8(::std::wstring const &);
::std::wstring  utf8_to_wstring(::std::string  const &);

}
