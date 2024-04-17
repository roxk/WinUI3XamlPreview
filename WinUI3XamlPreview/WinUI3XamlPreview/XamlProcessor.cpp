#include "pch.h"
#include "XamlProcessor.h"

using namespace winrt;
namespace mut = winrt::Microsoft::UI::Text;
namespace wdxd = Windows::Data::Xml::Dom;

constexpr auto defaultNamespace = L"http://schemas.microsoft.com/winfx/2006/xaml/presentation";

std::vector<wdxd::XmlAttribute> GetNamespaces(wdxd::XmlDocument const& doc)
{
    std::vector<wdxd::XmlAttribute> namespaces;
    auto docAttributes{ doc.DocumentElement().Attributes() };
    for (auto&& docAttr : docAttributes)
    {
        if (std::wstring_view(docAttr.NodeName())._Starts_with(L"xmlns"))
        {
            namespaces.emplace_back(docAttr.CloneNode(true).as<wdxd::XmlAttribute>());
        }
    }
    return namespaces;
}
bool IsAttrValid(wdxd::XmlDocument const& doc, wdxd::XmlElement const& element, wdxd::XmlAttribute const& attr)
{
    auto copied = element.CloneNode(false);
    auto attributes = copied.Attributes();
    while (true)
    {
        auto iter = attributes.First();
        if (!iter.HasCurrent())
        {
            break;
        }
        auto attrNode = iter.Current();
        if (attrNode.NodeType() != wdxd::NodeType::AttributeNode)
        {
            return false;
        }
        auto victim = attrNode.try_as<wdxd::XmlAttribute>();
        if (victim == nullptr)
        {
            return false;
        }
        attributes.RemoveNamedItem(victim.as<wdxd::XmlAttribute>().Name());
    }
    auto copiedAttr = attr.CloneNode(true);
    auto namespaceUri = attr.NamespaceUri().try_as<winrt::hstring>();
    if (namespaceUri == L"")
    {
        attributes.SetNamedItem(copiedAttr);
    }
    else
    {
        attributes.SetNamedItemNS(copiedAttr);
    }
    auto namespaces{ GetNamespaces(doc) };
    for (auto&& aNamespace : namespaces)
    {
        copied.Attributes().SetNamedItemNS(aNamespace);
    }
    auto xml = copied.GetXml();
    try
    {
        muxm::XamlReader::Load(xml);
        return true;
    }
    catch (...)
    {
        return false;
    }
}
void VisitAndTrim(wdxd::XmlDocument const& doc, wdxd::IXmlNode const& candidate, std::vector<winrt::hstring>& templateTargetTypes)
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
            auto border = doc.CreateElementNS(box_value(winrt::hstring(defaultNamespace)), L"Border");
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
        // ControlTempalte is a sealed class so it's OK to just check default NS + tag name to see if it's a ControlTemplate
        auto elemetName = element.LocalName().try_as<winrt::hstring>();
        auto elementNamespace = element.NamespaceUri().try_as<winrt::hstring>();
        auto isControlTemplate = elemetName == L"ControlTemplate" && elementNamespace == defaultNamespace;
        auto isSetter = elemetName == L"Setter" && elementNamespace == defaultNamespace;
        if (isSetter)
        {
            VisitAndTrim(doc, element, templateTargetTypes);
            continue;
        }
        for (uint32_t i = 0; i < attributes.Size(); ++i)
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
            auto isAttrValid = IsAttrValid(doc, element, attr);
            if (!isAttrValid)
            {
                element.RemoveAttributeNode(attr);
            }
            else
            {
                auto name = attr.NodeName();
                if (isControlTemplate && attr.Name() == L"TargetType")
                {
                    templateTargetTypes.emplace_back(attr.Value());
                }
            }
        }
        VisitAndTrim(doc, element, templateTargetTypes);
    }
}
std::pair<std::wstring_view, std::wstring_view> SplitTargetTypeToNamespaceAndLocalName(std::wstring_view targetType)
{
    auto colonIndex = targetType.find(L":");
    if (colonIndex == -1)
    {
        return { targetType.substr(0, 0), targetType};
    }
    return { targetType.substr(0, colonIndex), targetType.substr(colonIndex + 1) };
}
winrt::WinUI3XamlPreview::ProcessResult winrt::WinUI3XamlPreview::XamlProcessor::ProcessXaml(winrt::hstring const& xaml)
{
    wdxd::XmlDocument doc;
    doc.LoadXml(xaml);
    std::vector<winrt::hstring> templateTargetTypes;
    VisitAndTrim(doc, doc, templateTargetTypes);
    auto processedXaml = doc.GetXml();
    auto tree = muxm::XamlReader::Load(processedXaml);
    if (auto element = tree.try_as<mux::UIElement>(); element != nullptr)
    {
        return SingleElement{ std::move(element) };
    }
    if (auto dict = tree.try_as<mux::ResourceDictionary>(); dict != nullptr)
    {
        if (templateTargetTypes.empty())
        {
            return MultipleElement{};
        }
        std::vector<mux::UIElement> elements;
        for (auto&& targetType : templateTargetTypes)
        {
            auto elementRoot = doc.CreateElementNS(box_value(winrt::hstring(defaultNamespace)), L"Border");
            auto namespaces{ GetNamespaces(doc) };
            for (auto&& aNamespace : namespaces)
            {
                elementRoot.Attributes().SetNamedItem(aNamespace);
            }
            auto elementResource = doc.CreateElementNS(box_value(winrt::hstring(defaultNamespace)), L"Border.Resources");
            elementResource.AppendChild(doc.DocumentElement().CloneNode(true));
            elementRoot.AppendChild(elementResource);
            auto targetTypeParts{ SplitTargetTypeToNamespaceAndLocalName(targetType) };
            auto& targetTypePrefix{ targetTypeParts.first };
            winrt::hstring targetTypeNamespaceUri;
            for (auto&& aNamespace : namespaces)
            {
                if (aNamespace.LocalName().try_as<winrt::hstring>() == targetTypePrefix)
                {
                    targetTypeNamespaceUri = aNamespace.Value();
                    break;
                }
            }
            if (targetTypeNamespaceUri == L"")
            {
                // Can't find the namespace for the target type...
                continue;
            }
            auto targetTypeElement = doc.CreateElementNS(box_value(targetTypeNamespaceUri), targetTypeParts.second);
            elementRoot.AppendChild(targetTypeElement);
            auto elementRootXaml = elementRoot.GetXml();
            // TODO: Assign control template's style name
            auto targetTypeTree = muxm::XamlReader::Load(elementRootXaml);
            if (auto element = targetTypeTree.try_as<mux::UIElement>(); element != nullptr)
            {
                elements.emplace_back(std::move(element));
            }
            else
            {
                // TODO: Log?
                continue;
            }
        }
        return MultipleElement{ std::move(elements) };
    }
    throw hresult_not_implemented();
}
