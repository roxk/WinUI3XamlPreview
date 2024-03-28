#include "pch.h"
#include "Toast.xaml.h"
#if __has_include("Toast.g.cpp")
#include "Toast.g.cpp"
#endif
#include <chrono>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace std::chrono_literals;

constexpr auto infoBarShownDuration = 5s;

namespace winrt::WinUI3XamlPreview::implementation
{
	void Toast::ShowError(winrt::hstring const& title, winrt::hstring const& message)
	{
		muxc::InfoBar bar;
		bar.Severity(muxc::InfoBarSeverity::Error);
		bar.Title(title);
		bar.Message(message);
		bar.IsOpen(true);
		items().Items().InsertAt(0, bar);
		Hide(winrt::make_weak(bar));
	}
	winrt::fire_and_forget Toast::Hide(winrt::weak_ref<muxc::InfoBar> infoBar)
	{
		auto queue = mud::DispatcherQueue::GetForCurrentThread();
		auto weak = get_weak();
		co_await winrt::resume_after(infoBarShownDuration);
		co_await wil::resume_foreground(queue);
		auto strongBar = infoBar.get();
		auto strongSelf = weak.get();
		if (strongBar == nullptr || strongSelf == nullptr)
		{
			co_return;
		}
		auto items = strongSelf->items().Items();
		const auto itemCount = items.Size();
		// We always insert at the start, so look up from the end.
		for (int i = itemCount - 1; i >= 0; --i)
		{
			auto item = items.GetAt(i);
			if (item == strongBar)
			{
				items.RemoveAt(i);
				break;
			}
		}
	}
}
