#pragma once

#include "Toast.g.h"

namespace winrt::WinUI3XamlPreview::implementation
{
    struct Toast : ToastT<Toast>, idlgen::author_class<muxc::UserControl>
    {
        Toast()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }
        void ShowError(winrt::hstring const& title, winrt::hstring const& message);
    private:
        winrt::fire_and_forget Hide(winrt::weak_ref<muxc::InfoBar> infoBar);
    };
}

namespace winrt::WinUI3XamlPreview::factory_implementation
{
    struct Toast : ToastT<Toast, implementation::Toast>
    {
    };
}
