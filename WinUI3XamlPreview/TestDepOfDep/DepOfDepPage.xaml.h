#pragma once

#include "DepOfDepPage.g.h"

namespace winrt::TestDepOfDep::implementation
{
    struct DepOfDepPage : DepOfDepPageT<DepOfDepPage>
    {
        DepOfDepPage()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }
    };
}

namespace winrt::TestDepOfDep::factory_implementation
{
    struct DepOfDepPage : DepOfDepPageT<DepOfDepPage, implementation::DepOfDepPage>
    {
    };
}
