#pragma once

#include "Preview.g.h"
#include <wil/resource.h>

namespace winrt::WinUI3XamlPreview::implementation
{
    struct Preview : PreviewT<Preview>, idlgen::author_class<>
    {
        [[idlgen::hide]]
        Preview();
        static wf::IAsyncOperation<bool> IsXamlPreviewLaunched();
        [[idlgen::hide]]
        winrt::event_token FilePathChanged(wf::EventHandler<winrt::hstring> const& handler)
        {
            return _filePathChanged.add(handler);
        }
        [[idlgen::hide]]
        void FilePathChanged(winrt::event_token token)
        {
            _filePathChanged.remove(token);
        }
        static WinUI3XamlPreview::Preview Instance();
        [[idlgen::hide]]
        static winrt::com_ptr<Preview> InstanceInternal();
        winrt::event_token XamlMetaDataProviderLoaded(wf::EventHandler<muxm::IXamlMetadataProvider> const& handler)
        {
            return _xamlMetaDataProviderLoaded.add(handler);
        }
        void XamlMetaDataProviderLoaded(winrt::event_token token)
        {
            _xamlMetaDataProviderLoaded.remove(token);
        }
        winrt::event_token XamlThemeGenericFilePathAdded(wf::EventHandler<winrt::hstring> const& handler)
        {
            return _xamlThemeGenericFilePathAdded.add(handler);
        }
        void XamlThemeGenericFilePathAdded(winrt::event_token token)
        {
            _xamlThemeGenericFilePathAdded.remove(token);
        }
    private:
        wf::IAsyncOperation<bool> IsXamlPreviewLaunchedInternal();
        winrt::fire_and_forget OnActivated(IInspectable sender, mwal::AppActivationArguments e);
        wf::IAsyncOperation<bool> Redirect(mwal::AppActivationArguments const& e);
        mud::DispatcherQueue _queue{ nullptr };
        winrt::event<wf::EventHandler<winrt::hstring>> _filePathChanged;
        winrt::event<wf::EventHandler<muxm::IXamlMetadataProvider>> _xamlMetaDataProviderLoaded;
        winrt::event<wf::EventHandler<winrt::hstring>> _xamlThemeGenericFilePathAdded;
        std::vector<wil::unique_hmodule> _modules;
    };
}

namespace winrt::WinUI3XamlPreview::factory_implementation
{
    struct Preview : PreviewT<Preview, implementation::Preview>
    {
    };
}
