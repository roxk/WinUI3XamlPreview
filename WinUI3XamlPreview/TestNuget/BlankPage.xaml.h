#pragma once

#include "BlankPage.g.h"

namespace winrt::TestNuget::implementation
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

namespace winrt::TestNuget::factory_implementation
{
    struct BlankPage : BlankPageT<BlankPage, implementation::BlankPage>
    {
    };
}
