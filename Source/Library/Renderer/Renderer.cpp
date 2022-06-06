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
        , m_d3dDevice()
        , m_d3dDevice1()
        , m_immediateContext()
        , m_immediateContext1()
        , m_swapChain()
        , m_swapChain1()
        , m_renderTargetView()
        , m_depthStencil()
        , m_depthStencilView()
        , m_cbChangeOnResize()
        , m_pszMainSceneName(nullptr)
        , m_padding{ '\0' }
        , m_camera(XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f))
        , m_projection()
        , m_scenes()
        , m_invalidTexture(std::make_shared<Texture>(L"Content/Common/InvalidTexture.png"))
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
    HRESULT Renderer::Initialize(_In_ HWND hWnd)
    {
        HRESULT hr = S_OK;

        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT uWidth = static_cast<UINT>(rc.right - rc.left);
        UINT uHeight = static_cast<UINT>(rc.bottom - rc.top);

        UINT uCreateDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
        uCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
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
            hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

            if (hr == E_INVALIDARG)
            {
                // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
            }

            if (SUCCEEDED(hr))
            {
                break;
            }
        }
        if (FAILED(hr))
        {
            return hr;
        }

        // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
        ComPtr<IDXGIFactory1> dxgiFactory;
        {
            ComPtr<IDXGIDevice> dxgiDevice;
            hr = m_d3dDevice.As(&dxgiDevice);
            if (SUCCEEDED(hr))
            {
                ComPtr<IDXGIAdapter> adapter;
                hr = dxgiDevice->GetAdapter(&adapter);
                if (SUCCEEDED(hr))
                {
                    hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
                }
            }
        }
        if (FAILED(hr))
        {
            return hr;
        }

        // Create swap chain
        ComPtr<IDXGIFactory2> dxgiFactory2;
        hr = dxgiFactory.As(&dxgiFactory2);
        if (SUCCEEDED(hr))
        {
            // DirectX 11.1 or later
            hr = m_d3dDevice.As(&m_d3dDevice1);
            if (SUCCEEDED(hr))
            {
                m_immediateContext.As(&m_immediateContext1);
            }

            DXGI_SWAP_CHAIN_DESC1 sd =
            {
                .Width = uWidth,
                .Height = uHeight,
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .SampleDesc = {.Count = 1u, .Quality = 0u },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1u
            };

            hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
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
                .BufferDesc = {.Width = uWidth, .Height = uHeight, .RefreshRate = {.Numerator = 60, .Denominator = 1 }, .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
                .SampleDesc = {.Count = 1, .Quality = 0 },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1u,
                .OutputWindow = hWnd,
                .Windowed = TRUE
            };

            hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
        }

        // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
        dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

        if (FAILED(hr))
        {
            return hr;
        }

        // Create a render target view
        ComPtr<ID3D11Texture2D> pBackBuffer;
        hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Create depth stencil texture
        D3D11_TEXTURE2D_DESC descDepth =
        {
            .Width = uWidth,
            .Height = uHeight,
            .MipLevels = 1u,
            .ArraySize = 1u,
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc = {.Count = 1u, .Quality = 0u },
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
            .CPUAccessFlags = 0u,
            .MiscFlags = 0u
        };
        hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV =
        {
            .Format = descDepth.Format,
            .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
            .Texture2D = {.MipSlice = 0 }
        };
        hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

        // Setup the viewport
        D3D11_VIEWPORT vp =
        {
            .TopLeftX = 0.0f,
            .TopLeftY = 0.0f,
            .Width = static_cast<FLOAT>(uWidth),
            .Height = static_cast<FLOAT>(uHeight),
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f,
        };
        m_immediateContext->RSSetViewports(1, &vp);

        // Set primitive topology
        m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Create the constant buffers
        D3D11_BUFFER_DESC bd =
        {
            .ByteWidth = sizeof(CBChangeOnResize),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0
        };
        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnResize.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Initialize the projection matrix
        m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, static_cast<FLOAT>(uWidth) / static_cast<FLOAT>(uHeight), 0.01f, 1000.0f);

        CBChangeOnResize cbChangesOnResize =
        {
            .Projection = XMMatrixTranspose(m_projection)
        };
        m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0, nullptr, &cbChangesOnResize, 0, 0);

        m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
        m_immediateContext->PSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());

        bd.ByteWidth = sizeof(CBLights);
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = 0u;

        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbLights.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        m_immediateContext->VSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
        m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

        m_camera.Initialize(m_d3dDevice.Get());

        if (!m_scenes.contains(m_pszMainSceneName))
        {
            return E_FAIL;
        }

        hr = m_scenes[m_pszMainSceneName]->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_invalidTexture->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        if (FAILED(hr))
        {
            return hr;
        }

        m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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
    HRESULT Renderer::AddScene(_In_ PCWSTR pszSceneName, _In_ const std::shared_ptr<Scene>& scene)
    {
        if (m_scenes.contains(pszSceneName))
        {
            return E_FAIL;
        }

        m_scenes[pszSceneName] = scene;

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::GetSceneOrNull

      Summary:  Return scene with the given name or null

      Args:     PCWSTR pszSceneName
                  The name of the scene

      Returns:  std::shared_ptr<Scene>
                  The shared pointer to Scene
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    std::shared_ptr<Scene> Renderer::GetSceneOrNull(_In_ PCWSTR pszSceneName)
    {
        if (m_scenes.contains(pszSceneName))
        {
            return m_scenes[pszSceneName];
        }

        return nullptr;
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
    HRESULT Renderer::SetMainScene(_In_ PCWSTR pszSceneName)
    {
        if (!m_scenes.contains(pszSceneName))
        {
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
    void Renderer::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
    {
        m_camera.HandleInput(directions, mouseRelativeMovement, deltaTime);
    }



    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Update

      Summary:  Update the renderables each frame

      Args:     FLOAT deltaTime
                  Time difference of a frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Update(_In_ FLOAT deltaTime)
    {
        m_scenes[m_pszMainSceneName]->Update(deltaTime);

        m_camera.Update(deltaTime);
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

        CBLights cbLights = {};

        std::shared_ptr<library::Scene> mainScene = m_scenes[m_pszMainSceneName];

        for (int i = 0; i < NUM_LIGHTS; i++) {
            if (!mainScene->GetPointLight(i)) continue;
            cbLights.LightPositions[i] = mainScene->GetPointLight(i)->GetPosition();
            cbLights.LightColors[i] = mainScene->GetPointLight(i)->GetColor();
            FLOAT attenuationDistance = mainScene->GetPointLight(i)->GetAttenuationDistance();
            FLOAT attenuationDistanceSquared = attenuationDistance * attenuationDistance;
            cbLights.LightAttenuationDistance[i] = XMFLOAT4(
                attenuationDistance,
                attenuationDistance,
                attenuationDistanceSquared,
                attenuationDistanceSquared
            );
        }

        m_immediateContext->UpdateSubresource(
            m_cbLights.Get(),
            0,
            nullptr,
            &cbLights,
            0,
            0
        );

        /*
        m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
        m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
        m_immediateContext->VSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
        m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
        */


        for (auto i = mainScene->GetRenderables().begin(); i != mainScene->GetRenderables().end(); i++) {
            UINT strides[2] = { sizeof(SimpleVertex), sizeof(NormalData) };
            UINT offsets[2] = { 0u,0u };

            auto& renderable = i->second;

            m_immediateContext->IASetVertexBuffers(
                0,
                1,
                renderable->GetVertexBuffer().GetAddressOf(),
                &strides[0],
                &offsets[0]
            );

            m_immediateContext->IASetVertexBuffers(
                1,
                1,
                renderable->GetNormalBuffer().GetAddressOf(),
                &strides[1],
                &offsets[1]
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
                .OutputColor = renderable->GetOutputColor(),
                .HasNormalMap = renderable->HasNormalMap()
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
            m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2, 1, renderable->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2, 1, renderable->GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

            if (renderable->HasNormalMap()) {
                for (UINT j = 0u; j < renderable->GetNumMeshes(); j++) {
                    m_immediateContext->PSSetShaderResources(
                        1,
                        1,
                        renderable->GetMaterial(renderable->GetMesh(j).uMaterialIndex)->pNormal->GetTextureResourceView().GetAddressOf()
                    );

                    eTextureSamplerType textureSamplerType = renderable->GetMaterial(renderable->GetMesh(j).uMaterialIndex)->pNormal->GetSamplerType();

                    m_immediateContext->PSSetSamplers(
                        1,
                        1,
                        Texture::s_samplers[static_cast<size_t>(textureSamplerType)].GetAddressOf()
                    );
                }
            }

            if (renderable->HasTexture()) {
                for (UINT j = 0u; j < renderable->GetNumMeshes(); j++) {
                    m_immediateContext->PSSetShaderResources(
                        0,
                        1,
                        renderable->GetMaterial(renderable->GetMesh(j).uMaterialIndex)->pDiffuse->GetTextureResourceView().GetAddressOf()
                    );

                    eTextureSamplerType textureSamplerType = renderable->GetMaterial(renderable->GetMesh(j).uMaterialIndex)->pDiffuse->GetSamplerType();

                    m_immediateContext->PSSetSamplers(
                        0,
                        1,
                        Texture::s_samplers[static_cast<size_t>(textureSamplerType)].GetAddressOf()
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

        for (auto& voxel : mainScene->GetVoxels()) {
            UINT strides[3] = { sizeof(SimpleVertex), sizeof(NormalData), sizeof(InstanceData)};
            UINT offsets[3] = { 0u, 0u, 0u };

            ID3D11Buffer* InstBuffers[3] = { 
                voxel->GetVertexBuffer().Get(), 
                voxel->GetNormalBuffer().Get(),
                voxel->GetInstanceBuffer().Get()
            };

            m_immediateContext->IASetVertexBuffers(
                0,
                3,
                InstBuffers,
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
                .OutputColor = voxel->GetOutputColor(),
                .HasNormalMap = voxel->HasNormalMap()
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
            m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2, 1, voxel->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2, 1, voxel->GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

            if (voxel->HasNormalMap()) {
                for (UINT j = 0u; j < voxel->GetNumMeshes(); j++) {
                    m_immediateContext->PSSetShaderResources(
                        1,
                        1,
                        voxel->GetMaterial(voxel->GetMesh(j).uMaterialIndex)->pNormal->GetTextureResourceView().GetAddressOf()
                    );

                    eTextureSamplerType textureSamplerType = voxel->GetMaterial(voxel->GetMesh(j).uMaterialIndex)->pNormal->GetSamplerType();

                    m_immediateContext->PSSetSamplers(
                        1,
                        1,
                        Texture::s_samplers[static_cast<size_t>(textureSamplerType)].GetAddressOf()
                    );
                }
            }

            if (voxel->HasTexture()) {
                for (UINT j = 0u; j < voxel->GetNumMeshes(); j++) {
                    m_immediateContext->PSSetShaderResources(
                        0,
                        1,
                        voxel->GetMaterial(voxel->GetMesh(j).uMaterialIndex)->pDiffuse->GetTextureResourceView().GetAddressOf()
                    );

                    eTextureSamplerType textureSamplerType = voxel->GetMaterial(voxel->GetMesh(j).uMaterialIndex)->pDiffuse->GetSamplerType();

                    m_immediateContext->PSSetSamplers(
                        0,
                        1,
                        Texture::s_samplers[static_cast<size_t>(textureSamplerType)].GetAddressOf()
                    );

                    //m_immediateContext->DrawIndexedInstanced(voxel->GetNumIndices(), voxel->GetNumInstances(), 0, 0, 0);

                    m_immediateContext->DrawIndexedInstanced(
                        voxel->GetMesh(j).uNumIndices,
                        voxel->GetNumInstances(),
                        voxel->GetMesh(j).uBaseIndex,
                        voxel->GetMesh(j).uBaseVertex,
                        0
                    );
                }
            }
            else {
                m_immediateContext->DrawIndexedInstanced(voxel->GetNumIndices(), voxel->GetNumInstances(), 0, 0, 0);
            }
        }

        for (auto& model : mainScene->GetModels()) {
            UINT strides[2] = { sizeof(SimpleVertex), sizeof(NormalData)};
            UINT offsets[2] = { 0u,0u };

            auto& renderable = model.second;

            ID3D11Buffer* aBuffers[2]
            {
                renderable->GetVertexBuffer().Get(),
                renderable->GetNormalBuffer().Get()
            };

            // animationData는 없어서 굳이 안 넣음.

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
                .OutputColor = renderable->GetOutputColor(),
                .HasNormalMap = renderable->HasNormalMap()
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
                cbSkin.BoneTransforms[i] = XMMatrixTranspose(renderable->GetBoneTransforms()[i]);
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

            m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());

            m_immediateContext->VSSetConstantBuffers(2, 1, renderable->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2, 1, renderable->GetConstantBuffer().GetAddressOf());


            m_immediateContext->VSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

            m_immediateContext->VSSetConstantBuffers(4, 1, renderable->GetSkinningConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(4, 1, renderable->GetSkinningConstantBuffer().GetAddressOf());

            if (renderable->HasNormalMap()) {
                for (UINT j = 0u; j < renderable->GetNumMeshes(); j++) {
                    m_immediateContext->PSSetShaderResources(
                        1,
                        1,
                        renderable->GetMaterial(renderable->GetMesh(j).uMaterialIndex)->pNormal->GetTextureResourceView().GetAddressOf()
                    );

                    eTextureSamplerType textureSamplerType = renderable->GetMaterial(renderable->GetMesh(j).uMaterialIndex)->pNormal->GetSamplerType();

                    m_immediateContext->PSSetSamplers(
                        1,
                        1,
                        Texture::s_samplers[static_cast<size_t>(textureSamplerType)].GetAddressOf()
                    );
                }
            }

            if (renderable->HasTexture()) {
                for (UINT j = 0u; j < renderable->GetNumMeshes(); j++) {
                    m_immediateContext->PSSetShaderResources(
                        0,
                        1,
                        renderable->GetMaterial(renderable->GetMesh(j).uMaterialIndex)->pDiffuse->GetTextureResourceView().GetAddressOf()
                    );

                    eTextureSamplerType textureSamplerType = renderable->GetMaterial(renderable->GetMesh(j).uMaterialIndex)->pDiffuse->GetSamplerType();

                    m_immediateContext->PSSetSamplers(
                        0,
                        1,
                        Texture::s_samplers[static_cast<size_t>(textureSamplerType)].GetAddressOf()
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

        if (mainScene->GetSkyBox()) {
            std::shared_ptr<Skybox> skybox = mainScene->GetSkyBox();

            UINT stride = sizeof(XMFLOAT3);
            UINT offset = 0u;

            m_immediateContext->IASetVertexBuffers(
                0,
                1,
                skybox->GetVertexBuffer().GetAddressOf(),
                &stride,
                &offset
            );

            m_immediateContext->IASetIndexBuffer(
                skybox->GetIndexBuffer().Get(),
                DXGI_FORMAT_R16_UINT,
                0
            );

            m_immediateContext->IASetInputLayout(
                skybox->GetVertexLayout().Get()
            );

            CBChangesEveryFrame cbRenderable = {
                .World = XMMatrixTranspose(skybox->GetWorldMatrix() * XMMatrixTranslationFromVector(m_camera.GetEye())),
                .OutputColor = skybox->GetOutputColor(),
                .HasNormalMap = skybox->HasNormalMap()
            };

            m_immediateContext->UpdateSubresource(
                skybox->GetConstantBuffer().Get(),
                0,
                nullptr,
                &cbRenderable,
                0,
                0
            );

            m_immediateContext->VSSetShader(skybox->GetVertexShader().Get(), nullptr, 0);
            m_immediateContext->PSSetShader(skybox->GetPixelShader().Get(), nullptr, 0);

            m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());

            m_immediateContext->VSSetConstantBuffers(2, 1, skybox->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2, 1, skybox->GetConstantBuffer().GetAddressOf());

            m_immediateContext->VSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

            if (skybox->HasNormalMap()) {
                for (UINT j = 0u; j < skybox->GetNumMeshes(); j++) {
                    m_immediateContext->PSSetShaderResources(
                        1,
                        1,
                        skybox->GetMaterial(skybox->GetMesh(j).uMaterialIndex)->pNormal->GetTextureResourceView().GetAddressOf()
                    );

                    eTextureSamplerType textureSamplerType = skybox->GetMaterial(skybox->GetMesh(j).uMaterialIndex)->pNormal->GetSamplerType();

                    m_immediateContext->PSSetSamplers(
                        1,
                        1,
                        Texture::s_samplers[static_cast<size_t>(textureSamplerType)].GetAddressOf()
                    );
                }
            }

            if (skybox->HasTexture()) {
                for (UINT j = 0u; j < skybox->GetNumMeshes(); j++) {
                    m_immediateContext->PSSetShaderResources(
                        0,
                        1,
                        skybox->GetMaterial(skybox->GetMesh(j).uMaterialIndex)->pDiffuse->GetTextureResourceView().GetAddressOf()
                    );

                    eTextureSamplerType textureSamplerType = skybox->GetMaterial(skybox->GetMesh(j).uMaterialIndex)->pDiffuse->GetSamplerType();

                    m_immediateContext->PSSetSamplers(
                        0,
                        1,
                        Texture::s_samplers[static_cast<size_t>(textureSamplerType)].GetAddressOf()
                    );

                    m_immediateContext->DrawIndexed(
                        skybox->GetMesh(j).uNumIndices,
                        skybox->GetMesh(j).uBaseIndex,
                        skybox->GetMesh(j).uBaseVertex
                    );
                }
            }
            else {
                m_immediateContext->DrawIndexed(skybox->GetNumIndices(), 0, 0);
            }
        }

        m_swapChain->Present(0, 0);

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
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