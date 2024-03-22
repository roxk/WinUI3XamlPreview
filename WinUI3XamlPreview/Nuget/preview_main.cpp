#include "pch.h"
#include "XamlMetaDataProvider.h"

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    winrt::init_apartment();
    winrt::Microsoft::UI::Xaml::Application::Start(
        [](auto&&)
        {
            winrt::WinUI3XamlPreview::Preview::App(winrt::TARGET-NAMESPACE::XamlMetaDataProvider());
        });
}
