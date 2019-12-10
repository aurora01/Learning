#pragma once

#include <dcomp.h>      // DirectComposition Header Files
#include <d3d11.h>      // Direct3D Header Files
#include <wrl.h>

using namespace Microsoft::WRL;

#define NUM_VISUALS 4 // number of visuals in the composition

class DemoApp
{
public:
    DemoApp(HINSTANCE hInstance);
    ~DemoApp();

    HRESULT Initialize();

    void RunMessageLoop();

private:
    HRESULT InitializeDirectCompositionDevice();

    HRESULT CreateResources();
    void DiscardResources();

    HRESULT OnClientClick();

    HRESULT LoadResourceGDIBitmap(
        PCWSTR resourceName,
        HBITMAP& hbmp
    );
    HRESULT CreateRootImage(int cx, int cy, HBITMAP &hbmp);

    HRESULT MyCreateGDIRenderedDCompSurface(HBITMAP hBitmap,
        IDCompositionSurface** ppSurface);

    LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    static LRESULT CALLBACK Static_WndProc(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
    );

private:
    HINSTANCE m_hInstance;
    HWND m_hWnd;
    HBITMAP m_hBitmaps[NUM_VISUALS];
 
    float m_xOffset;
    float m_yOffset;

    ComPtr<ID3D11Device> m_spD3D11Device;
    ComPtr<IDCompositionDevice> m_spDCompDevice;
    ComPtr<IDCompositionTarget> m_spDCompTarget;
};

