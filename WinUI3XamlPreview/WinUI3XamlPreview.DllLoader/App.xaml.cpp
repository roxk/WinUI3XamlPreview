#include "pch.h"
#include "App.xaml.h"

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::WinUI3XamlPreview_DllLoader::implementation
{
    /// <summary>
    /// Initializes the singleton application object.  This is the first line of authored code
    /// executed, and as such is the logical equivalent of main() or WinMain().
    /// </summary>
    App::App()
    {
        // Xaml objects should not call InitializeComponent during construction.
        // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
        UnhandledException([](IInspectable const&, UnhandledExceptionEventArgs const& e)
        {
            if (IsDebuggerPresent())
            {
                auto errorMessage = e.Message();
                __debugbreak();
            }
        });
#endif
        auto preview = WinUI3XamlPreview::Preview::Instance();
        _providerLoaded = preview.XamlMetaDataProviderLoaded({ get_weak(), &App::OnXamlMetaDataProviderLoaded });
    }
    App::~App()
    {
        auto instance = WinUI3XamlPreview::Preview::Instance();
        instance.XamlMetaDataProviderLoaded(_providerLoaded);
    }

    IXamlType App::GetXamlType(::winrt::Windows::UI::Xaml::Interop::TypeName const& type)
    {
        auto xamlType = AppT::GetXamlType(type);
        auto userXamlType = xamlType ? xamlType.try_as<IXamlUserType>() : nullptr;
        if (!xamlType || (userXamlType && userXamlType->IsReturnTypeStub() && !userXamlType->IsLocalType()))
        {
            for (auto&& provider : _providers)
            {
                auto libXamlType = provider.GetXamlType(type);
                if (libXamlType)
                {
                    if (libXamlType.IsConstructible() || !xamlType)
                    {
                        xamlType = libXamlType;
                    }
                }
            }
        }
        if (xamlType == nullptr)
        {
            if (auto provider = WinUI3XamlPreview::Preview::Instance().GetProvider(type.Name))
            {
                xamlType = provider.GetXamlType(type);
            }
        }
        return xamlType;
    }

    IXamlType App::GetXamlType(::winrt::hstring const& fullName)
    {
        auto xamlType = AppT::GetXamlType(fullName);
        auto userXamlType = xamlType ? xamlType.try_as<IXamlUserType>() : nullptr;
        if (!xamlType || (userXamlType && userXamlType->IsReturnTypeStub() && !userXamlType->IsLocalType()))
        {
            for (auto&& provider : _providers)
            {
                auto libXamlType = provider.GetXamlType(fullName);
                if (libXamlType)
                {
                    if (libXamlType.IsConstructible() || !xamlType)
                    {
                        xamlType = libXamlType;
                    }
                }
            }
        }
        if (xamlType == nullptr)
        {
            if (auto provider = WinUI3XamlPreview::Preview::Instance().GetProvider(fullName))
            {
                xamlType = provider.GetXamlType(fullName);
            }
        }
        return xamlType;
    }

    ::winrt::com_array<::winrt::Microsoft::UI::Xaml::Markup::XmlnsDefinition> App::GetXmlnsDefinitions()
    {
        return ::winrt::com_array<::winrt::Microsoft::UI::Xaml::Markup::XmlnsDefinition>(0);
    }

    /// <summary>
    /// Invoked when the application is launched.
    /// </summary>
    /// <param name="e">Details about the launch request and process.</param>
    winrt::fire_and_forget App::OnLaunched([[maybe_unused]] LaunchActivatedEventArgs const& e)
    {
        if (co_await winrt::WinUI3XamlPreview::Preview::IsXamlPreviewLaunched())
        {
            co_return;
        }
        winrt::Microsoft::UI::Xaml::Application::Current().Exit();
    }
    void App::OnXamlMetaDataProviderLoaded([[maybe_unused]] wf::IInspectable const& sender, muxm::IXamlMetadataProvider const& provider)
    {
        _providers.emplace_back(provider);
    }
}
