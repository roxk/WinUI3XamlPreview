#pragma once

#include "MyUserControl.g.h"

namespace winrt::TestNuget::implementation
{
    struct MyUserControl : MyUserControlT<MyUserControl>
    {
        MyUserControl()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }
    };
}

namespace winrt::TestNuget::factory_implementation
{
    struct MyUserControl : MyUserControlT<MyUserControl, implementation::MyUserControl>
    {
    };
}
