#pragma once

#include "BlankPage.g.h"

namespace winrt::WinUI3XamlPreview::implementation
{
    struct BlankPage : BlankPageT<BlankPage>, idlgen::author_class<muxc::Page>
    {
        BlankPage();
        ~BlankPage();
        void OnFilePathChanged(IInspectable const& sender, winrt::hstring const& e);
    private:
        friend struct BlankPageT<BlankPage>;
        void LoadXaml(winrt::hstring const& xaml);
        winrt::fire_and_forget OpenFileAndRead(winrt::hstring e);
        winrt::event_token _filePathChangedToken{};
    };
}

namespace winrt::WinUI3XamlPreview::factory_implementation
{
    struct BlankPage : BlankPageT<BlankPage, implementation::BlankPage>
    {
    };
}
