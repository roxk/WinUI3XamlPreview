#pragma once

#include "App.xaml.g.h"
#include "MyCustomControl.h"

namespace winrt::TestNuget::implementation
{
    struct App : AppT<App>
    {
        App();

        winrt::fire_and_forget OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);

    private:
        winrt::Microsoft::UI::Xaml::Window window{ nullptr };
    };
}
