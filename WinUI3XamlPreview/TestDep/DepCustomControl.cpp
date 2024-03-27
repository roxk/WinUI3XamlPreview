#include "pch.h"
#include "DepCustomControl.h"
#if __has_include("DepCustomControl.g.cpp")
#include "DepCustomControl.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::TestDep::implementation
{
    DepCustomControl::DepCustomControl()
    {
        DefaultStyleKey(winrt::box_value(L"TestDep.DepCustomControl"));
    }

    int32_t DepCustomControl::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void DepCustomControl::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }
}
