#include "pch.h"
#include "MasterRenderer.h"

using namespace DirectX;
using namespace winrt::Windows::Foundation;

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

    //if (m_game != nullptr)
    //{
    //    // Update the Projection Matrix and the associated Constant Buffer.
    //    m_game->GameCamera().SetProjParams(
    //        XM_PI / 2, renderTargetSize.Width / renderTargetSize.Height,
    //        0.01f,
    //        100.0f
    //    );

    //    XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

    //    ConstantBufferChangeOnResize changesOnResize;
    //    XMStoreFloat4x4(
    //        &changesOnResize.projection,
    //        XMMatrixMultiply(
    //            XMMatrixTranspose(m_game->GameCamera().Projection()),
    //            XMMatrixTranspose(XMLoadFloat4x4(&orientation))
    //        )
    //    );

    //    d3dContext->UpdateSubresource(
    //        m_constantBufferChangeOnResize.get(),
    //        0,
    //        nullptr,
    //        &changesOnResize,
    //        0,
    //        0
    //    );
    //}
}

void MasterRenderer::CreateModeResources()
{
    //In the original DirectX example this is an asynchronus function that loads certain resources for 
    //the game. For now just make this a normal function, but if loading starts taking awhile then
    //make this asynchronus
    //auto renderText = m_mode->getCurrentModeText();
    //for (int i = 0; i < renderText->size(); i++) editText(renderText->at(i));

    ////delete any existing menuObjects from the renderer
    //deleteExistingMenuObjects();
    //auto renderMenuObjects = m_mode->getCurrentModeMenuObjects();
    //for (int i = 0; i < renderMenuObjects.size(); i++) m_2DRenderer.addMenuObject(renderMenuObjects[i]);
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

void MasterRenderer::Render()
{
    auto d3dContext{ m_deviceResources->GetD3DDeviceContext() };
    auto d2dContext{ m_deviceResources->GetD2DDeviceContext() };

    ID3D11RenderTargetView* const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };

    d3dContext->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());
    d3dContext->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    d2dContext->SetTarget(m_deviceResources->GetD2DTargetBitmap());

    // Doing the Mono or Left Eye View.
    
    D2D1::ColorF clear = m_uiElementRenderer.getClearColor(m_mode->getBackgroundColor());
    const float clearColor[4] = { clear.r, clear.g, clear.b, clear.a };

    d3dContext->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), clearColor);

    //TODO: Uncomment the below block when ready to start working on 3d visuals

    //if (m_game != nullptr && m_gameResourcesLoaded && m_levelResourcesLoaded)
    //{
    //    // This section is only used after the game state has been initialized and all device
    //    // resources needed for the game have been created and associated with the game objects.
    //    if (stereoEnabled)
    //    {
    //        // When doing stereo, it is necessary to update the projection matrix once per rendering pass.

    //        auto orientation = m_deviceResources->GetOrientationTransform3D();

    //        ConstantBufferChangeOnResize changesOnResize;
    //        XMStoreFloat4x4(
    //            &changesOnResize.projection,
    //            XMMatrixMultiply(
    //                XMMatrixTranspose(
    //                    i == 0 ?
    //                    m_game->GameCamera().LeftEyeProjection() :
    //                    m_game->GameCamera().RightEyeProjection()
    //                ),
    //                XMMatrixTranspose(XMLoadFloat4x4(&orientation))
    //            )
    //        );

    //        d3dContext->UpdateSubresource(
    //            m_constantBufferChangeOnResize.get(),
    //            0,
    //            nullptr,
    //            &changesOnResize,
    //            0,
    //            0
    //        );
    //    }
    //    // Update variables that change once per frame.

    //    ConstantBufferChangesEveryFrame constantBufferChangesEveryFrameValue;
    //    XMStoreFloat4x4(
    //        &constantBufferChangesEveryFrameValue.view,
    //        XMMatrixTranspose(m_game->GameCamera().View())
    //    );
    //    d3dContext->UpdateSubresource(
    //        m_constantBufferChangesEveryFrame.get(),
    //        0,
    //        nullptr,
    //        &constantBufferChangesEveryFrameValue,
    //        0,
    //        0
    //    );

    //    // Set up the graphics pipeline. This sample uses the same InputLayout and set of
    //    // constant buffers for all shaders, so they only need to be set once per frame.

    //    d3dContext->IASetInputLayout(m_vertexLayout.get());
    //    ID3D11Buffer* constantBufferNeverChanges{ m_constantBufferNeverChanges.get() };
    //    d3dContext->VSSetConstantBuffers(0, 1, &constantBufferNeverChanges);
    //    ID3D11Buffer* constantBufferChangeOnResize{ m_constantBufferChangeOnResize.get() };
    //    d3dContext->VSSetConstantBuffers(1, 1, &constantBufferChangeOnResize);
    //    ID3D11Buffer* constantBufferChangesEveryFrame{ m_constantBufferChangesEveryFrame.get() };
    //    d3dContext->VSSetConstantBuffers(2, 1, &constantBufferChangesEveryFrame);
    //    ID3D11Buffer* constantBufferChangesEveryPrim{ m_constantBufferChangesEveryPrim.get() };
    //    d3dContext->VSSetConstantBuffers(3, 1, &constantBufferChangesEveryPrim);

    //    d3dContext->PSSetConstantBuffers(2, 1, &constantBufferChangesEveryFrame);
    //    d3dContext->PSSetConstantBuffers(3, 1, &constantBufferChangesEveryPrim);
    //    ID3D11SamplerState* samplerLinear{ m_samplerLinear.get() };
    //    d3dContext->PSSetSamplers(0, 1, &samplerLinear);

    //    for (auto&& object : m_game->RenderObjects())
    //    {
    //        object->Render(d3dContext, m_constantBufferChangesEveryPrim.get());
    //    }
    //}

    //Begin 2d rendering
    d3dContext->BeginEventInt(L"D2D BeginDraw", 1);
    d2dContext->BeginDraw();

    // To handle the swapchain being pre-rotated, set the D2D transformation to include it.
    d2dContext->SetTransform(m_deviceResources->GetOrientationTransform2D());

    //Render any UI elements or text on screen text
    //m_2DRenderer.Render(m_mode);
    m_uiElementRenderer.render(m_mode->getCurrentModeUIElements());

    //Render 2D UI elements like drop down boxes and buttons
    //for (int i = 0; i < m_mode->getCurrentModeMenuObjects().size(); i++) m_mode->getCurrentModeMenuObjects()[i]->Render(d2dContext);

    // We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
    // is lost. It will be handled during the next call to Present.
    HRESULT hr = d2dContext->EndDraw();
    d3dContext->EndEvent();
    if (hr != D2DERR_RECREATE_TARGET)
    {
        winrt::check_hresult(hr);
    }
}