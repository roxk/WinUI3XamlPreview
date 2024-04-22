#pragma once

#include "MainPage.g.h"
#include "ScaleValue.g.h"
#include "XamlProcessor.h"

namespace winrt::WinUI3XamlPreview::implementation
{
    struct [[idlgen::property]] [[idlgen::attribute("bindable")]] ScaleValue : ScaleValueT<ScaleValue>, idlgen::author_class<wf::IStringable>
    {
        [[idlgen::hide]] ScaleValue(double scale);
        double Scale();
        winrt::hstring DisplayName();
        [[idlgen::method]] winrt::hstring ToString();
    private:
        static winrt::hstring ScaleDisplay(double scalePercentage);
        double _scale;
        winrt::hstring _displayName;
    };

    struct MainPage : MainPageT<MainPage>, idlgen::author_class<muxc::Page>
    {
        MainPage();
        ~MainPage();
        [[idlgen::hide]] void Window(mux::Window const& window);
        [[idlgen::hide]] void OnFilePathChanged(wf::IInspectable const& sender, winrt::hstring const& e);
        static winrt::hstring ThemeDisplay(winrt::hstring const& theme);
        static wfc::IVector<wf::IInspectable> Scales();
        static wfc::IVector<wf::IInspectable> Resolutions();
        static winrt::hstring ResolutionDisplay(wf::IInspectable const& resolutionFloat2);
    private:
        void SetRegionsForCustomTitleBar();
        static wg::RectInt32 GetControlClientRect(mux::FrameworkElement const& element);
        static winrt::hstring ResolutionDisplay(wf::Numerics::float2 resolution);
        muw::AppWindow _appWindow{nullptr};
        winrt::hstring _currentTheme{};
        WinUI3XamlPreview::ScaleValue _currentScale{nullptr};
        wf::Numerics::float2 _currentResolution{};
        wfc::IVector<winrt::hstring> _customControlNames;
        wf::IInspectable _lastSelectedCustomControlName{};
        std::vector<CustomControlItem> _customControlItems;
        friend struct MainPageT<MainPage>;
        void LoadXaml(winrt::hstring const& xaml);
        winrt::fire_and_forget OpenFileAndRead(winrt::hstring e);
        winrt::event_token _filePathChangedToken{};
        void widthInput_ValueChanged(winrt::Microsoft::UI::Xaml::Controls::NumberBox const& sender, winrt::Microsoft::UI::Xaml::Controls::NumberBoxValueChangedEventArgs const& args);
        void heightInput_ValueChanged(winrt::Microsoft::UI::Xaml::Controls::NumberBox const& sender, winrt::Microsoft::UI::Xaml::Controls::NumberBoxValueChangedEventArgs const& args);
        void scaleComboBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void Page_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void UpdateCustomControlItems(std::vector<CustomControlItem> items);
        void UpdateResolutionByComboBox();
        void UpdateCurrentTheme(winrt::hstring const& theme);
        void UpdateCurrentScale(double scale);
        void UpdateCurrentScale(WinUI3XamlPreview::ScaleValue const& value);
        void UpdateScaleForSliderAndScrollView(double scale);
        void UpdateCurrentResolution(wf::Numerics::float2 resolution);
        void FitToPage();
        void UpdateScaleByComboBoxText();
        void scaleSlider_ValueChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e);
        void scaleComboBox_TextSubmitted(winrt::Microsoft::UI::Xaml::Controls::ComboBox const& sender, winrt::Microsoft::UI::Xaml::Controls::ComboBoxTextSubmittedEventArgs const& args);
        void resolutionComboBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void fitPageButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void themeComboBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void customControlComboBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void titleBar_SizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::SizeChangedEventArgs const& e);
        void customControlComboBox_SizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::SizeChangedEventArgs const& e);
        void UpdateCustomControlComboBoxSize();
    };
}

namespace winrt::WinUI3XamlPreview::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
