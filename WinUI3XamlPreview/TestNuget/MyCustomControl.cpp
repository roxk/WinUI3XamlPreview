#include "pch.h"
#include "MyCustomControl.h"
#if __has_include("MyCustomControl.g.cpp")
#include "MyCustomControl.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::TestNuget::implementation
{
    MyCustomControl::MyCustomControl()
    {
        DefaultStyleKey(winrt::box_value(L"TestNuget.MyCustomControl"));
    }

    int32_t MyCustomControl::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void MyCustomControl::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }
}
