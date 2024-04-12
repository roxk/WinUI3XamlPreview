#include "pch.h"
#include "MainPage.xaml.h"
#if __has_include("MainPage.g.cpp")
#include "MainPage.g.cpp"
#endif
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Microsoft.UI.Text.h>
#include <winrt/Windows.Data.Xml.Dom.h>
#include "Preview.h"
#include <regex>

using namespace winrt;
namespace mut = winrt::Microsoft::UI::Text;
namespace wdxd = Windows::Data::Xml::Dom;

double GetScaleComboBoxSelectedScalePercentage(muxc::ComboBox const& comboBox)
{
    auto item = comboBox.SelectedItem();
    auto value = winrt::unbox_value<double>(item);
    return value;
}

namespace winrt::WinUI3XamlPreview::implementation
{
    void VisitAndTrim(wdxd::XmlDocument const& doc, wdxd::IXmlNode const& candidate)
    {
        for (auto&& node : candidate.ChildNodes())
        {
            auto element = node.try_as<wdxd::XmlElement>();
            if (element == nullptr)
            {
                continue;
            }
            if (element.NodeName() == L"Window")
            {
                auto border = doc.CreateElement(L"Border");
                auto children = element.ChildNodes();
                for (auto&& child : children)
                {
                    border.AppendChild(child.CloneNode(true));
                }
                auto attributes = element.Attributes();
                for (auto&& attrNode : attributes)
                {
                    border.SetAttributeNode(attrNode.CloneNode(true).as<wdxd::XmlAttribute>());
                }
                candidate.RemoveChild(element);
                candidate.AppendChild(border);
                element = border;
            }
            auto attributes = element.Attributes();
            for (int i = 0; i < attributes.Size(); ++i)
            {
                auto attrNode = attributes.GetAt(i);
                if (attrNode.NodeType() != wdxd::NodeType::AttributeNode)
                {
                    continue;
                }
                auto attr = attrNode.try_as<wdxd::XmlAttribute>();
                if (attr == nullptr)
                {
                    continue;
                }
                auto attrValue = attrNode.NodeValue();
                if (attrValue == nullptr)
                {
                    continue;
                }
                auto attrStr = attrValue.try_as<winrt::hstring>();
                if (!attrStr)
                {
                    continue;
                }
                auto strView = std::wstring_view(*attrStr);
                auto startsWithXColon = std::wstring_view(attr.Name())._Starts_with(L"x");
                auto hasBinding = strView._Starts_with(L"{x:Bind") || strView._Starts_with(L"{Binding");
                if (startsWithXColon || hasBinding)
                {
                    element.RemoveAttributeNode(attr);
                }
            }
            VisitAndTrim(doc, element);
        }
    }
    MainPage::MainPage()
    {
        _filePathChangedToken = Preview::InstanceInternal()->FilePathChanged({get_weak(), &MainPage::OnFilePathChanged});
    }
    MainPage::~MainPage()
    {
        Preview::InstanceInternal()->FilePathChanged(_filePathChangedToken);
    }
    void MainPage::OnFilePathChanged(IInspectable const& sender, winrt::hstring const& e)
    {
        OpenFileAndRead(e);
    }
    void MainPage::LoadXaml(winrt::hstring const& xaml)
    {
        try
        {
            wdxd::XmlDocument doc;
            doc.LoadXml(xaml);
            VisitAndTrim(doc, doc);
            auto trimmedXaml = doc.GetXml();
            auto tree = muxm::XamlReader::Load(trimmedXaml);
            auto element = tree.try_as<mux::UIElement>();
            if (element == nullptr)
            {
                return;
            }
            elementWrapper().Child(element);
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
    mud::DispatcherQueue::GetForCurrentThread().TryEnqueue([strong = get_strong(), display = std::move(display)]()
        {
            strong->scaleComboBox().Text(display);
        });
}

void winrt::WinUI3XamlPreview::implementation::MainPage::fitPageButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    FitToPage();
}
