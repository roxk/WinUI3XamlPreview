#pragma once

#include "pch.h"
#include <vector>
#include <variant>

namespace winrt::WinUI3XamlPreview
{
	struct SingleElement
	{
		mux::UIElement element;
	};

	struct CustomControlItem
	{
		winrt::hstring displayName;
		mux::UIElement element;
	};

	struct MultipleElement
	{
		std::vector<CustomControlItem> elements;
	};

	using ProcessResult = std::variant<SingleElement, MultipleElement>;

	class XamlProcessor
	{
	public:
		static ProcessResult ProcessXaml(winrt::hstring const& xaml);
	};
}
