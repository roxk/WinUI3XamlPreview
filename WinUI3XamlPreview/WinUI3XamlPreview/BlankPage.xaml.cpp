#include "pch.h"
#include "BlankPage.xaml.h"
#if __has_include("BlankPage.g.cpp")
#include "BlankPage.g.cpp"
#endif
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Microsoft.UI.Text.h>
#include <winrt/Windows.Data.Xml.Dom.h>
#include "Preview.h"

using namespace winrt;
namespace mut = Microsoft::UI::Text;
namespace wdxd = Windows::Data::Xml::Dom;


// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

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
                auto hasXClass = attr.Name() == L"x:Class";
                auto hasBinding = strView._Starts_with(L"{x:Bind") || strView._Starts_with(L"{Binding");
                if (hasXClass || hasBinding)
                {
                    element.RemoveAttributeNode(attr);
                }
            }
            VisitAndTrim(doc, element);
        }
    }
    BlankPage::BlankPage()
    {
        _filePathChangedToken = Preview::Instance()->FilePathChanged({get_weak(), &BlankPage::OnFilePathChanged});
    }
    BlankPage::~BlankPage()
    {
        Preview::Instance()->FilePathChanged(_filePathChangedToken);
    }
    void BlankPage::OnFilePathChanged(IInspectable const& sender, winrt::hstring const& e)
    {
        OpenFileAndRead(e);
    }
    void BlankPage::LoadXaml(winrt::hstring const& xaml)
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
            // TODO: Show error
        }
    }
    winrt::fire_and_forget BlankPage::OpenFileAndRead(winrt::hstring e)
    {
        try
        {
            auto weak = get_weak();
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
            // TODO: Show error
        }
    }
}
