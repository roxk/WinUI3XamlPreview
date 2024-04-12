#pragma once

#include "MainWindow.g.h"

namespace winrt::WinUI3XamlPreview::implementation
{
    struct MainWindow : MainWindowT<MainWindow>, idlgen::author_class<mux::Window>
    {
        MainWindow()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }
        void InitializeComponent();
    };
}

namespace winrt::WinUI3XamlPreview::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
