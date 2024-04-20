#include "pch.h"
#include "MainPage.xaml.h"
#if __has_include("MainPage.g.cpp")
#include "MainPage.g.cpp"
#endif
#include <winrt/Windows.Storage.Streams.h>
#include "Preview.h"
#include <regex>
#include "XamlProcessor.h"

using namespace winrt;
using namespace std::string_view_literals;
namespace mut = winrt::Microsoft::UI::Text;

double GetScaleComboBoxSelectedScalePercentage(muxc::ComboBox const& comboBox)
{
    auto item = comboBox.SelectedItem();
    auto value = winrt::unbox_value<double>(item);
    return value;
}

namespace winrt::WinUI3XamlPreview::implementation
{
    MainPage::MainPage() : 
        _customControlNames(winrt::single_threaded_observable_vector<winrt::hstring>())
    {
        _filePathChangedToken = Preview::InstanceInternal()->FilePathChanged({get_weak(), &MainPage::OnFilePathChanged});
    }
    MainPage::~MainPage()
    {
        Preview::InstanceInternal()->FilePathChanged(_filePathChangedToken);
    }
    void MainPage::Window(mux::Window const& window)
    {
        _appWindow = window.AppWindow();
    }
    void MainPage::OnFilePathChanged(IInspectable const& sender, winrt::hstring const& e)
    {
        OpenFileAndRead(e);
    }
    void MainPage::LoadXaml(winrt::hstring const& xaml)
    {
        try
        {
            auto processResult{ XamlProcessor::ProcessXaml(xaml) };
            if (auto singleElement = std::get_if<SingleElement>(&processResult); singleElement != nullptr)
            {
                elementWrapper().Child(singleElement->element);
                UpdateCustomControlItems({});
            }
            else if (auto multipleElement = std::get_if<MultipleElement>(&processResult); multipleElement != nullptr)
            {
                auto& elements = multipleElement->elements;
                if (elements.empty())
                {
                    mwamr::ResourceLoader res(mwamr::ResourceLoader::GetDefaultResourceFilePath(), L"WinUI3XamlPreview/Resources");
                    toast().ShowError(res.GetString(L"XamlLoadedNothingError"),
                        res.GetString(L"ResourceDictionaryNoControlTemplates"));
                    return;
                }
                UpdateCustomControlItems(std::move(elements));
            }
        }
        catch (...)
        {
            mwamr::ResourceLoader res(mwamr::ResourceLoader::GetDefaultResourceFilePath(), L"WinUI3XamlPreview/Resources");
            toast().ShowError(res.GetString(L"XamlLoadErrorTitle"),
                res.GetString(L"XamlLoadErrorMessage"));
        }
    }
    winrt::fire_and_forget MainPage::OpenFileAndRead(winrt::hstring e)
    {
        auto weak = get_weak();
        try
        {
            auto file = co_await ws::StorageFile::GetFileFromPathAsync(e);
            auto content = co_await ws::FileIO::ReadTextAsync(file);
            auto strong = weak.get();
            if (strong == nullptr)
            {
                co_return;
            }
            strong->LoadXaml(std::move(content));
        }
        catch (...)
        {
            auto strong = weak.get();
            if (strong == nullptr)
            {
                co_return;
            }
            mwamr::ResourceLoader res(mwamr::ResourceLoader::GetDefaultResourceFilePath(), L"WinUI3XamlPreview/Resources");
            auto messageFormat = res.GetString(L"FileNotFoundMessage");
            auto message = std::regex_replace(messageFormat.c_str(), std::wregex(L"\\{\\}"), e.c_str());
            strong->toast().ShowError(res.GetString(L"FileNotFoundTitle"),
                message);
        }
    }
    winrt::hstring MainPage::ScaleDisplay(double scalePercentage)
    {
        return std::to_wstring(int(scalePercentage)) + winrt::hstring(L"%");
    }
    winrt::hstring MainPage::ThemeDisplay(winrt::hstring const& theme)
    {
        mwamr::ResourceLoader res(mwamr::ResourceLoader::GetDefaultResourceFilePath(), L"WinUI3XamlPreview/Resources");
        return res.GetString(winrt::hstring(L"ThemeDisplay_") + theme);
    }
    wfc::IVector<IInspectable> MainPage::Resolutions()
    {
        static auto resolutions = winrt::single_threaded_vector<wf::IInspectable>({
            box_value(wf::Numerics::float2{ 1920, 1080 }),
            box_value(wf::Numerics::float2{ 2560, 1440 }),
            box_value(wf::Numerics::float2{ 3840, 2160 }),
            box_value(wf::Numerics::float2{ 2880, 1920 }),
        });
        return resolutions;
    }
    winrt::hstring MainPage::ResolutionDisplay(wf::IInspectable const& resolutionFloat2)
    {
        auto resolution = winrt::unbox_value<wf::Numerics::float2>(resolutionFloat2);
        return ResolutionDisplay(resolution);
    }
    void MainPage::SetRegionsForCustomTitleBar()
    {
        auto& appWindow = _appWindow;
        if (appWindow == nullptr)
        {
            return;
        }
        auto scale = XamlRoot().RasterizationScale();
        auto leftInset = _appWindow.TitleBar().LeftInset() / scale;
        auto rightInset = _appWindow.TitleBar().RightInset() / scale;
        titleBar().Margin({ leftInset, 0, rightInset, 0 });
        wg::RectInt32 rectArray[] = { GetControlClientRect(themeComboBox()), GetControlClientRect(customControlComboBox()) };
        auto nonClientInputSrc =
            mui::InputNonClientPointerSource::GetForWindowId(_appWindow.Id());
        nonClientInputSrc.SetRegionRects(mui::NonClientRegionKind::Passthrough, rectArray);
    }
    wg::RectInt32 MainPage::GetControlClientRect(mux::FrameworkElement const& element)
    {
        auto scale = element.XamlRoot().RasterizationScale();
        auto transform = element.TransformToVisual(nullptr);
        auto bounds = transform.TransformBounds(wf::Rect{ 0, 0,
            float(element.ActualWidth()),
            float(element.ActualHeight())
            });
        return wg::RectInt32{ int32_t(bounds.X * scale),
            int32_t(bounds.Y * scale),
            int32_t(bounds.Width * scale),
            int32_t(bounds.Height * scale)
        };
    }
    winrt::hstring MainPage::ResolutionDisplay(wf::Numerics::float2 resolution)
    {
        auto width = std::to_wstring(int(resolution.x));
        auto height = std::to_wstring(int(resolution.y));
        // Wish we have hstring builder exposed. Right now it's in impl.
        return width + winrt::hstring(L"x") + height;
    }
    void MainPage::UpdateScaleByComboBox()
    {
        auto scale = GetScaleComboBoxSelectedScalePercentage(scaleComboBox());
        UpdateCurrentScale(scale);
    }
    void MainPage::UpdateResolutionByComboBox()
    {
        auto combobBox = resolutionComboBox();
        auto resolutionFloat2 = unbox_value<wf::Numerics::float2>(combobBox.SelectedItem());
        UpdateCurrentResolution(resolutionFloat2);
    }
    void MainPage::UpdateCurrentTheme(winrt::hstring const& theme)
    {
        if (!IsLoaded())
        {
            return;
        }
        if (_currentTheme == theme)
        {
            return;
        }
        _currentTheme = theme;
        CombobBoxSelectedItem(themeComboBox(), box_value(theme), ThemeDisplay(theme));
        elementWrapper().RequestedTheme(theme == L"System"
            ? mux::ElementTheme::Default
            : theme == L"Dark"
            ? mux::ElementTheme::Dark
            : mux::ElementTheme::Light);
    }
    void MainPage::UpdateCurrentScale(double scale)
    {
        if (!IsLoaded())
        {
            return;
        }
        if (_currentScale == scale)
        {
            return;
        }
        _currentScale = scale;
        CombobBoxSelectedItem(scaleComboBox(), box_value(scale), ScaleDisplay(scale));
        scaleSlider().Value(scale);
        auto zoom = float(scale * 0.01);
        viewportScrollView().ZoomTo(zoom,
            wf::Numerics::float2{ float(elementWrapper().Width() * 0.5), float(elementWrapper().Height() * 0.5) });
    }
    void winrt::WinUI3XamlPreview::implementation::MainPage::UpdateCurrentResolution(wf::Numerics::float2 resolution)
    {
        if (_currentResolution == resolution)
        {
            return;
        }
        _currentResolution = resolution;
        widthInput().Value(resolution.x);
        heightInput().Value(resolution.y);
    }
    void MainPage::FitToPage()
    {
        auto scrollView = viewportScrollView();
        auto wrapper = elementWrapper();
        auto wrapperWidth = wrapper.ActualWidth();
        auto wrapperHeight = wrapper.ActualHeight();
        if (wrapperWidth == 0 || wrapperHeight == 0)
        {
            return;
        }
        auto wRatio = scrollView.ActualWidth() / wrapperWidth;
        auto hRatio = scrollView.ActualHeight() / wrapperHeight;
        auto ratio = wRatio < hRatio ? wRatio : hRatio;
        UpdateCurrentScale(std::round(ratio * 100));
    }
    void winrt::WinUI3XamlPreview::implementation::MainPage::UpdateScaleByComboBoxText()
    {
        auto comboBox = scaleComboBox();
        try
        {
            auto text = comboBox.Text();
            auto scaleStr = std::wstring{ text.back() == L'%' ? std::wstring_view(text).substr(0, text.size() - 1) : text.c_str() };
            auto scale = std::stod(scaleStr);
            CombobBoxSelectedItem(comboBox, box_value(scale), ScaleDisplay(scale));
        }
        catch (...)
        {
            auto item = comboBox.SelectedItem();
            auto scale = winrt::unbox_value<double>(item);
            CombobBoxSelectedItem(scaleComboBox(), box_value(scale), ScaleDisplay(scale));
        }
    }
}

// TODO: Once C++'s XC fixed element binding requiring element to be public, use {x:Bind widthInput.Value, Mode=OneWay} instead
void winrt::WinUI3XamlPreview::implementation::MainPage::widthInput_ValueChanged(winrt::Microsoft::UI::Xaml::Controls::NumberBox const& sender, winrt::Microsoft::UI::Xaml::Controls::NumberBoxValueChangedEventArgs const& args)
{
    auto value = args.NewValue();
    if (std::isnan(value))
    {
        widthInput().Value(args.OldValue());
        return;
    }
    elementWrapper().Width(value);
}

void winrt::WinUI3XamlPreview::implementation::MainPage::heightInput_ValueChanged(winrt::Microsoft::UI::Xaml::Controls::NumberBox const& sender, winrt::Microsoft::UI::Xaml::Controls::NumberBoxValueChangedEventArgs const& args)
{
    auto value = args.NewValue();
    if (std::isnan(value))
    {
        heightInput().Value(args.OldValue());
        return;
    }
    elementWrapper().Height(value);
}

void winrt::WinUI3XamlPreview::implementation::MainPage::scaleComboBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
    UpdateScaleByComboBox();
}

void winrt::WinUI3XamlPreview::implementation::MainPage::Page_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    SetRegionsForCustomTitleBar();
    UpdateResolutionByComboBox();
    FitToPage();

    auto comboBox = scaleComboBox();
    auto scales = comboBox.Items();
    auto minScale = unbox_value<double>(scales.GetAt(0));
    auto maxScale = unbox_value<double>(scales.GetAt(scales.Size() - 1));
    auto viewportSV = viewportScrollView();
    viewportSV.MinZoomFactor(minScale * 0.01);
    viewportSV.MaxZoomFactor(maxScale * 0.01);

    auto slider = scaleSlider();
    slider.Minimum(minScale);
    slider.Maximum(maxScale);
    slider.Value(GetScaleComboBoxSelectedScalePercentage(comboBox));

    customControlComboBox().ItemsSource(_customControlNames);
}

void winrt::WinUI3XamlPreview::implementation::MainPage::UpdateCustomControlItems(std::vector<CustomControlItem> items)
{
    _customControlNames.Clear();
    for (auto&& item : items)
    {
        _customControlNames.Append(item.displayName);
    }
    _customControlItems = std::move(items);
    wf::IInspectable newSelected{};
    auto lastSelectedName = _lastSelectedCustomControlName.try_as<winrt::hstring>();
    for (auto&& candidate : _customControlNames)
    {
        if (candidate == lastSelectedName)
        {
            newSelected = box_value(candidate);
            break;
        }
    }
    // If can't select any previous selection, select the first one.
    if (newSelected == nullptr && _customControlNames.Size() > 0)
    {
        newSelected = box_value(_customControlNames.GetAt(0));
    }
    _lastSelectedCustomControlName = newSelected;
    customControlComboBox().SelectedItem(newSelected);
    customControlComboBox().Visibility(_customControlNames.Size() > 0 ? mux::Visibility::Visible : mux::Visibility::Collapsed);
}

void winrt::WinUI3XamlPreview::implementation::MainPage::scaleSlider_ValueChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e)
{
    auto scalePercentage = e.NewValue();
    UpdateCurrentScale(scalePercentage);
}

void winrt::WinUI3XamlPreview::implementation::MainPage::scaleComboBox_TextSubmitted(winrt::Microsoft::UI::Xaml::Controls::ComboBox const& sender, winrt::Microsoft::UI::Xaml::Controls::ComboBoxTextSubmittedEventArgs const& args)
{
    UpdateScaleByComboBoxText();
    args.Handled(true);
}

void winrt::WinUI3XamlPreview::implementation::MainPage::resolutionComboBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
    UpdateResolutionByComboBox();
}

// Situation: Setting ComboBox's Text _after_ setting selected item inside selection changed event doesn't update Text
// correctly, likely because it is internally tring to set text as well. Workaround: Set text after the event.
template<typename T, typename D>
void winrt::WinUI3XamlPreview::implementation::MainPage::CombobBoxSelectedItem(muxc::ComboBox const& comboBox, T&& value, D display)
{
    comboBox.SelectedItem(value);
    mud::DispatcherQueue::GetForCurrentThread().TryEnqueue([cb = comboBox, display = std::move(display)]()
        {
            cb.Text(display);
        });
}

void winrt::WinUI3XamlPreview::implementation::MainPage::fitPageButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    FitToPage();
}

void winrt::WinUI3XamlPreview::implementation::MainPage::themeComboBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
    UpdateCurrentTheme(unbox_value<winrt::hstring>(themeComboBox().SelectedItem()));
}

void winrt::WinUI3XamlPreview::implementation::MainPage::customControlComboBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
    customControlComboBox().Width(std::numeric_limits<double>::signaling_NaN());
    auto index = customControlComboBox().SelectedIndex();
    if (index < 0 || index >= _customControlItems.size())
    {
        return;
    }
    auto& element = _customControlItems[index];
    elementWrapper().Child(element.element);
}

void winrt::WinUI3XamlPreview::implementation::MainPage::titleBar_SizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::SizeChangedEventArgs const& e)
{
    SetRegionsForCustomTitleBar();
}

void winrt::WinUI3XamlPreview::implementation::MainPage::customControlComboBox_SizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::SizeChangedEventArgs const& e)
{
    // ComboBox for some reason doesn't have intrinsic width when flyout is shown if width isn't set.
    // Workaround: set measured actual width as width so even when flyout is shown the width stay constant.
    customControlComboBox().Width(e.NewSize().Width);
    SetRegionsForCustomTitleBar();
}
