#include "pch.h"
#include "PCMApp.h"
#if __has_include("PCMApp.g.cpp")
#include "PCMApp.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::DWMTestApp::implementation
{
    int32_t PCMApp::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void PCMApp::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void PCMApp::ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        Button().Content(box_value(L"Clicked"));
    }
}
