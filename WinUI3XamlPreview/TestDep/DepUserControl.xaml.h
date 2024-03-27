#pragma once

#include "DepUserControl.g.h"

namespace winrt::TestDep::implementation
{
    struct DepUserControl : DepUserControlT<DepUserControl>
    {
        DepUserControl()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void myButton_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::TestDep::factory_implementation
{
    struct DepUserControl : DepUserControlT<DepUserControl, implementation::DepUserControl>
    {
    };
}
