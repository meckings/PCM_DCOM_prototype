#pragma once
#include "DWMTestApp.h"
#include <d3d11_1.h>
#include <atlbase.h>
#include <windows.h>
#include <dcomp.h>
#include <dwmapi.h>

#include <windowsx.h>
#include <strsafe.h>
#include "resource.h"

#include <mfplay.h>
#include <mferror.h>

namespace DWMTestApp
{
    struct DComPCM 
    {
        DComPCM();
        ~DComPCM();

        HRESULT Initialize(HINSTANCE hInstance, HWND main, HWND child, HWND textBox);

        // Initialize main window functions including layered child window
        HRESULT InitializeMainWindow(HWND main);
        HRESULT InitializeLayeredChildWindows(HWND child, HWND textBox);
        HRESULT MoveLayeredChildWindows();

        // Initialize Direct Composition and D3D functions
        HRESULT CreateD3D11Device();
        HRESULT CreateDCompositionDevice();
        HRESULT CreateDCompositionRenderTarget();
        HRESULT CreateDCompositionVisualTree();
        HRESULT CreateTransforms();

        HRESULT OnCommand(int id);
        HRESULT DoPCM();



        int EnterMessageLoop();

        // Destroy
        VOID Destroy();
        VOID DestroyMainWindow();
        VOID DestroyLayeredChildWindows();
        VOID DestroyDCompositionVisualTree();
        VOID DestroyDCompositionRenderTarget();
        VOID DestroyDCompositionDevice();
        VOID DestroyD3D11Device();
        VOID DestroyTransforms();

    private:
        WCHAR m_fontTypeface[32];
        int   m_fontHeightLogo;
        int   m_fontHeightTitle;
        int   m_fontHeightDescription;

        HINSTANCE m_hInstance;

        HWND  m_hMainWindow;
        HWND  m_hControlChildWindow;
        HWND  m_textBoxHwnd;

        CComPtr<ID3D11Device> _d3d11Device;
        CComPtr<ID3D11DeviceContext> _d3d11DeviceContext;

        CComPtr<IDCompositionDevice> m_pDevice;
        CComPtr<IDCompositionTarget> m_pHwndRenderTarget;

        CComPtr<IDCompositionVisual> m_pRootVisual;
        CComPtr<IDCompositionVisual> m_pControlChildVisual;
        CComPtr<IDCompositionVisual> m_pTextboxChildVisual;

        CComPtr<IUnknown> m_pTextboxSurfaceTile;
        CComPtr<IDCompositionRectangleClip> m_rectangleClip;

        CComPtr<IDCompositionTranslateTransform> m_pControlTranslateTransform;
        CComPtr<IDCompositionTranslateTransform> m_pTextboxTranslateTransform;
    };
}
