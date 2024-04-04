#pragma once

#include "MyCustomControl.g.h"

namespace winrt::TestNuget::implementation
{
    struct MyCustomControl : MyCustomControlT<MyCustomControl>
    {
        MyCustomControl();

        int32_t MyProperty();
        void MyProperty(int32_t value);
    };
}

namespace winrt::TestNuget::factory_implementation
{
    struct MyCustomControl : MyCustomControlT<MyCustomControl, implementation::MyCustomControl>
    {
    };
}
