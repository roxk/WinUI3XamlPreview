#include "pch.h"
#include "DepCustomControl.h"
#if __has_include("DepCustomControl.g.cpp")
#include "DepCustomControl.g.cpp"
#endif
#include "../TestNonWinRTDep/SomeLibrary.h"
#include <iostream>

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::TestDepOfDep::implementation
{
    DepCustomControl::DepCustomControl()
    {
        DefaultStyleKey(winrt::box_value(L"TestDepOfDep.DepCustomControl"));
        int result = SomeFunction(42);
        std::cout << result << std::endl;
    }
}
