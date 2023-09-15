#include "pch.h"
#include "MasterRenderer.h"

#include "Graphics/Objects/3D/Meshes/FaceMesh.h"
#include "Graphics/Objects/3D/Elements/Face.h"
#include "Graphics/Utilities/BasicLoader.h"

using namespace DirectX;
using namespace winrt::Windows::Foundation;
using namespace std::literals::chrono_literals;

MasterRenderer::MasterRenderer(std::shared_ptr<DX::DeviceResources> const& deviceResources, std::shared_ptr<ModeScreen> const& modeScreen) :
    m_deviceResources(deviceResources),
    m_mode(modeScreen),
    m_initialized(false),
    m_gameResourcesLoaded(false),
    m_levelResourcesLoaded(false),
    m_uiElementRenderer(deviceResources)
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}

void MasterRenderer::CreateDeviceDependentResources()
{
    m_gameResourcesLoaded = false;
    m_levelResourcesLoaded = false;

    //m_2DRenderer.CreateDeviceDependentResources();
}

winrt::Windows::Foundation::Size MasterRenderer::getCurrentScreenSize()
{
    return m_deviceResources->GetLogicalSize();
}

void MasterRenderer::CreateWindowSizeDependentResources()
{
    //m_2DRenderer.CreateWindowSizeDependentResources(m_mode->getCurrentModeMenuObjects());

    auto d3dContext = m_deviceResources->GetD3DDeviceContext();
    auto renderTargetSize = m_deviceResources->GetRenderTargetSize();
}

IAsyncAction MasterRenderer::CreateModeResourcesAsync()
{
    auto d3dDevice = m_deviceResources->GetD3DDevice();

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    // Create the constant buffers.
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.ByteWidth = (sizeof(ConstantBufferNeverChanges) + 15) / 16 * 16;
    m_constantBufferNeverChanges = nullptr;
    winrt::check_hresult(
        d3dDevice->CreateBuffer(&bd, nullptr, m_constantBufferNeverChanges.put())
    );

    bd.ByteWidth = (sizeof(ConstantBufferChangeOnResize) + 15) / 16 * 16;
    m_constantBufferChangeOnResize = nullptr;
    winrt::check_hresult(
        d3dDevice->CreateBuffer(&bd, nullptr, m_constantBufferChangeOnResize.put())
    );

    bd.ByteWidth = (sizeof(ConstantBufferChangesEveryFrame) + 15) / 16 * 16;
    m_constantBufferChangesEveryFrame = nullptr;
    winrt::check_hresult(
        d3dDevice->CreateBuffer(&bd, nullptr, m_constantBufferChangesEveryFrame.put())
    );

    bd.ByteWidth = (sizeof(ConstantBufferChangesEveryPrim) + 15) / 16 * 16;
    m_constantBufferChangesEveryPrim = nullptr;
    winrt::check_hresult(
        d3dDevice->CreateBuffer(&bd, nullptr, m_constantBufferChangesEveryPrim.put())
    );

    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));

    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = FLT_MAX;
    m_samplerLinear = nullptr;
    winrt::check_hresult(
        d3dDevice->CreateSamplerState(&sampDesc, m_samplerLinear.put())
    );

    // Start the async tasks to load the shaders and textures.
    BasicLoader loader{ d3dDevice };

    std::vector<IAsyncAction> tasks;

    uint32_t numElements = ARRAYSIZE(PNTVertexLayout);
    tasks.push_back(loader.LoadShaderAsync(L"VertexShader.cso", PNTVertexLayout, numElements, m_vertexShader.put(), m_vertexLayout.put()));
    tasks.push_back(loader.LoadShaderAsync(L"VertexShaderFlat.cso", nullptr, numElements, m_vertexShaderFlat.put(), nullptr));
    tasks.push_back(loader.LoadShaderAsync(L"PixelShader.cso", m_pixelShader.put()));
    tasks.push_back(loader.LoadShaderAsync(L"PixelShaderFlat.cso", m_pixelShaderFlat.put()));

    // Make sure the previous versions if any of the textures are released.
    m_sphereTexture = nullptr;
    m_cylinderTexture = nullptr;
    m_ceilingTexture = nullptr;
    m_floorTexture = nullptr;
    m_wallsTexture = nullptr;

    // Load Game specific textures.
    /*tasks.push_back(loader.LoadTextureAsync(L"Assets\\seafloor.dds", nullptr, m_sphereTexture.put()));
    tasks.push_back(loader.LoadTextureAsync(L"Assets\\metal_texture.dds", nullptr, m_cylinderTexture.put()));
    tasks.push_back(loader.LoadTextureAsync(L"Assets\\cellceiling.dds", nullptr, m_ceilingTexture.put()));
    tasks.push_back(loader.LoadTextureAsync(L"Assets\\cellfloor.dds", nullptr, m_floorTexture.put()));
    tasks.push_back(loader.LoadTextureAsync(L"Assets\\cellwall.dds", nullptr, m_wallsTexture.put()));*/

    // Simulate loading additional resources by introducing a delay.
    tasks.push_back([]() -> IAsyncAction { co_await winrt::resume_after(2s); }());

    // Wait for all the tasks to complete.
    for (auto&& task : tasks)
    {
        co_await task;
    }
}

void MasterRenderer::ReleaseDeviceDependentResources()
{
    // On device lost all the device resources are invalid.
    // Set the state of the renderer to not have a pointer to the
    // Simple3DGame object. It will be reset as a part of the
    // game devices resources being recreated.
    //m_game = nullptr;
    //m_2DRenderer.ReleaseDeviceDependentResources();
}

void MasterRenderer::setTextLayoutPixels(UIText* text)
{
    m_uiElementRenderer.setTextLayoutPixels(text);
}

void MasterRenderer::Render()
{
    auto d3dContext{ m_deviceResources->GetD3DDeviceContext() };
    auto d2dContext{ m_deviceResources->GetD2DDeviceContext() };

    ID3D11RenderTargetView* const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };

    d3dContext->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());
    d3dContext->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    d2dContext->SetTarget(m_deviceResources->GetD2DTargetBitmap());

    D2D1::ColorF clear = m_uiElementRenderer.getClearColor(m_mode->getBackgroundColor());
    const float clearColor[4] = { clear.r, clear.g, clear.b, clear.a };

    d3dContext->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), clearColor);

    if (m_mode != nullptr && m_mode->needs3DRendering())
    {
        // Update variables that change once per frame.

        ConstantBufferChangesEveryFrame constantBufferChangesEveryFrameValue;
        XMStoreFloat4x4(
            &constantBufferChangesEveryFrameValue.view,
            XMMatrixTranspose(m_mode->getCameraView())
        );
        d3dContext->UpdateSubresource(
            m_constantBufferChangesEveryFrame.get(),
            0,
            nullptr,
            &constantBufferChangesEveryFrameValue,
            0,
            0
        );

        // Set up the graphics pipeline. This sample uses the same InputLayout and set of
        // constant buffers for all shaders, so they only need to be set once per frame.

        d3dContext->IASetInputLayout(m_vertexLayout.get());
        ID3D11Buffer* constantBufferNeverChanges{ m_constantBufferNeverChanges.get() };
        d3dContext->VSSetConstantBuffers(0, 1, &constantBufferNeverChanges);
        ID3D11Buffer* constantBufferChangeOnResize{ m_constantBufferChangeOnResize.get() };
        d3dContext->VSSetConstantBuffers(1, 1, &constantBufferChangeOnResize);
        ID3D11Buffer* constantBufferChangesEveryFrame{ m_constantBufferChangesEveryFrame.get() };
        d3dContext->VSSetConstantBuffers(2, 1, &constantBufferChangesEveryFrame);
        ID3D11Buffer* constantBufferChangesEveryPrim{ m_constantBufferChangesEveryPrim.get() };
        d3dContext->VSSetConstantBuffers(3, 1, &constantBufferChangesEveryPrim);

        d3dContext->PSSetConstantBuffers(2, 1, &constantBufferChangesEveryFrame);
        d3dContext->PSSetConstantBuffers(3, 1, &constantBufferChangesEveryPrim);
        ID3D11SamplerState* samplerLinear{ m_samplerLinear.get() };
        d3dContext->PSSetSamplers(0, 1, &samplerLinear);

        for (auto&& object : m_mode->getCurrentModeVolumeElements())
        {
            object->Render(d3dContext, m_constantBufferChangesEveryPrim.get());
        }
    }

    //With 3D rendering complete, we now begin 2D rendering. This includes things like
    //text and UI Elements such as buttons, drop down boxes, etc.
    d3dContext->BeginEventInt(L"D2D BeginDraw", 1);
    d2dContext->BeginDraw();

    // To handle the swapchain being pre-rotated, set the D2D transformation to include it.
    d2dContext->SetTransform(m_deviceResources->GetOrientationTransform2D());

    //Render any UI elements or text on screen text
    m_uiElementRenderer.render(m_mode->getCurrentModeUIElements());

    // We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
    // is lost. It will be handled during the next call to Present.
    HRESULT hr = d2dContext->EndDraw();
    d3dContext->EndEvent();
    if (hr != D2DERR_RECREATE_TARGET)
    {
        winrt::check_hresult(hr);
    }
}