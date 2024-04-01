#pragma once

#include "DepOfDepPage.g.h"

namespace winrt::TestDepOfDep::implementation
{
    struct DepOfDepPage : DepOfDepPageT<DepOfDepPage>
    {
        DepOfDepPage();
    };
}

namespace winrt::TestDepOfDep::factory_implementation
{
    struct DepOfDepPage : DepOfDepPageT<DepOfDepPage, implementation::DepOfDepPage>
    {
    };
}
