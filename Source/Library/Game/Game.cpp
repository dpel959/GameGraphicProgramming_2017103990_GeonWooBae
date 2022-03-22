#include "Game/Game.h"

namespace library
{
    /*--------------------------------------------------------------------
      Global Variables
    --------------------------------------------------------------------*/
    HINSTANCE               g_hInst = nullptr;
    HWND                    g_hWnd = nullptr;
    D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
    D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
    // 얘들은 COM이 없나?

    ComPtr<ID3D11Device> g_pd3dDevice(nullptr);
    ComPtr<ID3D11Device1> g_pd3dDevice1(nullptr);
    ComPtr<ID3D11DeviceContext> g_pImmediateContext(nullptr);
    ComPtr<ID3D11DeviceContext1> g_pImmediateContext1(nullptr);
    ComPtr<IDXGISwapChain> g_pSwapChain(nullptr);
    ComPtr<IDXGISwapChain1> g_pSwapChain1(nullptr);
    ComPtr<ID3D11RenderTargetView> g_pRenderTargetView(nullptr);

    /*--------------------------------------------------------------------
      Forward declarations
    --------------------------------------------------------------------*/

    /*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      Function: WindowProc

      Summary:  Defines the behavior of the window—its appearance, how
                it interacts with the user, and so forth

      Args:     HWND hWnd
                  Handle to the window
                UINT uMsg
                  Message code
                WPARAM wParam
                  Additional data that pertains to the message
                LPARAM lParam
                  Additional data that pertains to the message

      Returns:  LRESULT
                  Integer value that your program returns to Windows
    -----------------------------------------------------------------F-F*/
    LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
        switch (uMsg) {
        case WM_CLOSE:
            if (MessageBox(hWnd, L"Really quit?", L"Game Graphic Programming", MB_OKCANCEL) == IDOK) {
                DestroyWindow(hWnd);
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }

        return 0;
    }

    HRESULT InitWindow(_In_ HINSTANCE hInstance, _In_ INT nCmdShow) {

        WNDCLASSEX wcex =
        {
            .cbSize = sizeof(WNDCLASSEX),
            .style = CS_HREDRAW | CS_VREDRAW,
            .lpfnWndProc = WindowProc,
            .cbClsExtra = 0,
            .cbWndExtra = 0,
            .hInstance = hInstance,
            .hIcon = LoadIcon(hInstance, IDI_APPLICATION),
            .hCursor = LoadCursor(nullptr, IDC_ARROW),
            .hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
            .lpszMenuName = nullptr,
            .lpszClassName = L"Game Graphics Window Class",
            .hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION),
        };

        if (!RegisterClassEx(&wcex)) {
            DWORD dwError = GetLastError();

            MessageBox(nullptr, L"Call RegitserClassEx", L"FAILED", NULL);

            if (dwError != ERROR_CLASS_ALREADY_EXISTS) {
                return HRESULT_FROM_WIN32(dwError);
            }

            return E_FAIL;
        }

        g_hInst = hInstance;
        RECT rc = { 0,0,800,600 };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

        g_hWnd = CreateWindow(L"Game Graphics Window Class", L"Game Graphics Programming Lab 01 : Direct3D 11 Basics",
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
            nullptr);

        if (!g_hWnd) {
            DWORD dwError = GetLastError();

            MessageBox(nullptr, L"Create Window", L"FAILED" , NULL);

            if (dwError != ERROR_CLASS_ALREADY_EXISTS) {
                return HRESULT_FROM_WIN32(dwError);
            }

            return E_FAIL;
        }

        ShowWindow(g_hWnd, nCmdShow);

        return S_OK;
    }

    HRESULT InitDevice() {
        HRESULT hr = S_OK;

        RECT rc;
        GetClientRect(g_hWnd, &rc);
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;

        UINT createDeviceFlags = 0;
#ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D_DRIVER_TYPE driverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };
        UINT numDriverTypes = ARRAYSIZE(driverTypes);

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        UINT numFeatureLevels = ARRAYSIZE(featureLevels);

        // 디바이스, context를 생성.

        // &g_pd3dDevice를 쓰는 대신 g_pd3dDevice.GetAddressOf()를 사용한다. &를 쓰면 refernce count를 줄이고 raw pointer를 반환받음.
        // GetAddressOf()는 안전하게 받을 수 있음.

        for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
        {
            g_driverType = driverTypes[driverTypeIndex];
            hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, g_pd3dDevice.GetAddressOf(), &g_featureLevel, g_pImmediateContext.GetAddressOf());

            if (hr == E_INVALIDARG)
            {
                // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                    D3D11_SDK_VERSION, g_pd3dDevice.GetAddressOf(), &g_featureLevel, g_pImmediateContext.GetAddressOf());
            }

            if (SUCCEEDED(hr))
                break;
        }
        if (FAILED(hr))
            return hr;

        // g_pd3dDevice->QueryInterface(__uuidof(IDXGIFactory1), (&dxgiFactory)) 에서
        // g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&dxgiDevice)) 가 g_pd3dDevice.As(&dxgiDevice) 로 바뀐 것.
        // 그리고 ComPtr은 당연히 초기화도 다름. Queryinterface를 쓰면 포인터로 선언해야한다.
        // As는 템플릿 매개변수로 인식되는 '인터페이스'를 반환, Get()은 이 Comptr 인터페이스와 연결된 인터페이스에 대한 포인터를 검색
        // GetAddressOf()는 이 데이터 멤버가 나타내는 인터페이스에 대한 포인터를 포함하는 ptr_ 데이터 멤버의 주소를 검색함.

        // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
        // 만들어둔 디바이스를 이용해 DXGI 팩토리를 받아온다.
        ComPtr<IDXGIFactory1> dxgiFactory(nullptr);
        {
            ComPtr<IDXGIDevice> dxgiDevice(nullptr);
            hr = g_pd3dDevice.As(&dxgiDevice); // 디바이스에서 dxgiDevice로서의 인터페이스 받음
            if (SUCCEEDED(hr))
            {
                ComPtr<IDXGIAdapter> adapter(nullptr);
                hr = dxgiDevice->GetAdapter(adapter.GetAddressOf()); // 디바이스에서 어댑터 넘겨줌.
                if (SUCCEEDED(hr))
                {
                    hr = adapter->GetParent(__uuidof(IDXGIFactory1), &dxgiFactory); // 어댑터에서 팩토리 넘김.
                }
            }
        }
        if (FAILED(hr))
            return hr;
        // 이제 dxgiDevice, dxgiFactory, adapter가 초기화가 되었음.

        // Create swap chain
        // dxgiFactory에서 dxgiFactory2를 받아옴. 전에는 어댑터에서 받았는데, 이제 그럴 필요가 없는 것.
        ComPtr<IDXGIFactory2> dxgiFactory2(nullptr);
        hr = dxgiFactory.As(&dxgiFactory2);
        if (dxgiFactory2)
        {
            // DirectX 11.1 or later
            hr = g_pd3dDevice.As(&g_pd3dDevice1);
            if (SUCCEEDED(hr))
            {
                hr = g_pImmediateContext.As(&g_pImmediateContext1);
            }

            DXGI_SWAP_CHAIN_DESC1 sd = {};
            sd.Width = width;
            sd.Height = height;
            sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.BufferCount = 1;

            hr = dxgiFactory2->CreateSwapChainForHwnd(g_pd3dDevice.Get(), g_hWnd, &sd, nullptr, nullptr, g_pSwapChain1.GetAddressOf());
            if (SUCCEEDED(hr))
            {
                hr = g_pSwapChain1.As(&g_pSwapChain);
            }
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd = {};
            sd.BufferCount = 1;
            sd.BufferDesc.Width = width;
            sd.BufferDesc.Height = height;
            sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.BufferDesc.RefreshRate.Numerator = 60;
            sd.BufferDesc.RefreshRate.Denominator = 1;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.OutputWindow = g_hWnd;
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
            sd.Windowed = TRUE;

            hr = dxgiFactory->CreateSwapChain(g_pd3dDevice.Get(), &sd, g_pSwapChain.GetAddressOf());
        }

        // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
        dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);

        if (FAILED(hr))
            return hr;

        // Create a render target view
        ComPtr<ID3D11Texture2D> pBackBuffer(nullptr);

        hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &pBackBuffer);
        if (FAILED(hr))
            return hr;

        hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, g_pRenderTargetView.GetAddressOf());
        if (FAILED(hr))
            return hr;

        g_pImmediateContext->OMSetRenderTargets(1, g_pRenderTargetView.GetAddressOf(), nullptr);

        // Setup the viewport
        D3D11_VIEWPORT vp;
        vp.Width = (FLOAT)width;
        vp.Height = (FLOAT)height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        g_pImmediateContext->RSSetViewports(1, &vp);
        
        return S_OK;
    }

    void Render() {
        g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView.Get(), DirectX::Colors::MidnightBlue);
        g_pSwapChain->Present(0, 0);
    }

    void CleanupDevice() {
        if (g_pImmediateContext) g_pImmediateContext->ClearState();
    }
}