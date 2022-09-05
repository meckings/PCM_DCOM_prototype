#include "pch.h"
#include "PCMControl.h"
#if __has_include("PCMControl.g.cpp")
#include "PCMControl.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::PCMApp::implementation
{
    int32_t PCMControl::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void PCMControl::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void PCMControl::ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        Button().Content(box_value(L"Clicked"));
    }
}
