
#include "framework.h"
#include "DemoApp.h"

#define IMAGE_WIDTH     300
#define IMAGE_HEIGHT    200

WCHAR DemoApp::m_szClassName[MAX_PATH]{};
WCHAR DemoApp::m_szTitle[MAX_PATH]{};

HINSTANCE DemoApp::m_hInstance{};
HICON DemoApp::m_hiconDefault{};
HICON DemoApp::m_hiconSmall{};

DemoApp::DemoApp()
    : m_hWnd(NULL)
    , m_hBitmap(NULL)
    , m_xOffset(20)
    , m_yOffset(20)
    , m_Transparent(0.5)
    , m_Opaque(1.0)
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

HRESULT DemoApp::InitializeUI()
{
    HRESULT hr = S_OK;
    ComPtr<IDCompositionSurface> spSurface = nullptr;

    // Create a visual object.          
    hr = m_spDCompDevice->CreateVisual(m_spVisual.GetAddressOf());

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
        m_spVisual->SetOffsetX(float(m_xOffset));
        m_spVisual->SetOffsetY(float(m_yOffset));
        hr = SetVisualOpacity(m_spVisual.Get(), m_Transparent);
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

HRESULT DemoApp::OnMouseMove(int xPos, int yPos)
{
    HRESULT hr = S_OK;
    static BOOL fOverImage = FALSE;

    // Determine whether the cursor is over the visual.
    if ((xPos >= m_xOffset && xPos <= (m_xOffset + IMAGE_WIDTH))
        && (yPos >= m_xOffset && yPos <= (m_xOffset + IMAGE_HEIGHT)))
    {
        if (!fOverImage)
        {
            // The cursor has moved over the visual, so make the visual 
            // 100% opaque.
            hr = SetVisualOpacity(m_spVisual.Get(), m_Opaque);
            fOverImage = TRUE;
        }
    }

    else if (fOverImage)
    {
        // The cursor has moved off the visual, so make the visual
        // 50% opaque.
        hr = SetVisualOpacity(m_spVisual.Get(), m_Transparent);
        fOverImage = FALSE;
    }

    return hr;
}

HRESULT DemoApp::SetVisualOpacity(IDCompositionVisual* pVisual, float opacity)
{
    // Validate the input arguments.
    if (pVisual == NULL || (opacity > 1.0f || opacity < 0.0f))
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    ComPtr<IDCompositionVisual> spVisual = pVisual;
    ComPtr<IDCompositionEffectGroup> spEffectGroup = nullptr;

    // Create an effect group object.
    hr = m_spDCompDevice->CreateEffectGroup(spEffectGroup.GetAddressOf());

    if (SUCCEEDED(hr))
    {
        // Set the Opacity of the effect group object.
        hr = spEffectGroup->SetOpacity(opacity);
    }

    if (SUCCEEDED(hr))
    {
        // Apply the effect group object to the Effect property of the visual.
        hr = spVisual->SetEffect(spEffectGroup.Get());
    }

    if (SUCCEEDED(hr))
    {
        // Commit the visual to DirectComposition.
        hr = m_spDCompDevice->Commit();
    }

    return hr;
}

HRESULT DemoApp::OnClientClick(int xPos, int yPos)
{
    HRESULT hr = S_OK;

    // Determine whether the mouse cursor is over the visual. If so,
    // rotate the visual.
    if ((xPos >= m_xOffset && xPos <= (m_xOffset + IMAGE_WIDTH))
        && (yPos >= m_yOffset && yPos <= (m_yOffset + IMAGE_HEIGHT)))
    {
        hr = RotateVisual(m_spVisual.Get(), 360.0f);
    }

    return hr;
}

HRESULT DemoApp::RotateVisual(IDCompositionVisual* pVisual, float degrees)
{
    // Validate the input arguments.
    if (pVisual == NULL || (degrees > 360.0f || degrees < -360.0f))
        return E_INVALIDARG;

    HRESULT hr = S_OK;

    ComPtr<IDCompositionVisual> spVisual = pVisual;
    ComPtr<IDCompositionAnimation> spAnimation = nullptr;
    ComPtr<IDCompositionRotateTransform3D> spRotate3D = nullptr;
    ComPtr<IDCompositionEffectGroup> spEffectGroup = nullptr;

    // Create a 3D rotate transform object.
    hr = m_spDCompDevice->CreateRotateTransform3D(spRotate3D.GetAddressOf());

    if (SUCCEEDED(hr))
    {
        // Create an effect group object.
        hr = m_spDCompDevice->CreateEffectGroup(spEffectGroup.GetAddressOf());
    }

    if (SUCCEEDED(hr))
    {
        // Create an animation object.
        hr = m_spDCompDevice->CreateAnimation(spAnimation.GetAddressOf());
    }

    if (SUCCEEDED(hr))
    {
        // Define the animation function.
        spAnimation->AddCubic(0.0f, 0.0f, degrees, 0.0f, 0.0f);
        spAnimation->End(1.0f, degrees);

        // Set the properties of the rotate transform object.  
        //
        // Apply the animation object to the Angle property so that
        // the visual will appear to spin around an axis. 
        spRotate3D->SetAngle(spAnimation.Get());

        // Set a vertical axis through the center of the visual's bitmap. 
        spRotate3D->SetAxisX(0.0f);
        spRotate3D->SetAxisY(1.0f);
        spRotate3D->SetAxisZ(0.0f);

        // Set the center of rotation to the center of the visual's bitmap.
        spRotate3D->SetCenterX(IMAGE_WIDTH / 2.0f);
        spRotate3D->SetCenterY(IMAGE_HEIGHT / 2.0f);
    }

    if (SUCCEEDED(hr))
    {
        // Apply the rotate transform object to the Tranform3D property
        // of the effect group object.
        hr = spEffectGroup->SetTransform3D(spRotate3D.Get());
    }

    if (SUCCEEDED(hr))
    {
        // Apply the effect group object to the Effect property of the visual.
        hr = spVisual->SetEffect(spEffectGroup.Get());
    }

    if (SUCCEEDED(hr))
    {
        // Commit the visual to DirectComposition.
        hr = m_spDCompDevice->Commit();
    }

    return hr;
}

LRESULT DemoApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static bool bRepaint = true;
    LRESULT result{ 0 };

    switch (message)
    {
    case WM_CREATE:
        result = 1;
        break;
    case WM_DISPLAYCHANGE:
        bRepaint = true;
        ::InvalidateRect(hWnd, NULL, FALSE);
        break;
    case WM_PAINT:
        if (bRepaint)
        {
            InitializeUI();
            bRepaint = false;
        }
        ::ValidateRect(hWnd, nullptr);
        break;
    case WM_LBUTTONDOWN:
        OnClientClick(LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_MOUSEMOVE:
        OnMouseMove(LOWORD(lParam), HIWORD(lParam));
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

    return result;
}

LRESULT CALLBACK DemoApp::Static_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    DemoApp* pDemoApp = reinterpret_cast<DemoApp*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));

    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        pDemoApp = (DemoApp*)pcs->lpCreateParams;

        ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, LONG_PTR(pDemoApp));
    }

    return pDemoApp->WndProc(hWnd, message, wParam, lParam);
}
