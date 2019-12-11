#pragma once

class DemoApp
{
public:
    static WCHAR s_szClassName[MAX_PATH];
    static WCHAR s_szTitle[MAX_PATH];

    static HINSTANCE    s_hInstance;
    static HICON        s_hiconDefault;
    static HICON        s_hiconSmall;

    static void GlobalInit(HINSTANCE hInstance)
    {
        s_hInstance = hInstance;

        ::LoadStringW(s_hInstance, IDC_CLASSNAME, s_szClassName, MAX_PATH);
        ::LoadStringW(s_hInstance, IDS_APP_TITLE, s_szTitle, MAX_PATH);

        s_hiconDefault = ::LoadIconW(s_hInstance, MAKEINTRESOURCEW(IDI_DEFAULT));
        s_hiconSmall = ::LoadIconW(s_hInstance, MAKEINTRESOURCEW(IDI_SMALL));
    }

public:
    DemoApp();
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
    HWND m_hWnd;
    HBITMAP m_hBitmap;

    float m_xOffset;
    float m_yOffset;

    ComPtr<ID3D11Device> m_spD3D11Device;
    ComPtr<IDCompositionDevice> m_spDCompDevice;
    ComPtr<IDCompositionTarget> m_spDCompTarget;
    ComPtr<IDCompositionVisual> m_spVisual;
};

