#pragma once

class DemoApp
{
public:
    DemoApp(HINSTANCE hInstance);
    ~DemoApp();

    HRESULT Initialize();

    void RunMessageLoop();

private:
    HRESULT InitializeDirectCompositionDevice();
    HRESULT InitializeUI();

    HRESULT CreateResources();
    void DiscardResources();

    HRESULT LoadResourceGDIBitmap(
        PCWSTR resourceName,
        HBITMAP& hbmp
    );

    HRESULT MyCreateGDIRenderedDCompSurface(HBITMAP hBitmap,
        IDCompositionSurface** ppSurface);

    void OnLMouseDown();
    void OnRMouseDown();

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
    HBITMAP m_hBitmap;

    float m_xOffset;
    float m_yOffset;

    ComPtr<ID3D11Device> m_spD3D11Device;
    ComPtr<IDCompositionDevice> m_spDCompDevice;
    ComPtr<IDCompositionTarget> m_spDCompTarget;
    ComPtr<IDCompositionVisual> m_spVisual;
};

