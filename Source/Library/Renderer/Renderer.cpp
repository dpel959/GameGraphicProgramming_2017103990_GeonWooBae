#include "Renderer/Renderer.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Renderer

      Summary:  Constructor

      Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                 m_immediateContext, m_immediateContext1, m_swapChain,
                 m_swapChain1, m_renderTargetView, m_depthStencil,
                 m_depthStencilView, m_cbChangeOnResize, m_camera,
                 m_projection, m_renderables, m_vertexShaders,
                 m_pixelShaders].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    Renderer::Renderer()
        : m_driverType(D3D_DRIVER_TYPE_NULL)
        , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
        , m_d3dDevice(nullptr)
        , m_d3dDevice1(nullptr)
        , m_immediateContext(nullptr)
        , m_immediateContext1(nullptr)
        , m_swapChain(nullptr)
        , m_swapChain1(nullptr)
        , m_renderTargetView(nullptr)
        , m_depthStencil(nullptr)
        , m_depthStencilView(nullptr)
        , m_cbChangeOnResize(nullptr)
        , m_cbLights(nullptr)
        , m_camera(Camera(XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f)))
        , m_projection()
        , m_pszMainSceneName()
        , m_renderables()
        , m_models()
        , m_aPointLights()
        , m_vertexShaders()
        , m_pixelShaders()
        , m_scenes()
    {
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Initialize

      Summary:  Creates Direct3D device and swap chain

      Args:     HWND hWnd
                  Handle to the window

      Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
                 m_d3dDevice1, m_immediateContext1, m_swapChain1,
                 m_swapChain, m_renderTargetView, m_cbChangeOnResize,
                 m_projection, m_camera, m_vertexShaders,
                 m_pixelShaders, m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::Initialize(_In_ HWND hWnd) {
        HWND g_hWnd = hWnd;
        HRESULT hr = S_OK;

        RECT rc;
        POINT p1, p2;

        GetClientRect(g_hWnd, &rc);

        p1 = {
            .x = rc.left,
            .y = rc.top
        };

        p2 = {
            .x = rc.right,
            .y = rc.bottom
        };

        ClientToScreen(g_hWnd, &p1);
        ClientToScreen(g_hWnd, &p2);

        rc = {
            .left = p1.x,
            .top = p1.y,
            .right = p2.x,
            .bottom = p2.y
        };

        ClipCursor(&rc);

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

        for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
        {
            m_driverType = driverTypes[driverTypeIndex];
            hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

            if (hr == E_INVALIDARG)
            {
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
            }

            if (SUCCEEDED(hr))
                break;
        }
        if (FAILED(hr))
            return hr;

        ComPtr<IDXGIFactory1> dxgiFactory(nullptr);
        {
            ComPtr<IDXGIDevice> dxgiDevice(nullptr);
            hr = m_d3dDevice.As(&dxgiDevice);
            if (SUCCEEDED(hr))
            {
                ComPtr<IDXGIAdapter> adapter(nullptr);
                hr = dxgiDevice->GetAdapter(adapter.GetAddressOf());
                if (SUCCEEDED(hr))
                {
                    hr = adapter->GetParent(__uuidof(IDXGIFactory1), &dxgiFactory);
                }
            }
        }
        if (FAILED(hr))
            return hr;

        ComPtr<IDXGIFactory2> dxgiFactory2(nullptr);
        hr = dxgiFactory.As(&dxgiFactory2);
        if (dxgiFactory2)
        {
            // DirectX 11.1 or later
            hr = m_d3dDevice.As(&m_d3dDevice1);
            if (SUCCEEDED(hr))
            {
                hr = m_immediateContext.As(&m_immediateContext1);
            }

            DXGI_SWAP_CHAIN_DESC1 sd =
            {
                .Width = width,
                .Height = height,
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .SampleDesc =
                {
                    .Count = 1,
                    .Quality = 0
                },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1,
            };

            hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), g_hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
            if (SUCCEEDED(hr))
            {
                hr = m_swapChain1.As(&m_swapChain);
            }
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd =
            {
                .BufferDesc =
                {
                    .Width = width,
                    .Height = height,
                    .RefreshRate =
                    {
                        .Numerator = 60,
                        .Denominator = 1
                    },
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM
                },
                .SampleDesc =
                {
                    .Count = 1,
                    .Quality = 0
                },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1,
                .OutputWindow = g_hWnd,
                .Windowed = TRUE
            };

            hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
        }

        dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);

        if (FAILED(hr))
            return hr;

        ComPtr<ID3D11Texture2D> pBackBuffer(nullptr);

        hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &pBackBuffer);
        if (FAILED(hr))
            return hr;

        hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
        if (FAILED(hr))
            return hr;

        D3D11_TEXTURE2D_DESC descDepth =
        {
            .Width = width,
            .Height = height,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc =
            {
                .Count = 1,
                .Quality = 0
            },
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
            .CPUAccessFlags = 0,
            .MiscFlags = 0
        };

        m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
        if (FAILED(hr)) return hr;

        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV =
        {
            .Format = descDepth.Format,
            .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
            .Texture2D =
            {
                .MipSlice = 0
            },
        };

        m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());

        if (FAILED(hr)) return hr;

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

        D3D11_VIEWPORT vp =
        {
            .TopLeftX = 0,
            .TopLeftY = 0,
            .Width = (FLOAT)width,
            .Height = (FLOAT)height,
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f
        };

        m_immediateContext->RSSetViewports(1, &vp);

        // create cbChangeOnResize
        m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, (FLOAT)descDepth.Width / (FLOAT)descDepth.Height, 0.01f, 100.0f);

        D3D11_BUFFER_DESC bd =
        {
            .ByteWidth = sizeof(CBChangeOnResize),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0,
        };

        CBChangeOnResize cbChangeOnResize;
        cbChangeOnResize.Projection = XMMatrixTranspose(m_projection);

        D3D11_SUBRESOURCE_DATA cbResizeInit =
        {
            .pSysMem = &cbChangeOnResize
        };

        hr = m_d3dDevice->CreateBuffer(&bd, &cbResizeInit, m_cbChangeOnResize.GetAddressOf());
        if (FAILED(hr)) return hr;

        m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());

        bd = {
            .ByteWidth = sizeof(CBLights),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0
        };

        CBLights cbLights = {};
       
        D3D11_SUBRESOURCE_DATA cbLightsInit = {
            .pSysMem = &cbLights
        };

        hr = m_d3dDevice->CreateBuffer(&bd, &cbLightsInit, m_cbLights.GetAddressOf());
        if (FAILED(hr)) return hr;

        m_immediateContext->VSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
        m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

        for (auto vs = m_vertexShaders.begin(); vs != m_vertexShaders.end(); vs++) {
            hr = vs->second->Initialize(m_d3dDevice.Get());
            if (FAILED(hr)) return hr;
        }

        for (auto ps = m_pixelShaders.begin(); ps != m_pixelShaders.end(); ps++) {
            hr = ps->second->Initialize(m_d3dDevice.Get());
            if (FAILED(hr)) return hr;
        }

        for (auto i = m_renderables.begin(); i != m_renderables.end(); i++) {
            hr = i->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
            if (FAILED(hr)) return hr;
        }

        // 추후 과제에 따라 main Scene만 돌려야할수도 있음.
        for (auto i = m_scenes.begin(); i != m_scenes.end(); i++) {
            hr = i->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
            if (FAILED(hr)) return hr;
        }

        for (auto i = m_models.begin(); i != m_models.end(); i++) {
            hr = i->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
            if (FAILED(hr)) return hr;
        }

        // create cbChangesEveryFrame
        hr = m_camera.Initialize(m_d3dDevice.Get());
        if (FAILED(hr)) return hr;

        m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        return S_OK;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddRenderable

      Summary:  Add a renderable object and initialize the object

      Args:     PCWSTR pszRenderableName
                  Key of the renderable object
                const std::shared_ptr<Renderable>& renderable
                  Unique pointer to the renderable object

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code.
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddRenderable(
        _In_ PCWSTR pszRenderableName,
        _In_ const std::shared_ptr<Renderable>& renderable
    )
    {
        for (auto i = m_renderables.begin(); i != m_renderables.end(); i++) {
            if (wcscmp(i->first, pszRenderableName) == 0) {
                return E_FAIL;
            }
        }
        m_renderables.insert(std::make_pair(pszRenderableName, renderable));

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddPointLight

      Summary:  Add a point light

      Args:     size_t index
                  Index of the point light
                const std::shared_ptr<PointLight>& pointLight
                  Shared pointer to the point light object

      Modifies: [m_aPointLights].

      Returns:  HRESULT
                  Status code.
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddPointLight(_In_ size_t index, _In_ const std::shared_ptr<PointLight>& pPointLight) {
        if (index < NUM_LIGHTS && pPointLight) {
            m_aPointLights[index] = pPointLight;
        }
        else {
            return E_FAIL;
        }

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddVertexShader

      Summary:  Add the vertex shader into the renderer

      Args:     PCWSTR pszVertexShaderName
                  Key of the vertex shader
                const std::shared_ptr<VertexShader>&
                  Vertex shader to add

      Modifies: [m_vertexShaders].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddVertexShader(
        _In_ PCWSTR pszVertexShaderName,
        _In_ const std::shared_ptr<VertexShader>& vertexShader
    )
    {
        for (auto i = m_vertexShaders.begin(); i != m_vertexShaders.end(); i++) {
            if (wcscmp(i->first, pszVertexShaderName) == 0) {
                return E_FAIL;
            }
        }
        m_vertexShaders.insert(std::make_pair(pszVertexShaderName, vertexShader));

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddPixelShader

      Summary:  Add the pixel shader into the renderer

      Args:     PCWSTR pszPixelShaderName
                  Key of the pixel shader
                const std::shared_ptr<PixelShader>&
                  Pixel shader to add

      Modifies: [m_pixelShaders].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddPixelShader(
        _In_ PCWSTR pszPixelShaderName,
        _In_ const std::shared_ptr<PixelShader>& pixelShader
    )
    {
        for (auto i = m_pixelShaders.begin(); i != m_pixelShaders.end(); i++) {
            if (wcscmp(i->first, pszPixelShaderName) == 0) {
                return E_FAIL;
            }
        }
        m_pixelShaders.insert(std::make_pair(pszPixelShaderName, pixelShader));

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddScene

      Summary:  Add a scene

      Args:     PCWSTR pszSceneName
                  Key of a scene
                const std::filesystem::path& sceneFilePath
                  File path to initialize a scene

      Modifies: [m_scenes].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddScene(_In_ PCWSTR pszSceneName, const std::filesystem::path& sceneFilePath) {
        bool isSceneFound = false;
        for (auto& scene : m_scenes) {
            if (wcscmp(scene.first.c_str(), pszSceneName) == 0) {
                isSceneFound = true;
                break;
            }
        }

        if (isSceneFound) {
            return E_FAIL;
        }

        m_scenes.insert(std::make_pair(pszSceneName, std::make_shared<Scene>(sceneFilePath)));
        
        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
    Method:   Renderer::AddModel

    Summary:  Add a model object

    Args:     PCWSTR pszModelName
                Key of the model object
              const std::shared_ptr<Model>& pModel
                Shared pointer to the model object

    Modifies: [m_models].

    Returns:  HRESULT
                Status code.
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddModel(_In_ PCWSTR pszModelName, _In_ const std::shared_ptr<Model>& pModel) {
        bool isModelFound = false;
        for (auto& model : m_models) {
            if (wcscmp(model.first, pszModelName) == 0) {
                isModelFound = true;
                break;
            }
        }

        if (isModelFound) {
            return E_FAIL;
        }

        m_models.insert(std::make_pair(pszModelName, pModel));

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetMainScene

      Summary:  Set the main scene

      Args:     PCWSTR pszSceneName
                  Name of the scene to set as the main scene

      Modifies: [m_pszMainSceneName].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetMainScene(_In_ PCWSTR pszSceneName) {
        if (!m_scenes.contains(pszSceneName)) {
            return E_FAIL;
        }
        m_pszMainSceneName = pszSceneName;
        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::HandleInput

      Summary:  Add the pixel shader into the renderer and initialize it

      Args:     const DirectionsInput& directions
                  Data structure containing keyboard input data
                const MouseRelativeMovement& mouseRelativeMovement
                  Data structure containing mouse relative input data

      Modifies: [m_camera].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::HandleInput(
        _In_ const DirectionsInput& directions,
        _In_ const MouseRelativeMovement& mouseRelativeMovement,
        _In_ FLOAT deltaTime
        )
    {
        m_camera.HandleInput(
            directions,
            mouseRelativeMovement,
            deltaTime
        );
    }



    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Update

      Summary:  Update the renderables each frame

      Args:     FLOAT deltaTime
                  Time difference of a frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Update(_In_ FLOAT deltaTime) {
        for (auto i = m_renderables.begin(); i != m_renderables.end(); i++) {
            i->second->Update(deltaTime);
        }

        for (auto& voxel : m_scenes[m_pszMainSceneName]->GetVoxels()) {
            voxel->Update(deltaTime);
        }

        for (auto& model : m_models) {
            model.second->Update(deltaTime);
        }

        for (auto& light : m_aPointLights)
        {
            light->Update(deltaTime);
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Render

      Summary:  Render the frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Render() {
        m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), DirectX::Colors::MidnightBlue);
        m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        XMFLOAT4 camPos;
        XMStoreFloat4(&camPos, m_camera.GetEye());
        CBChangeOnCameraMovement cbView = {
            .View = XMMatrixTranspose(m_camera.GetView()),
            .CameraPosition = camPos
        };

        m_immediateContext->UpdateSubresource(
            m_camera.GetConstantBuffer().Get(),
            0,
            nullptr,
            &cbView,
            0,
            0
        );

        m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
        m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());

        CBLights cbLights = {};

        for (int i = 0; i < NUM_LIGHTS; i++) {
            if (!m_aPointLights[i]) continue;
            cbLights.LightPositions[i] = m_aPointLights[i]->GetPosition();
            cbLights.LightColors[i] = m_aPointLights[i]->GetColor();
        }

        m_immediateContext->UpdateSubresource(
            m_cbLights.Get(),
            0,
            nullptr,
            &cbLights,
            0,
            0
        );

        m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

        for (auto i = m_renderables.begin(); i != m_renderables.end(); i++) {
            UINT stride = sizeof(SimpleVertex);
            UINT offset = 0;

            auto& renderable = i->second;

            m_immediateContext->IASetVertexBuffers(
                0,
                1,
                renderable->GetVertexBuffer().GetAddressOf(),
                &stride,
                &offset
            );

            m_immediateContext->IASetIndexBuffer(
                renderable->GetIndexBuffer().Get(),
                DXGI_FORMAT_R16_UINT,
                0
            );

            m_immediateContext->IASetInputLayout(
                renderable->GetVertexLayout().Get()
            );

            CBChangesEveryFrame cbRenderable = {
                .World = XMMatrixTranspose(renderable->GetWorldMatrix()),
                .OutputColor = renderable->GetOutputColor()
            };

            m_immediateContext->UpdateSubresource(
                renderable->GetConstantBuffer().Get(),
                0,
                nullptr,
                &cbRenderable,
                0,
                0
            );

            m_immediateContext->VSSetShader(renderable->GetVertexShader().Get(), nullptr, 0);
            m_immediateContext->PSSetShader(renderable->GetPixelShader().Get(), nullptr, 0);

            m_immediateContext->VSSetConstantBuffers(2, 1, renderable->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2, 1, renderable->GetConstantBuffer().GetAddressOf());

            if (renderable->HasTexture()) {
                for (UINT j = 0u; j < renderable->GetNumMeshes(); j++) {
                    m_immediateContext->PSSetShaderResources(
                        0,
                        1,
                        renderable->GetMaterial(renderable->GetMesh(j).uMaterialIndex).pDiffuse->GetTextureResourceView().GetAddressOf()
                    );
                    m_immediateContext->PSSetSamplers(
                        0,
                        1,
                        renderable->GetMaterial(renderable->GetMesh(j).uMaterialIndex).pDiffuse->GetSamplerState().GetAddressOf()
                    );

                    m_immediateContext->DrawIndexed(
                        renderable->GetMesh(j).uNumIndices,
                        renderable->GetMesh(j).uBaseIndex,
                        renderable->GetMesh(j).uBaseVertex
                    );
                }
            }
            else {
                m_immediateContext->DrawIndexed(renderable->GetNumIndices(), 0, 0);
            }
        }

        for (auto& voxel : m_scenes[m_pszMainSceneName]->GetVoxels()) {
            UINT strides[2] = { sizeof(SimpleVertex), sizeof(InstanceData) };
            UINT offsets[2] = { 0,0 };

            ID3D11Buffer* vertInstBuffers[2] = { voxel->GetVertexBuffer().Get(), voxel->GetInstanceBuffer().Get() };

            m_immediateContext->IASetVertexBuffers(
                0,
                2,
                vertInstBuffers,
                strides,
                offsets
            );

            m_immediateContext->IASetIndexBuffer(
                voxel->GetIndexBuffer().Get(),
                DXGI_FORMAT_R16_UINT,
                0
            );

            m_immediateContext->IASetInputLayout(
                voxel->GetVertexLayout().Get()
            );

            CBChangesEveryFrame cbRenderable = {
                .World = XMMatrixTranspose(voxel->GetWorldMatrix()),
                .OutputColor = voxel->GetOutputColor()
            };

            m_immediateContext->UpdateSubresource(
                voxel->GetConstantBuffer().Get(),
                0,
                nullptr,
                &cbRenderable,
                0,
                0
            );

            m_immediateContext->VSSetShader(voxel->GetVertexShader().Get(), nullptr, 0);
            m_immediateContext->PSSetShader(voxel->GetPixelShader().Get(), nullptr, 0);

            m_immediateContext->VSSetConstantBuffers(2, 1, voxel->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2, 1, voxel->GetConstantBuffer().GetAddressOf());

            // 추후, texture가 들어가면 고려
            //if (voxel->HasTexture()) {
            //    for (UINT j = 0u; j < voxel->GetNumMeshes(); j++) {
            //        m_immediateContext->PSSetShaderResources(
            //            0,
            //            1,
            //            voxel->GetMaterial(voxel->GetMesh(j).uMaterialIndex).pDiffuse->GetTextureResourceView().GetAddressOf()
            //        );
            //        m_immediateContext->PSSetSamplers(
            //            0,
            //            1,
            //            voxel->GetMaterial(voxel->GetMesh(j).uMaterialIndex).pDiffuse->GetSamplerState().GetAddressOf()
            //        );

            //        m_immediateContext->DrawIndexedInstanced(
            //            voxel->GetMesh(j).uNumIndices,
            //            1,
            //            voxel->GetMesh(j).uBaseIndex,
            //            voxel->GetMesh(j).uBaseVertex,
            //            0
            //        );
            //    }
            //}
            //else {
                m_immediateContext->DrawIndexedInstanced(voxel->GetNumIndices(), voxel->GetNumInstances(), 0, 0, 0);
            //}
        }

        for (auto& model : m_models) {
            UINT strides[2] = { static_cast<UINT>(sizeof(SimpleVertex)), static_cast<UINT>(sizeof(AnimationData))};
            UINT offsets[2] = { 0u,0u };

            auto& renderable = model.second;

            ID3D11Buffer* aBuffers[2]
            {
                renderable->GetVertexBuffer().Get(),
                renderable->GetAnimationBuffer().Get()
            };

            m_immediateContext->IASetVertexBuffers(
                0,
                2,
                aBuffers,
                strides,
                offsets
            );

            m_immediateContext->IASetIndexBuffer(
                renderable->GetIndexBuffer().Get(),
                DXGI_FORMAT_R16_UINT,
                0
            );

            m_immediateContext->IASetInputLayout(
                renderable->GetVertexLayout().Get()
            );

            CBChangesEveryFrame cbRenderable = {
                .World = XMMatrixTranspose(renderable->GetWorldMatrix()),
                .OutputColor = renderable->GetOutputColor()
            };

            m_immediateContext->UpdateSubresource(
                renderable->GetConstantBuffer().Get(),
                0,
                nullptr,
                &cbRenderable,
                0,
                0
            );

            CBSkinning cbSkin;

            for (UINT i = 0u; i < renderable->GetBoneTransforms().size(); i++) {
                cbSkin.BoneTransforms[i] = renderable->GetBoneTransforms()[i];
            }

            m_immediateContext->UpdateSubresource(
                renderable->GetSkinningConstantBuffer().Get(),
                0,
                nullptr,
                &cbSkin,
                0,
                0
            );

            m_immediateContext->VSSetShader(renderable->GetVertexShader().Get(), nullptr, 0);
            m_immediateContext->PSSetShader(renderable->GetPixelShader().Get(), nullptr, 0);

            m_immediateContext->VSSetConstantBuffers(2, 1, renderable->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2, 1, renderable->GetConstantBuffer().GetAddressOf());

            m_immediateContext->VSSetConstantBuffers(4, 1, renderable->GetSkinningConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(4, 1, renderable->GetSkinningConstantBuffer().GetAddressOf());

            if (renderable->HasTexture()) {
                for (UINT j = 0u; j < renderable->GetNumMeshes(); j++) {
                    m_immediateContext->PSSetShaderResources(
                        0,
                        1,
                        renderable->GetMaterial(renderable->GetMesh(j).uMaterialIndex).pDiffuse->GetTextureResourceView().GetAddressOf()
                    );
                    m_immediateContext->PSSetSamplers(
                        0,
                        1,
                        renderable->GetMaterial(renderable->GetMesh(j).uMaterialIndex).pDiffuse->GetSamplerState().GetAddressOf()
                    );

                    m_immediateContext->DrawIndexed(
                        renderable->GetMesh(j).uNumIndices,
                        renderable->GetMesh(j).uBaseIndex,
                        renderable->GetMesh(j).uBaseVertex
                    );
                }
            }
            else {
                m_immediateContext->DrawIndexed(renderable->GetNumIndices(), 0, 0);
            }
        }

        m_swapChain->Present(0, 0);

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetVertexShaderOfRenderable

      Summary:  Sets the vertex shader for a renderable

      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszVertexShaderName
                  Key of the vertex shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetVertexShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszVertexShaderName) {
        if (!m_vertexShaders[pszVertexShaderName] || !m_renderables[pszRenderableName]) {
            return E_FAIL;
        }
        m_renderables[pszRenderableName]->SetVertexShader(m_vertexShaders[pszVertexShaderName]);
        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetPixelShaderOfRenderable

      Summary:  Sets the pixel shader for a renderable

      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszPixelShaderName
                  Key of the pixel shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetPixelShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszPixelShaderName) {
        if (!m_pixelShaders[pszPixelShaderName] || !m_renderables[pszRenderableName]) {
            return E_FAIL;
        }
        m_renderables[pszRenderableName]->SetPixelShader(m_pixelShaders[pszPixelShaderName]);
        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetVertexShaderOfScene

      Summary:  Sets the vertex shader for the voxels in a scene

      Args:     PCWSTR pszSceneName
                  Key of the scene
                PCWSTR pszVertexShaderName
                  Key of the vertex shader

      Modifies: [m_scenes].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetVertexShaderOfScene(_In_ PCWSTR pszSceneName, _In_ PCWSTR pszVertexShaderName) {
        if (!m_vertexShaders[pszVertexShaderName])
        {
            return E_FAIL;
        }

        bool isSceneFound = false;
        for(auto& scene : m_scenes) {
            if (wcscmp(scene.first.c_str(), pszSceneName) == 0) {
                isSceneFound = true;
                break;
            }
        }

        if (!isSceneFound) {
            return E_FAIL;
        }

        for (auto& voxel : m_scenes[pszSceneName]->GetVoxels()) {
            voxel->SetVertexShader(m_vertexShaders[pszVertexShaderName]);
        }
        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetPixelShaderOfScene

      Summary:  Sets the pixel shader for the voxels in a scene

      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszPixelShaderName
                  Key of the pixel shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetPixelShaderOfScene(_In_ PCWSTR pszSceneName, _In_ PCWSTR pszPixelShaderName) {
        if (!m_pixelShaders[pszPixelShaderName]) {
            return E_FAIL;
        }

        bool isSceneFound = false;
        for (auto& scene : m_scenes) {
            if (wcscmp(scene.first.c_str(), pszSceneName) == 0) {
                isSceneFound = true;
                break;
            }
        }

        if (!isSceneFound) {
            return E_FAIL;
        }

        for (auto& voxel : m_scenes[pszSceneName]->GetVoxels()) {
            voxel->SetPixelShader(m_pixelShaders[pszPixelShaderName]);
        }
        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetVertexShaderOfModel

      Summary:  Sets the pixel shader for a model

      Args:     PCWSTR pszModelName
                  Key of the model
                PCWSTR pszVertexShaderName
                  Key of the vertex shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetVertexShaderOfModel(_In_ PCWSTR pszModelName, _In_ PCWSTR pszVertexShaderName) {
        if (!m_vertexShaders[pszVertexShaderName])
        {
            return E_FAIL;
        }

        bool isModelFound = false;
        for (auto& model : m_models) {
            if (wcscmp(model.first, pszModelName) == 0) {
                isModelFound = true;
                model.second->SetVertexShader(m_vertexShaders[pszVertexShaderName]);
                break;
            }
        }

        if (!isModelFound) {
            return E_FAIL;
        }
        
        return S_OK;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetPixelShaderOfModel

      Summary:  Sets the pixel shader for a model

      Args:     PCWSTR pszModelName
                  Key of the model
                PCWSTR pszPixelShaderName
                  Key of the pixel shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetPixelShaderOfModel(_In_ PCWSTR pszModelName, _In_ PCWSTR pszPixelShaderName) {
        if (!m_pixelShaders[pszPixelShaderName])
        {
            return E_FAIL;
        }

        bool isModelFound = false;
        for (auto& model : m_models) {
            if (wcscmp(model.first, pszModelName) == 0) {
                isModelFound = true;
                model.second->SetPixelShader(m_pixelShaders[pszPixelShaderName]);
                break;
            }
        }

        if (!isModelFound) {
            return E_FAIL;
        }

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::GetDriverType

      Summary:  Returns the Direct3D driver type

      Returns:  D3D_DRIVER_TYPE
                  The Direct3D driver type used
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    D3D_DRIVER_TYPE Renderer::GetDriverType() const {
        return m_driverType;
    }
}