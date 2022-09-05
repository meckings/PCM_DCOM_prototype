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
        m_hwnd(NULL),
        m_hBitmap(NULL),
        m_pDCompDevice(nullptr),
        m_pDCompTarget(nullptr),
        m_pD3D11Device(nullptr) 
    {}

    DComPCM::~DComPCM()
    {
        SafeRelease(&m_pDCompDevice);
        SafeRelease(&m_pDCompTarget);
        SafeRelease(&m_pD3D11Device);
    }

    HRESULT DComPCM::Initialize(HWND hwnd)
    {
        HRESULT hr;

        // Create the application window.
        //
        // Because the CreateWindow function takes its size in pixels, we
        // obtain the system DPI and use it to scale the window size.
        int dpiX = 0;
        int dpiY = 0;
        HDC hdc = GetDC(NULL);
        if (hdc)
        {
            dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
            dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
            ReleaseDC(NULL, hdc);
        }

        m_hwnd = hwnd;
        /*m_hwnd = CreateWindowEx(
            0,
            L"DirectCompDemoApp",
            L"DirectComposition Demo Application",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            static_cast<UINT>(ceil(640.f * dpiX / 96.f)),
            static_cast<UINT>(ceil(480.f * dpiY / 96.f)),
            NULL,
            NULL,
            HINST_THISCOMPONENT,
            NULL
        );*/

        hr = m_hwnd ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            // Initialize DirectComposition resources, such as the
            // device object and composition target object.
            hr = InitializeDirectCompositionDevice();
        }

        if (SUCCEEDED(hr))
        {
            hr = CreateResources();
        }

        return hr;
    }

    /******************************************************************
    *                                                                 *
    *  This method creates the DirectComposition device object and    *
    *  and the composition target object. These objects endure for    *
    *  the lifetime of the application.                               *
    *                                                                 *
    ******************************************************************/

    HRESULT DComPCM::InitializeDirectCompositionDevice()
    {
        HRESULT hr = S_OK;

        D3D_FEATURE_LEVEL featureLevelSupported;

        // Create the D3D device object. The D3D11_CREATE_DEVICE_BGRA_SUPPORT
        // flag enables rendering on surfaces using Direct2D.
        hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            NULL,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT,
            NULL,
            0,
            D3D11_SDK_VERSION,
            &m_pD3D11Device,
            &featureLevelSupported,
            nullptr);

        IDXGIDevice* pDXGIDevice = nullptr;

        // Check the result of calling D3D11CreateDriver.
        if (SUCCEEDED(hr))
        {
            // Create the DXGI device used to create bitmap surfaces.
            hr = m_pD3D11Device->QueryInterface(&pDXGIDevice);
        }

        if (SUCCEEDED(hr))
        {
            // Create the DirectComposition device object.
            hr = DCompositionCreateDevice(pDXGIDevice, __uuidof(IDCompositionDevice), reinterpret_cast<void**>(&m_pDCompDevice));
        }
        if (SUCCEEDED(hr))
        {
            // Create the composition target object based on the specified application window.
            hr = m_pDCompDevice->CreateTargetForHwnd(m_hwnd, TRUE, &m_pDCompTarget);
        }

        SafeRelease(&pDXGIDevice);

        return hr;
    }

    /******************************************************************
    *                                                                 *
    *  This method creates the GDI bitmap that the application gives  *
    *  to DirectComposition to be composed.                           *
    *                                                                 *
    ******************************************************************/

    HRESULT DComPCM::CreateResources()
    {
        HRESULT hr = S_OK;

        hr = LoadResourceGDIBitmap(L"Logo", m_hBitmap);

        return hr;
    }

    /******************************************************************
    *                                                                 *
    *  Discard device-specific resources.                             *
    *                                                                 *
    ******************************************************************/

    void DComPCM::DiscardResources()
    {
        DeleteObject(m_hBitmap);
    }

    /******************************************************************
    *                                                                 *
    *  Called whenever the user clicks in client area of the main     *
    *  window. This method builds a simple visual tree and passes it  *
    *  to DirectComposition.
    *                                                                 *
    ******************************************************************/

    HRESULT DComPCM::OnClientClick()
    {
        HRESULT hr = S_OK;
        float xOffset = 20; // horizonal position of visual
        float yOffset = 20; // vertical position of visual

        IDCompositionVisual* pVisual = nullptr;

        // Create a visual object.          
        hr = m_pDCompDevice->CreateVisual(&pVisual);

        IDCompositionSurface* pSurface = nullptr;

        if (SUCCEEDED(hr))
        {
            // Create a composition surface and render a GDI bitmap 
            // to the surface.
            hr = MyCreateGDIRenderedDCompSurface(m_hBitmap, &pSurface);
        }

        if (SUCCEEDED(hr))
        {
            // Set the bitmap content of the visual. 
            hr = pVisual->SetContent(pSurface);
        }

        if (SUCCEEDED(hr))
        {
            // Set the horizontal and vertical position of the visual relative
            // to the upper-left corner of the composition target window.
            hr = pVisual->SetOffsetX(xOffset);
            if (SUCCEEDED(hr))
            {
                hr = pVisual->SetOffsetY(yOffset);
            }
        }

        if (SUCCEEDED(hr))
        {
            // Set the visual to be the root of the visual tree.          
            hr = m_pDCompTarget->SetRoot(pVisual);
        }

        if (SUCCEEDED(hr))
        {
            // Commit the visual to be composed and displayed.
            hr = m_pDCompDevice->Commit();
        }

        // Free the visual. 
        SafeRelease(&pVisual);

        return hr;
    }

    /******************************************************************
    *                                                                 *
    *  This method loads the specified GDI bitmap from the            *
    *  application resources, creates a new bitmap that is in a       *
    *  format that DirectComposition can use, and copies the contents *
    *  of the original bitmap to the new bitmap.                      *
    *                                                                 *
    ******************************************************************/
    HRESULT DComPCM::LoadResourceGDIBitmap(PCWSTR resourceName, HBITMAP& hbmp)
    {
        hbmp = static_cast<HBITMAP>(LoadImageW(HINST_THISCOMPONENT, resourceName,
            IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR));

        return hbmp ? S_OK : E_FAIL;
    }

    // MyCreateGDIRenderedDCompSurface - Creates a DirectComposition surface and 
    //   copies the bitmap to the surface. 
    //
    // Parameters:
    //   hBitmap - a GDI bitmap.
    //   ppSurface - the composition surface object.
    //                                                                 
    HRESULT DComPCM::MyCreateGDIRenderedDCompSurface(HBITMAP hBitmap,
        IDCompositionSurface** ppSurface)
    {
        HRESULT hr = S_OK;

        int bitmapWidth = 0;
        int bitmapHeight = 0;
        int bmpSize = 0;
        BITMAP bmp = { };
        HBITMAP hBitmapOld = NULL;

        HDC hSurfaceDC = NULL;
        HDC hBitmapDC = NULL;

        IDXGISurface1* pDXGISurface = nullptr;
        IDCompositionSurface* pDCSurface = nullptr;
        POINT pointOffset = { };

        if (ppSurface == nullptr)
            return E_INVALIDARG;

        // Get information about the bitmap.
        bmpSize = GetObject(hBitmap, sizeof(BITMAP), &bmp);

        hr = bmpSize ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            // Save the bitmap dimensions.
            bitmapWidth = bmp.bmWidth;
            bitmapHeight = bmp.bmHeight;

            // Create a DirectComposition-compatible surface that is the same size 
            // as the bitmap. The DXGI_FORMAT_B8G8R8A8_UNORM flag is required for 
            // rendering on the surface using GDI via GetDC.
            hr = m_pDCompDevice->CreateSurface(bitmapWidth, bitmapHeight,
                DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ALPHA_MODE_IGNORE, &pDCSurface);
        }

        if (SUCCEEDED(hr))
        {
            // Begin rendering to the surface.
            hr = pDCSurface->BeginDraw(NULL, __uuidof(IDXGISurface1),
                reinterpret_cast<void**>(&pDXGISurface), &pointOffset);
        }

        if (SUCCEEDED(hr))
        {
            // Get the device context (DC) for the surface.
            hr = pDXGISurface->GetDC(FALSE, &hSurfaceDC);
        }

        if (SUCCEEDED(hr))
        {
            // Create a compatible DC and select the surface 
            // into the DC.
            hBitmapDC = CreateCompatibleDC(hSurfaceDC);
            if (hBitmapDC != NULL)
            {
                hBitmapOld = (HBITMAP)SelectObject(hBitmapDC, hBitmap);
                BitBlt(hSurfaceDC, pointOffset.x, pointOffset.y,
                    bitmapWidth, bitmapHeight, hBitmapDC, 0, 0, SRCCOPY);

                if (hBitmapOld)
                {
                    SelectObject(hBitmapDC, hBitmapOld);
                }
                DeleteDC(hBitmapDC);
            }

            pDXGISurface->ReleaseDC(NULL);
        }

        // End the rendering.
        pDCSurface->EndDraw();
        *ppSurface = pDCSurface;

        // Call an application-defined macro to free the surface pointer.
        SafeRelease(&pDXGISurface);

        return hr;
    }
}