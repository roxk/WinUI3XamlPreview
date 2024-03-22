#pragma once

#include "Preview.g.h"

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
        [[idlgen::hide]]
        static winrt::com_ptr<Preview> Instance();
    private:
        wf::IAsyncOperation<bool> IsXamlPreviewLaunchedInternal();
        winrt::fire_and_forget OnActivated(IInspectable sender, mwal::AppActivationArguments e);
        wf::IAsyncOperation<bool> Redirect(mwal::AppActivationArguments const& e);
        mud::DispatcherQueue _queue{ nullptr };
        winrt::event<wf::EventHandler<winrt::hstring>> _filePathChanged;
    };
}

namespace winrt::WinUI3XamlPreview::factory_implementation
{
    struct Preview : PreviewT<Preview, implementation::Preview>
    {
    };
}
