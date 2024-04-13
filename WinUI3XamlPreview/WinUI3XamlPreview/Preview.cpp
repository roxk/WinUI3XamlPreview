#include "pch.h"
#include "Preview.h"
#if __has_include("Preview.g.cpp")
#include "Preview.g.cpp"
#endif
#include "MainWindow.xaml.h"
#include <filesystem>

constexpr auto instanceMainKey = L"WinUI3XamlPreview.Main";

namespace winrt::WinUI3XamlPreview::implementation
{
    HMODULE load_library(std::wstring_view dll)
    {
        return LoadLibraryEx(dll.data(), nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    }
    Preview::Preview()
    {
        _queue = mud::DispatcherQueue::GetForCurrentThread();
    }
    wf::IAsyncOperation<bool> Preview::IsXamlPreviewLaunched()
    {
        return InstanceInternal()->IsXamlPreviewLaunchedInternal();
    }
    winrt::com_ptr<Preview> Preview::InstanceInternal()
    {
        return Instance().as<Preview>();
    }
    muxm::IXamlMetadataProvider Preview::GetProvider(winrt::hstring const& typeName)
    {
        std::wstring dllPath{ typeName };
        std::size_t count{};
        auto loader = [this](std::wstring_view dllPath, winrt::hstring const& providerName) -> muxm::IXamlMetadataProvider
            {
                // TODO: Prevent the same provider from created more than once?
                wil::unique_hmodule library{ load_library(dllPath) };
                if (!library.is_valid())
                {
                    return nullptr;
                }
                auto library_call = reinterpret_cast<int32_t(__stdcall*)(void* classId, void** factory)>(WINRT_IMPL_GetProcAddress(library.get(), "DllGetActivationFactory"));
                if (!library_call)
                {
                    return nullptr;
                }
                winrt::param::hstring paramName{ providerName };
                wf::IActivationFactory factory;
                if (0 != library_call(*(void**)(&paramName), winrt::put_abi(factory)))
                {
                    return nullptr;
                }
                if (auto provider = factory.ActivateInstance<muxm::IXamlMetadataProvider>(); provider != nullptr)
                {
                    _xamlMetaDataProviderLoaded(*this, provider);
                    _modules.emplace_back(library.release());
                    return provider;
                }
                return nullptr;
            };
        while (std::wstring::npos != (count = dllPath.rfind('.')))
        {
            dllPath.resize(count);
            auto aNamespace = winrt::hstring(dllPath);
            dllPath += L".dll";
            auto provider = loader(dllPath, aNamespace + L".XamlMetaDataProvider");
            if (provider != nullptr)
            {
                return provider;
            }
            // TODO: CsWinRT support is currently blocked by CsWinRT's host dll issue. Renable once 
            // https://github.com/microsoft/CsWinRT/issues/1564 is fixed
            dllPath.resize(dllPath.size() - 4);
        }
        return nullptr;
    }
    wf::IAsyncOperation<bool> Preview::IsXamlPreviewLaunchedInternal()
    {
        auto currentInstance = mwal::AppInstance::GetCurrent();
        mwal::AppActivationArguments activatedArgs{ nullptr };
        {
            bool errored{};
            try
            {
                activatedArgs = currentInstance.GetActivatedEventArgs();
            }
            catch (...)
            {
                errored = true;
            }
            if (errored || activatedArgs == nullptr)
            {
                co_return false;
            }
        }
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
        else if (host == L"loadLibrary")
        {
            // Note: All errors are ignored as there is nothing meanful to show users.
            // However, since we handled the protocol, we still tell the caller that "yes, the preview
            // had been launched" (and we call exist instead).
            // <returns>true if there are errors</returns>
            auto loadLibrary = [&]()
                {
                    try
                    {
                        auto queries = uri.QueryParsed();
                        std::vector<wil::unique_hmodule> modules;
                        for (auto&& query : queries)
                        {
                            if (query.Name() == L"dllPath")
                            {
                                const auto dllPath = query.Value().c_str();
                                wil::unique_hmodule mod{ load_library(dllPath) };
                                if (!mod.is_valid())
                                {
                                    return true;
                                }
                                auto path = std::filesystem::path(dllPath);
                                auto fileName = path.stem().wstring();
                                // TODO: Investigate dynamic dependency etc to make the dll visible to combase.dll?
                                // Then we can use the nice and compact winrt::get_activation_factory method.
                                // TODO: Winmd might not be dll name, e.g. WASDK.
                                auto name = winrt::hstring(fileName) + L".XamlMetaDataProvider";
                                wf::IActivationFactory factory;
                                auto library_call = reinterpret_cast<int32_t(__stdcall*)(void* classId, void** factory)>(WINRT_IMPL_GetProcAddress(mod.get(), "DllGetActivationFactory"));
                                if (library_call != nullptr)
                                {
                                    auto paramName = winrt::param::hstring(name);
                                    winrt::check_hresult(library_call(*(void**)(&paramName), winrt::put_abi(factory)));
                                    _xamlMetaDataProviderLoaded(*this, factory.ActivateInstance<muxm::IXamlMetadataProvider>());
                                }
                                // If library_call is nullptr, it's a Non-WinRT dll.
                                modules.emplace_back(std::move(mod));
                            }
                        }
                        for (auto&& mod : modules)
                        {
                            _modules.emplace_back(std::move(mod));
                        }
                        return false;
                    }
                    catch (...)
                    {
                        return true;
                    }
                };
            if (loadLibrary())
            {
                mux::Application::Current().Exit();
                co_return true;
            }
            // Continue execution as we want to handle showing as well.
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
    WinUI3XamlPreview::Preview Preview::Instance()
    {
        static auto preview{ winrt::make<Preview>() };
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
