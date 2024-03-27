#pragma once

#include "DepCustomControl.g.h"

namespace winrt::TestDep::implementation
{
    struct DepCustomControl : DepCustomControlT<DepCustomControl>
    {
        DepCustomControl();

        int32_t MyProperty();
        void MyProperty(int32_t value);
    };
}

namespace winrt::TestDep::factory_implementation
{
    struct DepCustomControl : DepCustomControlT<DepCustomControl, implementation::DepCustomControl>
    {
    };
}
