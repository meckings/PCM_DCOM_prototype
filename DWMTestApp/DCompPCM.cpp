#include "DCompPCM.h"


/******************************************************************
*                                                                 *
*  Macros                                                         *
*                                                                 *
******************************************************************/
template<class Interface>
inline void
SafeRelease(
    Interface** ppInterfaceToRelease
)
{
    if (*ppInterfaceToRelease != NULL)
    {
        (*ppInterfaceToRelease)->Release();

        (*ppInterfaceToRelease) = NULL;
    }
}

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

namespace DWMTestApp
{
    DComPCM::DComPCM() :
        m_hInstance(NULL),
        m_hMainWindow(NULL),
        m_textBoxHwnd(NULL)
    {}

    DComPCM::~DComPCM()
    {
        
    }

    HRESULT DComPCM::Initialize(HINSTANCE hInstance, HWND main, HWND child, HWND textBox)
    {
        m_hInstance = hInstance;
        HRESULT hr = InitializeMainWindow(main);

        if (SUCCEEDED(hr))
        {
            hr = InitializeLayeredChildWindows(child, textBox);
        }

        if (SUCCEEDED(hr))
        {
            hr = MoveLayeredChildWindows();
        }

        if (SUCCEEDED(hr))
        {
            hr = CreateD3D11Device();
        }

        if (SUCCEEDED(hr))
        {
            hr = CreateDCompositionDevice();
        }

        if (SUCCEEDED(hr))
        {
            hr = CreateDCompositionRenderTarget();
        }

        if (SUCCEEDED(hr))
        {
            hr = CreateDCompositionVisualTree();
        }

        if (SUCCEEDED(hr))
        {
            hr = CreateTransforms();
        }

        if (SUCCEEDED(hr))
        {
            // Commit the batch.
            hr = m_pDevice->Commit();
        }

        return hr;
    }

    // Creates the main application window.
    HRESULT DComPCM::InitializeMainWindow(HWND hwnd)
    {
        HRESULT hr = S_OK;

        m_hMainWindow = hwnd;
        ShowWindow(m_hMainWindow, SW_SHOWDEFAULT);
        return hr;
    }

    // Creates the layered child windows.
    HRESULT DComPCM::InitializeLayeredChildWindows(HWND child, HWND textBox)
    {
        HRESULT hr = S_OK;

        m_hControlChildWindow = child;
        m_textBoxHwnd = textBox;

        if (SUCCEEDED(hr))
        {
            ShowWindow(m_hControlChildWindow, SW_SHOWDEFAULT);
        }

        return hr;
    }

    HRESULT DComPCM::MoveLayeredChildWindows()
    {
        HRESULT hr = S_OK;

        RECT rcClient;
        GetClientRect(m_hMainWindow, &rcClient);

        // Move the Control window to its location.
        if (!MoveWindow(m_hControlChildWindow,  // Window
            rcClient.left,                // New position of the left side of the window
            rcClient.top,                 // New position of the top side of the window
            rcClient.right,               // New width
            rcClient.bottom,              // New height
            TRUE))                        // Repaint the window
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

        return hr;
    }

    HRESULT DComPCM::CreateD3D11Device()
    {
        HRESULT hr = S_OK;

        D3D_DRIVER_TYPE driverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
        };

        D3D_FEATURE_LEVEL featureLevelSupported;

        for (int i = 0; i < sizeof(driverTypes) / sizeof(driverTypes[0]); ++i)
        {
            CComPtr<ID3D11Device> d3d11Device;
            CComPtr<ID3D11DeviceContext> d3d11DeviceContext;

            hr = D3D11CreateDevice(
                nullptr,
                driverTypes[i],
                NULL,
                D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                NULL,
                0,
                D3D11_SDK_VERSION,
                &d3d11Device,
                &featureLevelSupported,
                &d3d11DeviceContext);

            if (SUCCEEDED(hr))
            {
                _d3d11Device = d3d11Device.Detach();
                _d3d11DeviceContext = d3d11DeviceContext.Detach();

                break;
            }
        }

        return hr;
    }

    HRESULT DComPCM::CreateDCompositionDevice()
    {
        HRESULT hr = (_d3d11Device == nullptr) ? E_UNEXPECTED : S_OK;

        CComPtr<IDXGIDevice> dxgiDevice;

        if (SUCCEEDED(hr))
        {
            hr = _d3d11Device->QueryInterface(&dxgiDevice);
        }

        if (SUCCEEDED(hr))
        {
            hr = DCompositionCreateDevice(dxgiDevice, __uuidof(IDCompositionDevice), reinterpret_cast<void**>(&m_pDevice));
        }

        return hr;
    }

    HRESULT DComPCM::CreateDCompositionRenderTarget()
    {
        HRESULT hr = ((m_pDevice == nullptr) || (m_hMainWindow == NULL)) ? E_UNEXPECTED : S_OK;

        if (SUCCEEDED(hr))
        {
            // FALSE puts the composition content beneath the Win32 buttons.
            hr = m_pDevice->CreateTargetForHwnd(m_hMainWindow, FALSE, &m_pHwndRenderTarget);
        }

        return hr;
    }

    HRESULT DComPCM::CreateDCompositionVisualTree()
    {
        HRESULT hr = ((m_pDevice == nullptr) || (m_hMainWindow == NULL)) ? E_UNEXPECTED : S_OK;

        if (SUCCEEDED(hr))
        {
            // Create the root visual.
            hr = m_pDevice->CreateVisual(&m_pRootVisual);
        }

        if (SUCCEEDED(hr))
        {
            // Make the visual the root of the tree.
            hr = m_pHwndRenderTarget->SetRoot(m_pRootVisual);
        }

        if (SUCCEEDED(hr))
        {
            // Make the visual the root of the tree.
            hr = m_pDevice->CreateVisual(&m_pControlChildVisual);
        }

        if (SUCCEEDED(hr))
        {
            // Make the visual the root of the tree.
            hr = m_pRootVisual->AddVisual(m_pControlChildVisual, TRUE, NULL);
        }
        return hr;
    }

    HRESULT DComPCM::CreateTransforms()
    {
        RECT rcClient;

        GetClientRect(m_hMainWindow, &rcClient);

        // Create a translate transform for the control child visual.
        HRESULT hr = m_pDevice->CreateTranslateTransform(&m_pControlTranslateTransform);

        if (SUCCEEDED(hr))
        {
            // Set the offset of x-axis of the control child visual
            hr = m_pControlChildVisual->SetOffsetX(0.0f);
        }

        if (SUCCEEDED(hr))
        {
            // Set the offset of y-axis of the control child visual
            hr = m_pControlChildVisual->SetOffsetY(0.0f);
        }

        if (SUCCEEDED(hr))
        {
            // Set the offset of x-axis of control translate transform
            hr = m_pControlTranslateTransform->SetOffsetX(0.0f);
        }

        if (SUCCEEDED(hr))
        {
            // Set the offset of y-axis of control translate transform
            hr = m_pControlTranslateTransform->SetOffsetY(0.0f);
        }

        if (SUCCEEDED(hr))
        {
            // Apply translate transform to the control child visual.
            hr = m_pControlChildVisual->SetTransform(m_pControlTranslateTransform);
        }

        return hr;
    }

    // Handles the WM_COMMAND message.
    HRESULT DComPCM::OnCommand(int id)
    {
        HRESULT hr = S_OK;

        int wmId = LOWORD(id);
        switch (wmId)
        {
        case IDC_TEXTBOX:
            hr = DoPCM();
            break;
        }

        return hr;
    }

    HRESULT DComPCM::DoPCM() {

        HRESULT hr{ S_OK };
        if (m_textBoxHwnd) {

            RECT cursorRect;
            GetCursorPos((LPPOINT)&cursorRect);
            POINT point;
            GetCursorPos(&point);

            RECT rect;
            GetWindowRect(m_textBoxHwnd, &rect);

            MapWindowPoints(HWND_DESKTOP, GetParent(m_textBoxHwnd), (LPPOINT)&rect, 2);
            MapWindowPoints(HWND_DESKTOP, GetParent(m_textBoxHwnd), (LPPOINT)&cursorRect, 2);
            ScreenToClient(GetParent(m_textBoxHwnd), &point);

            if (!m_pTextboxSurfaceTile) {

                hr = m_pDevice->CreateSurfaceFromHwnd(m_textBoxHwnd, &m_pTextboxSurfaceTile);

                if (SUCCEEDED(hr))
                {
                    // Create the TextBox child visual.
                    hr = m_pDevice->CreateVisual(&m_pTextboxChildVisual);
                }

                if (SUCCEEDED(hr))
                {
                    // Set the content of the text box child visual.
                    hr = m_pTextboxChildVisual->SetContent(m_pTextboxSurfaceTile);
                }

                if (SUCCEEDED(hr))
                {
                    // Add the textbox child visual to the visual tree.
                    hr = m_pRootVisual->AddVisual(m_pTextboxChildVisual, TRUE, NULL);
                }

                if (SUCCEEDED(hr))
                {
                    // Create a translate transform for the control child visual. Without creating a transform the visual won't be seen
                    hr = m_pDevice->CreateTranslateTransform(&m_pTextboxTranslateTransform);
                }

                if (SUCCEEDED(hr))
                {
                    float height = rect.bottom - rect.top;
                    float margin = 5;
                    m_pTextboxChildVisual->SetOffsetY(rect.top - height - margin);
                    m_pTextboxChildVisual->SetOffsetX(rect.left);

                    // Set the offset of x-axis of control translate transform
                    hr = m_pTextboxTranslateTransform->SetOffsetX(0.0f);

                    // Set the offset of y-axis of control translate transform
                    hr = m_pTextboxTranslateTransform->SetOffsetY(0.0f);
                }

                if (SUCCEEDED(hr))
                {
                    // Create the TextBox child visual.
                    hr = m_pDevice->CreateRectangleClip(&m_rectangleClip);
                }
            }
            else {

                hr = m_pRootVisual->RemoveVisual(m_pTextboxChildVisual);

                if (SUCCEEDED(hr))
                {
                    // Add the textbox child visual to the visual tree.
                    hr = m_pRootVisual->AddVisual(m_pTextboxChildVisual, TRUE, NULL);
                }

                /*m_pRootVisual->RemoveVisual(m_pTextboxChildVisual);
                m_pTextboxChildVisual = nullptr;
                m_pTextboxTranslateTransform = nullptr;
                m_pTextboxSurfaceTile = nullptr;
                m_pRootVisual->SetTransform(m_pTextboxTranslateTransform);

                m_pDevice->Commit();*/
            }
            if (SUCCEEDED(hr))
            {
                float height = rect.bottom - rect.top;
                float width = rect.right - rect.left;

                m_rectangleClip->SetTop(0.0f);
                m_rectangleClip->SetLeft(cursorRect.left - rect.left);
                m_rectangleClip->SetRight(cursorRect.left - rect.left + 50);
                m_rectangleClip->SetBottom(height);

                hr = m_pTextboxChildVisual->SetClip(m_rectangleClip);
            }
            if (SUCCEEDED(hr))
            {
                // Apply translate transform to the control child visual.
                hr = m_pRootVisual->SetTransform(m_pTextboxTranslateTransform);
            }

            if (SUCCEEDED(hr))
            {
                m_pDevice->Commit();
            }
        }
        return hr;
    }

    VOID DComPCM::Destroy()
    {
        DestroyTransforms();
        DestroyMainWindow();
        DestroyLayeredChildWindows();
        DestroyDCompositionVisualTree();
        DestroyDCompositionRenderTarget();
        DestroyDCompositionDevice();
        DestroyD3D11Device();
        CoUninitialize();
    }

    VOID DComPCM::DestroyTransforms()
    {
        m_pControlTranslateTransform = nullptr;

        m_pTextboxTranslateTransform = nullptr;
    }

    VOID DComPCM::DestroyMainWindow()
    {
        if (m_hMainWindow != NULL)
        {
            DestroyWindow(m_hMainWindow);
            m_hMainWindow = NULL;
        }
    }

    VOID DComPCM::DestroyLayeredChildWindows()
    {
        if (m_hControlChildWindow != NULL)
        {
            DestroyWindow(m_hControlChildWindow);
            m_hControlChildWindow = NULL;
        }

        if (m_textBoxHwnd != NULL)
        {
            DestroyWindow(m_textBoxHwnd);
            m_textBoxHwnd = NULL;
        }
    }

    VOID DComPCM::DestroyDCompositionVisualTree()
    {
        m_pTextboxSurfaceTile = nullptr;
    }

    VOID DComPCM::DestroyDCompositionRenderTarget()
    {
        m_pHwndRenderTarget = nullptr;
    }

    VOID DComPCM::DestroyDCompositionDevice()
    {
        m_pDevice = nullptr;
    }

    VOID DComPCM::DestroyD3D11Device()
    {
        _d3d11DeviceContext = nullptr;
        _d3d11Device = nullptr;
    }
}