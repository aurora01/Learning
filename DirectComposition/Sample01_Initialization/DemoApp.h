#pragma once

class DemoApp
{
public:
    static WCHAR m_szClassName[MAX_PATH];
    static WCHAR m_szTitle[MAX_PATH];

    static HINSTANCE    m_hInstance;
    static HICON        m_hiconDefault;
    static HICON        m_hiconSmall;

    static void GlobalInit(HINSTANCE hInstance)
    {
        m_hInstance = hInstance;

        ::LoadStringW(m_hInstance, IDC_CLASSNAME, m_szClassName, MAX_PATH);
        ::LoadStringW(m_hInstance, IDS_APP_TITLE, m_szTitle, MAX_PATH);

        m_hiconDefault = ::LoadIconW(m_hInstance, MAKEINTRESOURCEW(IDI_DEFAULT));
        m_hiconSmall = ::LoadIconW(m_hInstance, MAKEINTRESOURCEW(IDI_SMALL));
    }

public:
    DemoApp();
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
    HWND m_hWnd;
    HBITMAP m_hBitmap;
 
    float m_xOffset;
    float m_yOffset;

    ComPtr<ID3D11Device> m_spD3D11Device;
    ComPtr<IDCompositionDevice> m_spDCompDevice;
    ComPtr<IDCompositionTarget> m_spDCompTarget;
};

