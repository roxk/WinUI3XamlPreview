#pragma once

#include "App.xaml.g.h"
#include <vector>

namespace winrt::WinUI3XamlPreview_DllLoader::implementation
{
    struct App : AppT<App>
    {
        App();
        ~App();

        IXamlType GetXamlType(::winrt::Windows::UI::Xaml::Interop::TypeName const& type);

        IXamlType GetXamlType(::winrt::hstring const& fullName);

        ::winrt::com_array<::winrt::Microsoft::UI::Xaml::Markup::XmlnsDefinition> GetXmlnsDefinitions();

        winrt::fire_and_forget OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);

    private:
        void OnXamlMetaDataProviderLoaded(wf::IInspectable const& sender, muxm::IXamlMetadataProvider const& provider);
        winrt::event_token _providerLoaded{};
        winrt::event_token _themesGenericFileAdded{};
        std::vector<muxm::IXamlMetadataProvider> _providers{};
    };
}
