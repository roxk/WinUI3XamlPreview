#pragma once

#include "MainPage.g.h"

namespace winrt::WinUI3XamlPreview::implementation
{
    struct MainPage : MainPageT<MainPage>, idlgen::author_class<muxc::Page>
    {
        MainPage();
        ~MainPage();
        [[idlgen::hide]] void OnFilePathChanged(wf::IInspectable const& sender, winrt::hstring const& e);
        static winrt::hstring ScaleDisplay(double scalePercentage);
        static wfc::IVector<wf::IInspectable> Resolutions();
        static winrt::hstring ResolutionDisplay(wf::IInspectable const& resolutionFloat2);
    private:
        static winrt::hstring ResolutionDisplay(wf::Numerics::float2 resolution);
        double _currentScale{};
        wf::Numerics::float2 _currentResolution{};
        friend struct MainPageT<MainPage>;
        void LoadXaml(winrt::hstring const& xaml);
        winrt::fire_and_forget OpenFileAndRead(winrt::hstring e);
        winrt::event_token _filePathChangedToken{};
        void widthInput_ValueChanged(winrt::Microsoft::UI::Xaml::Controls::NumberBox const& sender, winrt::Microsoft::UI::Xaml::Controls::NumberBoxValueChangedEventArgs const& args);
        void heightInput_ValueChanged(winrt::Microsoft::UI::Xaml::Controls::NumberBox const& sender, winrt::Microsoft::UI::Xaml::Controls::NumberBoxValueChangedEventArgs const& args);
        void scaleComboBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void Page_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void UpdateScaleByComboBox();
        void UpdateResolutionByComboBox();
        void UpdateCurrentScale(double scale);
        void UpdateCurrentResolution(wf::Numerics::float2 resolution);
        void UpdateScaleByComboBoxText();
        void scaleSlider_ValueChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e);
        void scaleComboBox_TextSubmitted(winrt::Microsoft::UI::Xaml::Controls::ComboBox const& sender, winrt::Microsoft::UI::Xaml::Controls::ComboBoxTextSubmittedEventArgs const& args);
        void resolutionComboBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        template<typename T, typename D>
        void CombobBoxSelectedItem(muxc::ComboBox const& comboBox, T&& value, D display);
    };
}

namespace winrt::WinUI3XamlPreview::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
