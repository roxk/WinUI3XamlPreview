#pragma once

#include "DepCustomControl.g.h"

namespace winrt::TestDepOfDep::implementation
{
    struct DepCustomControl : DepCustomControlT<DepCustomControl>
    {
        DepCustomControl();
    };
}

namespace winrt::TestDepOfDep::factory_implementation
{
    struct DepCustomControl : DepCustomControlT<DepCustomControl, implementation::DepCustomControl>
    {
    };
}
