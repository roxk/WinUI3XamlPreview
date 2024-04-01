#pragma once

#include "BlankPage.g.h"

namespace winrt::TestDep::implementation
{
    struct BlankPage : BlankPageT<BlankPage>
    {
        BlankPage() 
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }
    };
}

namespace winrt::TestDep::factory_implementation
{
    struct BlankPage : BlankPageT<BlankPage, implementation::BlankPage>
    {
    };
}
