#include "pch.h"
#include "VolumeElement.h"

using namespace DirectX;

VolumeElement::VolumeElement() :
    m_normalMaterial(nullptr),
    m_hitMaterial(nullptr)
{
    m_active = false;
    m_target = false;
    m_targetId = 0;
    m_hit = false;
    m_ground = true;

    m_position = XMFLOAT3(0.0f, 0.0f, 0.0f);
    m_velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
    m_defaultXAxis = XMFLOAT3(1.0f, 0.0f, 0.0f);
    m_defaultYAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
    m_defaultZAxis = XMFLOAT3(0.0f, 0.0f, 1.0f);
    XMStoreFloat4x4(&m_modelMatrix, XMMatrixIdentity());

    m_hitTime = 0.0f;

    m_animatePosition = nullptr;
}

void VolumeElement::Render(
    _In_ ID3D11DeviceContext* context,
    _In_ ID3D11Buffer* primitiveConstantBuffer
)
{
    if (!m_active || (m_mesh == nullptr) || (m_normalMaterial == nullptr))
    {
        return;
    }

    ConstantBufferChangesEveryPrim constantBuffer;

    XMStoreFloat4x4(
        &constantBuffer.worldMatrix,
        XMMatrixTranspose(getModelMatrix())
    );

    if (m_hit && m_hitMaterial != nullptr)
    {
        m_hitMaterial->RenderSetup(context, &constantBuffer);
    }
    else
    {
        m_normalMaterial->RenderSetup(context, &constantBuffer);
    }
    context->UpdateSubresource(primitiveConstantBuffer, 0, nullptr, &constantBuffer, 0, 0);

    m_mesh->Render(context);
}