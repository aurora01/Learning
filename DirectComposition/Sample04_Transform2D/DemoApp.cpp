
#include "framework.h"
#include "DemoApp.h"

#define IMAGE_WIDTH     100
#define IMAGE_HEIGHT    66

WCHAR DemoApp::m_szClassName[MAX_PATH]{};
WCHAR DemoApp::m_szTitle[MAX_PATH]{};

HINSTANCE DemoApp::m_hInstance{};
HICON DemoApp::m_hiconDefault{};
HICON DemoApp::m_hiconSmall{};

DemoApp::DemoApp()
    : m_hWnd(NULL)
    , m_hBitmap(NULL)
    , m_xOffset(10)
    , m_yOffset(10)
{
}

DemoApp::~DemoApp()
{
    DiscardResources();
}

HRESULT DemoApp::Initialize()
{
    HRESULT hr = S_OK;

    // Register the window class.
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = DemoApp::Static_WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(LONG_PTR);
    wcex.hInstance = m_hInstance;
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);;
    wcex.lpszMenuName = nullptr;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.lpszClassName = m_szClassName;
    wcex.hIcon = m_hiconDefault;
    wcex.hIconSm = m_hiconSmall;

    RegisterClassEx(&wcex);

    // Create the application window.
    //
    // Because the CreateWindow function takes its size in pixels, we
    // obtain the system DPI and use it to scale the window size.
    int dpiX = 0;
    int dpiY = 0;
    HDC hdc = ::GetDC(NULL);
    if (hdc)
    {
        dpiX = ::GetDeviceCaps(hdc, LOGPIXELSX);
        dpiY = ::GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(NULL, hdc);
    }

    m_hWnd = ::CreateWindow(
        m_szClassName,
        m_szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        static_cast<UINT>(ceil(640.f * dpiX / 96.f)),
        static_cast<UINT>(ceil(480.f * dpiY / 96.f)),
        NULL,
        NULL,
        m_hInstance,
        this
    );

    hr = m_hWnd ? S_OK : E_FAIL;
    if (SUCCEEDED(hr))
    {
        // Initialize DirectComposition resources, such as the
        // device object and composition target object.
        hr = InitializeDirectCompositionDevice();
        if (SUCCEEDED(hr))
        {
            hr = CreateResources();
            if (SUCCEEDED(hr))
            {
                InitializeUI();
            }

        }

        ShowWindow(m_hWnd, SW_SHOWNORMAL);
        UpdateWindow(m_hWnd);

    }

    return hr;

}

void DemoApp::RunMessageLoop()
{
    MSG msg;

    while (::GetMessage(&msg, NULL, 0, 0))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }
}

/******************************************************************
*                                                                 *
*  This method creates the DirectComposition device object and    *
*  and the composition target object. These objects endure for    *
*  the lifetime of the application.                               *
*                                                                 *
******************************************************************/
#pragma warning(push)
#pragma warning(disable : 26812) // disabling a warning when including a header works normally for most warnin
HRESULT DemoApp::InitializeDirectCompositionDevice()
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
        m_spD3D11Device.GetAddressOf(),
        &featureLevelSupported,
        nullptr);

    ComPtr<IDXGIDevice> spDXGIDevice;

    // Check the result of calling D3D11CreateDriver.
    if (SUCCEEDED(hr))
    {
        // Create the DXGI device used to create bitmap surfaces.
        hr = m_spD3D11Device->QueryInterface(spDXGIDevice.GetAddressOf());
    }

    if (SUCCEEDED(hr))
    {
        // Create the DirectComposition device object.
        hr = DCompositionCreateDevice(spDXGIDevice.Get(),
            __uuidof(IDCompositionDevice),
            reinterpret_cast<void**>(m_spDCompDevice.GetAddressOf()));
    }
    if (SUCCEEDED(hr))
    {
        // Create the composition target object based on the 
        // specified application window.
        hr = m_spDCompDevice->CreateTargetForHwnd(m_hWnd, TRUE, m_spDCompTarget.GetAddressOf());
    }

    return hr;
}
#pragma warning(pop)

HRESULT DemoApp::InitializeUI()
{
    HRESULT hr = S_OK;


    // Create a visual object.          
    hr = m_spDCompDevice->CreateVisual(m_spVisual.GetAddressOf());

    ComPtr<IDCompositionSurface> spSurface;

    if (SUCCEEDED(hr))
    {
        // Create a composition surface and render a GDI bitmap 
        // to the surface.
        hr = MyCreateGDIRenderedDCompSurface(m_hBitmap, spSurface.GetAddressOf());
    }

    if (SUCCEEDED(hr))
    {
        // Set the bitmap content of the visual. 
        hr = m_spVisual->SetContent(spSurface.Get());
    }

    if (SUCCEEDED(hr))
    {
        // Set the horizontal and vertical position of the visual relative
        // to the upper-left corner of the composition target window.
        hr = m_spVisual->SetOffsetX(m_xOffset);
        if (SUCCEEDED(hr))
        {
            hr = m_spVisual->SetOffsetY(m_yOffset);
        }
    }

    if (SUCCEEDED(hr))
    {
        // Set the visual to be the root of the visual tree.          
        hr = m_spDCompTarget->SetRoot(m_spVisual.Get());
    }

    if (SUCCEEDED(hr))
    {
        // Commit the visual to be composed and displayed.
        hr = m_spDCompDevice->Commit();
    }

    return hr;
}

HRESULT DemoApp::CreateResources()
{
    HRESULT hr = S_OK;

    hr = LoadResourceGDIBitmap(MAKEINTRESOURCE(IDB_BITMAP1), m_hBitmap);

    return hr;
}

void DemoApp::DiscardResources()
{
    if (m_hBitmap != NULL)
    {
        ::DeleteObject(m_hBitmap);
        m_hBitmap = NULL;
    }
}

HRESULT DemoApp::LoadResourceGDIBitmap(PCWSTR resourceName, HBITMAP& hbmp)
{
    hbmp = static_cast<HBITMAP>(::LoadImageW(m_hInstance, resourceName, IMAGE_BITMAP, IMAGE_WIDTH, IMAGE_HEIGHT, LR_DEFAULTCOLOR));

    return hbmp ? S_OK : E_FAIL;
}

// MyCreateGDIRenderedDCompSurface - Creates a DirectComposition surface and 
//   copies the bitmap to the surface. 
//
// Parameters:
//   hBitmap - a GDI bitmap.
//   ppSurface - the composition surface object.
//      
HRESULT DemoApp::MyCreateGDIRenderedDCompSurface(HBITMAP hBitmap, IDCompositionSurface** ppSurface)
{
    HRESULT hr = S_OK;

    int bitmapWidth = 0;
    int bitmapHeight = 0;
    int bmpSize = 0;
    BITMAP bmp = { };
    HBITMAP hBitmapOld = NULL;

    HDC hSurfaceDC = NULL;
    HDC hBitmapDC = NULL;

    ComPtr<IDXGISurface1> spDXGISurface;
    ComPtr<IDCompositionSurface> spDCSurface;
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
        hr = m_spDCompDevice->CreateSurface(bitmapWidth, bitmapHeight,
            DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ALPHA_MODE_IGNORE, spDCSurface.GetAddressOf());
    }

    if (SUCCEEDED(hr))
    {
        // Begin rendering to the surface.
        hr = spDCSurface->BeginDraw(NULL, __uuidof(IDXGISurface1),
            reinterpret_cast<void**>(spDXGISurface.GetAddressOf()), &pointOffset);
    }

    if (SUCCEEDED(hr))
    {
        // Get the device context (DC) for the surface.
        hr = spDXGISurface->GetDC(FALSE, &hSurfaceDC);
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

        spDXGISurface->ReleaseDC(NULL);
    }

    // End the rendering.
    spDCSurface->EndDraw();
    spDCSurface.CopyTo(ppSurface);

    return hr;
}

void DemoApp::OnLMouseDown()
{
    IDCompositionTransform* pTransforms[3];

    ComPtr<IDCompositionRotateTransform> spRotateTransform;
    ComPtr<IDCompositionScaleTransform> spScaleTransform;
    ComPtr<IDCompositionTranslateTransform> spTranslateTransform;
    ComPtr<IDCompositionTransform> spTransformGroup;

    // Create the rotate transform.
    HRESULT hr = m_spDCompDevice->CreateRotateTransform(spRotateTransform.GetAddressOf());
    if (SUCCEEDED(hr))
    {
        // Set the center of rotation to the center point of the visual.
        hr = spRotateTransform->SetCenterX(IMAGE_WIDTH / 2.0f);

        if (SUCCEEDED(hr)) {
            hr = spRotateTransform->SetCenterY(IMAGE_HEIGHT / 2.0f);
        }

        // Set the rotate angle to 180 degrees.
        if (SUCCEEDED(hr)) {
            hr = spRotateTransform->SetAngle(180.0f);
        }

        // Add the rotate transform to the transform group array.
        pTransforms[0] = spRotateTransform.Get();

        // Create the scale transform.
        if (SUCCEEDED(hr)) {
            hr = m_spDCompDevice->CreateScaleTransform(spScaleTransform.GetAddressOf());
        }
    }

    if (SUCCEEDED(hr))
    {
        // Set the scaling origin to the upper-right corner of the visual.
        hr = spScaleTransform->SetCenterX(0.0f);
        if (SUCCEEDED(hr)) {
            hr = spScaleTransform->SetCenterY(0.0f);
        }

        // Set the scaling factor to three for both the width and height. 
        if (SUCCEEDED(hr)) {
            hr = spScaleTransform->SetScaleX(3.0f);
        }
        if (SUCCEEDED(hr)) {
            hr = spScaleTransform->SetScaleY(3.0f);
        }

        // Add the scale transform to the transform group array.
        pTransforms[1] = spScaleTransform.Get();

        // Create the translate transform.
        if (SUCCEEDED(hr)) {
            hr = m_spDCompDevice->CreateTranslateTransform(spTranslateTransform.GetAddressOf());
        }
    }

    if (SUCCEEDED(hr))
    {
        POINT ptMouse = { };
        GetCursorPos(&ptMouse);
        ScreenToClient(m_hWnd, &ptMouse);
        
        // Move the visual 150 pixels to the right.
        //hr = spTranslateTransform->SetOffsetX(150.0f);
        //if (SUCCEEDED(hr)) {
        //    hr = spTranslateTransform->SetOffsetY(0.0f);
        //}


        // Move the visual 150 pixels to the right.
        hr = spTranslateTransform->SetOffsetX(ptMouse.x - m_xOffset);
        if (SUCCEEDED(hr)) {
            hr = spTranslateTransform->SetOffsetY(ptMouse.y - m_yOffset);
        }
        // Add the translate transform to the transform group array.
        pTransforms[2] = spTranslateTransform.Get();
    }

    if (SUCCEEDED(hr))
    {
        // Create the transform group.
        hr = m_spDCompDevice->CreateTransformGroup(pTransforms, 3, spTransformGroup.GetAddressOf());
    }

    if (SUCCEEDED(hr))
    {
        // Apply the transform group to the visual.
        hr = m_spVisual->SetTransform(spTransformGroup.Get());
    }

    if (SUCCEEDED(hr))
    {
        // Commit the composition.
        hr = m_spDCompDevice->Commit();
    }
}

void DemoApp::OnRMouseDown()
{
    HRESULT hr = m_spVisual->SetTransform(nullptr);
    if (SUCCEEDED(hr))
    {
        m_spDCompDevice->Commit();
    }
}

LRESULT DemoApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_LBUTTONDOWN:
        OnLMouseDown();
        break;
    case WM_RBUTTONDOWN:
        OnRMouseDown();
        break;
    case WM_DISPLAYCHANGE:
        ::InvalidateRect(hWnd, NULL, FALSE);
        break;
    case WM_CLOSE:
        DiscardResources();
        ::DestroyWindow(hWnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}


LRESULT CALLBACK DemoApp::Static_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result{ 0 };

    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        DemoApp* pDemoApp = (DemoApp*)pcs->lpCreateParams;

        ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, LONG_PTR(pDemoApp));

        result = 1;
    }
    else
    {
        DemoApp* pDemoApp = reinterpret_cast<DemoApp*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));
        result = pDemoApp->WndProc(hWnd, message, wParam, lParam);
    }

    return result;
}