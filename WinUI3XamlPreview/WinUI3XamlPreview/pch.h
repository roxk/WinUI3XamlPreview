#pragma once
#define NOMINMAX
#include <windows.h>
#include <unknwn.h>
#include <restrictederrorinfo.h>
#include <hstring.h>

// Undefine GetCurrentTime macro to prevent
// conflict with Storyboard::GetCurrentTime
#undef GetCurrentTime

#include <winrt/Windows.Data.Xml.Dom.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.ApplicationModel.Activation.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Microsoft.UI.Composition.h>
#include <winrt/Microsoft.UI.Input.h>
#include <winrt/Microsoft.UI.Text.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.Windows.ApplicationModel.Resources.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Data.h>
#include <winrt/Microsoft.UI.Xaml.Interop.h>
#include <winrt/Microsoft.UI.Xaml.Markup.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.Navigation.h>
#include <winrt/Microsoft.UI.Xaml.Shapes.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Microsoft.Windows.AppLifecycle.h>
#include <wil/cppwinrt_helpers.h>
#include <idlgen.h>

namespace winrt
{
    namespace waa = Windows::ApplicationModel::Activation;
    namespace wf = Windows::Foundation;
    namespace wg = Windows::Graphics;
    namespace wfc = Windows::Foundation::Collections;
    namespace ws = Windows::Storage;
    namespace mux = Microsoft::UI::Xaml;
    namespace muxc = Microsoft::UI::Xaml::Controls;
    namespace muxm = Microsoft::UI::Xaml::Markup;
    namespace muxmi = Microsoft::UI::Xaml::Media::Imaging;
    namespace mwal = Microsoft::Windows::AppLifecycle;
    namespace mwamr = Microsoft::Windows::ApplicationModel::Resources;
    namespace mud = Microsoft::UI::Dispatching;
    namespace mui = Microsoft::UI::Input;
    namespace muw = Microsoft::UI::Windowing;
}
