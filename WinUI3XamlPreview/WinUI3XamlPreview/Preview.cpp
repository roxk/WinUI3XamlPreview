#include "pch.h"
#include "Preview.h"
#if __has_include("Preview.g.cpp")
#include "Preview.g.cpp"
#endif
#include "MainWindow.xaml.h"

constexpr auto instanceMainKey = L"WinUI3XamlPreview.Main";

namespace winrt::WinUI3XamlPreview::implementation
{
    Preview::Preview()
    {
        _queue = mud::DispatcherQueue::GetForCurrentThread();
    }
    wf::IAsyncOperation<bool> Preview::IsXamlPreviewLaunched()
    {
        return Instance()->IsXamlPreviewLaunchedInternal();
    }
    wf::IAsyncOperation<bool> Preview::IsXamlPreviewLaunchedInternal()
    {
        auto currentInstance = mwal::AppInstance::GetCurrent();
        const auto activatedArgs = currentInstance.GetActivatedEventArgs();
        const auto kind = activatedArgs.Kind();
        if (kind != mwal::ExtendedActivationKind::Protocol)
        {
            co_return false;
        }
        const auto data = activatedArgs.Data().try_as<waa::IProtocolActivatedEventArgs>();
        if (data == nullptr)
        {
            co_return false;
        }
        const auto uri = data.Uri();
        const auto scheme = uri.SchemeName();
        if (scheme != L"winui3xp")
        {
            co_return false;
        }
        if (co_await Redirect(activatedArgs))
        {
            mux::Application::Current().Exit();
            co_return true;
        }
        const auto host = uri.Host();
        if (host == L"close")
        {
            mux::Application::Current().Exit();
            co_return true;
        }
        const auto appInstance = mwal::AppInstance::FindOrRegisterForKey(instanceMainKey);
        if (!appInstance.IsCurrent())
        {
            mux::Application::Current().Exit();
            co_return true;
        }
        appInstance.Activated({ get_weak(), &Preview::OnActivated });
        make<MainWindow>().Activate();
        OnActivated(*this, activatedArgs);
        co_return true;
    }
    winrt::com_ptr<Preview> Preview::Instance()
    {
        static auto preview = winrt::make_self<Preview>();
        return preview;
    }
    winrt::fire_and_forget Preview::OnActivated(IInspectable sender, mwal::AppActivationArguments e)
    {
        co_await wil::resume_foreground(_queue);
        const auto kind = e.Kind();
        switch (kind)
        {
        case mwal::ExtendedActivationKind::Protocol:
        {
            const auto data = e.Data().try_as<waa::IProtocolActivatedEventArgs>();
            if (data == nullptr)
            {
                co_return;
            }
            const auto uri = data.Uri();
            const auto host = uri.Host();
            if (host == L"close")
            {
                mux::Application::Current().Exit();
                co_return;
            }
            const auto query = uri.QueryParsed();
            if (query == nullptr)
            {
                co_return;
            }
            try
            {
                const auto filePath = query.GetFirstValueByName(L"filePath");
                _filePathChanged(*this, filePath);
            }
            catch (...)
            {
                // No file path specified by user. Not an error so no need to log/react.
                co_return;
            }
            break;
        }
        }
    }
    wf::IAsyncOperation<bool> Preview::Redirect(mwal::AppActivationArguments const& e)
    {
        const auto instances = mwal::AppInstance::GetInstances();
        for (auto&& instance : instances)
        {
            if (instance.Key() == instanceMainKey)
            {
                co_await instance.RedirectActivationToAsync(e);
                co_return true;
            }
        }
        co_return false;
    }
}
