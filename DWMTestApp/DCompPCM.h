#pragma once
#include "DWMTestApp.h"
#include <windows.h>
#include <d3d11.h>
#include <dcomp.h>
#include <math.h>

namespace DWMTestApp
{
    struct DComPCM 
    {
        DComPCM();
        ~DComPCM();

        HRESULT Initialize(HWND hwnd);
        HRESULT InitializeDirectCompositionDevice();
        HRESULT CreateResources();
        void DiscardResources();
        HRESULT OnClientClick();
        HRESULT LoadResourceGDIBitmap(PCWSTR resourceName, HBITMAP& hbmp);
        HRESULT MyCreateGDIRenderedDCompSurface(HBITMAP hBitmap, IDCompositionSurface** ppSurface);

    private:
        /*ID3D11Device* m_pD3D11Device;
        D3D_FEATURE_LEVEL featureLevelSupported;
        IDXGIDevice* pDXGIDevice = nullptr;
        IDCompositionDevice* m_pDCompDevice;
        IDCompositionTarget* m_pDCompTarget;
        IDCompositionVisual* pVisual = nullptr;
        IDCompositionSurface* pSurface = nullptr;
        HRESULT hr;*/
        HWND m_hwnd;
        HBITMAP m_hBitmap;
        ID3D11Device* m_pD3D11Device;
        IDCompositionDevice* m_pDCompDevice;
        IDCompositionTarget* m_pDCompTarget;
    };
}
