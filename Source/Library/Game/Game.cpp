#include "Game/Game.h"

namespace library
{
    /*--------------------------------------------------------------------
      Global Variables
    --------------------------------------------------------------------*/
    HINSTANCE g_hInstance = nullptr;
    HWND g_hWnd = nullptr;
    D3D_DRIVER_TYPE g_driverType = D3D_DRIVER_TYPE_NULL;
    D3D_FEATURE_LEVEL g_featureLevel = D3D_FEATURE_LEVEL_11_0;
    ComPtr<ID3D11Device> g_pd3Device;
    ComPtr<ID3D11Device> g_pd3dDevice1;
    ComPtr<ID3D11DeviceContext> g_pImmediateContext;
    ComPtr<IDXGISwapChain> g_pSwapChain;
    /*
    ID3D11Device* g_pd3dDevice = nullptr;
    ID3D11Device1* g_pd3dDevice1 = nullptr;
    ID3D11DeviceContext* g_pImmediateContext = nullptr;
    ID3D11DeviceContext1* g_pImmediateContext1 = nullptr;
    IDXGISwapChain* g_pSwapChain = nullptr;
    IDXGISwapChain1* g_pSwapChain1 = nullptr;
    */
    ID3D11RenderTargetView* g_pRenderTargetView = nullptr;


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

        const wchar_t g_pszWindowClassName[] = L"Sample Window Class";

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
            .lpszClassName = g_pszWindowClassName,
            .hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION),
        };

        if (!RegisterClassEx(&wcex)) {
            DWORD dwError = GetLastError();

            MessageBox(nullptr, L"Failed : ", L"Call RegitserClassEx", NULL);

            if (dwError != ERROR_CLASS_ALREADY_EXISTS) {
                return HRESULT_FROM_WIN32(dwError);
            }

            return E_FAIL;
        }

        g_hInstance = hInstance;
        RECT rc = { 0,0,800,600 };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

        g_hWnd = CreateWindowEx(
            0,
            g_pszWindowClassName,
            L"Game Graphics Programming Lab 01 : Direct3D 11 Basics",
            WS_OVERLAPPEDWINDOW,

            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

            NULL,
            NULL,
            g_hInstance,
            NULL
        );

        if (!g_hWnd) {
            DWORD dwError = GetLastError();

            MessageBox(nullptr, L"Failed :", L"Create Window", NULL);

            if (dwError != ERROR_CLASS_ALREADY_EXISTS) {
                return HRESULT_FROM_WIN32(dwError);
            }

            return E_FAIL;
        }

        ShowWindow(g_hWnd, nCmdShow);

        return S_OK;
    }

    HRESULT InitDevice() {        
        RECT rc;
        GetClientRect(g_hWnd, &rc);
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;

        D3D_FEATURE_LEVEL lvl[] = {
    D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
    D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
    D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1 };

        DWORD createDeviceFlags = 0;
#ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        ComPtr<ID3D11Device> device;
        ComPtr<ID3D11DeviceContext> context;
        D3D_FEATURE_LEVEL fl;
        HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
            createDeviceFlags, lvl, _countof(lvl),
            D3D11_SDK_VERSION, &device, &fl, &context);
        if (hr == E_INVALIDARG)
        {
            hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                createDeviceFlags, &lvl[1], _countof(lvl) - 1,
                D3D11_SDK_VERSION, &device, &fl, &context);
        }

        ComPtr<IDXGIFactory1> dxgiFactory;
        {
            ComPtr<IDXGIDevice> dxgiDevice;
            if (SUCCEEDED(device.As(&dxgiDevice)))
            {
                ComPtr<IDXGIAdapter> adapter;
                if (SUCCEEDED(dxgiDevice->GetAdapter(&adapter)))
                {
                    hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
                    adapter->Release();
                }
                dxgiDevice->Release();
            }
        }
        if (FAILED(hr)) return hr;

        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 2;
        sd.BufferDesc.Width = width;
        sd.BufferDesc.Height = height;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.OutputWindow = g_hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

        hr = dxgiFactory->CreateSwapChain(device.Get(), &sd, &g_pSwapChain);

        dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);

        dxgiFactory->Release();

        if (FAILED(hr))
            return hr;

        ID3D11Texture2D* pBackBuffer = nullptr;
        hr = g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

        if (FAILED(hr)) return hr;

        hr = device->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
        pBackBuffer->Release(); 
        if (FAILED(hr)) {
            return hr;
        }

        g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);

        D3D11_VIEWPORT vp =
        {
            .TopLeftX = 0.0f, .TopLeftY = 0.0f,
            .Width = static_cast<FLOAT>(800), // 이거 수정
            .Height = static_cast<FLOAT>(600),
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f,
        };
        g_pImmediateContext->RSSetViewports(1, &vp);
        
        return S_OK;
    }

    void Render() {
        g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, DirectX::Colors::MidnightBlue);
        g_pSwapChain->Present(0, 0);
    }

    void CleanupDevice() {
        if (g_pImmediateContext) g_pImmediateContext->ClearState();
        if (g_pRenderTargetView) g_pRenderTargetView->Release();
        if (g_pSwapChain) g_pSwapChain->Release();
        if (g_pImmediateContext) g_pImmediateContext->Release();
        if (g_pd3dDevice1) g_pd3dDevice1->Release();
    }
}