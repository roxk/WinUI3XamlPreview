#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#include "MainPage.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::WinUI3XamlPreview::implementation
{
	void MainWindow::InitializeComponent()
	{
		MainWindowT::InitializeComponent();
		ExtendsContentIntoTitleBar(true);
		mainPage().as<MainPage>()->Window(*this);
	}
}
