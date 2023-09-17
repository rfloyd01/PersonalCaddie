#pragma once

// VolumeElement:
// This is the class representing a generic three dimensional object. There are
// specific sub-classes that have specific geometric shapes. This class contains all the
// properties of objects that are common.

#include "Graphics/Objects/3D/Meshes/MeshObject.h"
#include "Graphics/Objects/3D/Animate.h"
#include "Graphics/Rendering/Material.h"

class VolumeElement
{
public:
    VolumeElement();

    void Render(
        _In_ ID3D11DeviceContext* context,
        _In_ ID3D11Buffer* primitiveConstantBuffer
    );

    void AnimatePosition(_In_ std::shared_ptr<Animate> const& animate);
    std::shared_ptr<Animate> AnimatePosition();

    void setMesh(_In_ std::shared_ptr<MeshObject> const& mesh);

    void setMaterial(_In_ std::shared_ptr<Material> const& material);
    std::shared_ptr<Material> getMaterial();

    void setPosition(DirectX::XMFLOAT3 position);
    void setPosition(DirectX::XMVECTOR position);
    void setVelocity(DirectX::XMFLOAT3 velocity);
    void setVelocity(DirectX::XMVECTOR velocity);
    DirectX::XMMATRIX getModelMatrix();
    DirectX::XMFLOAT3 getPosition();
    DirectX::XMVECTOR getVectorPosition();
    DirectX::XMVECTOR getVectorVelocity();
    DirectX::XMFLOAT3 getVelocity();

protected:
    DirectX::XMFLOAT4X4 m_modelMatrix;
    DirectX::XMFLOAT3   m_position;

private:
    virtual void UpdatePosition() {};
    // Object Data
    bool                         m_active;
    bool                         m_target;
    int                          m_targetId;
    bool                         m_hit;
    bool                         m_ground;

    DirectX::XMFLOAT3            m_velocity;

    std::shared_ptr<Material>    m_normalMaterial;
    std::shared_ptr<Material>    m_hitMaterial;

    DirectX::XMFLOAT3            m_defaultXAxis;
    DirectX::XMFLOAT3            m_defaultYAxis;
    DirectX::XMFLOAT3            m_defaultZAxis;

    float                        m_hitTime;

    std::shared_ptr<Animate>     m_animatePosition;
    std::shared_ptr<MeshObject>  m_mesh;
};

__forceinline void VolumeElement::setPosition(DirectX::XMFLOAT3 position)
{
    m_position = position;
    // Update any internal states that are dependent on the position.
    // UpdatePosition is a virtual function that is specific to the derived class.
    UpdatePosition();
}

__forceinline void VolumeElement::setPosition(DirectX::XMVECTOR position)
{
    XMStoreFloat3(&m_position, position);
    // Update any internal states that are dependent on the position.
    // UpdatePosition is a virtual function that is specific to the derived class.
    UpdatePosition();
}

__forceinline DirectX::XMFLOAT3 VolumeElement::getPosition()
{
    return m_position;
}

__forceinline DirectX::XMVECTOR VolumeElement::getVectorPosition()
{
    return DirectX::XMLoadFloat3(&m_position);
}

__forceinline void VolumeElement::setVelocity(DirectX::XMFLOAT3 velocity)
{
    m_velocity = velocity;
}

__forceinline void VolumeElement::setVelocity(DirectX::XMVECTOR velocity)
{
    XMStoreFloat3(&m_velocity, velocity);
}

__forceinline DirectX::XMFLOAT3 VolumeElement::getVelocity()
{
    return m_velocity;
}

__forceinline DirectX::XMVECTOR VolumeElement::getVectorVelocity()
{
    return DirectX::XMLoadFloat3(&m_velocity);
}

__forceinline void VolumeElement::AnimatePosition(_In_ std::shared_ptr<Animate> const& animate)
{
    m_animatePosition = animate;
}

__forceinline std::shared_ptr<Animate> VolumeElement::AnimatePosition()
{
    return m_animatePosition;
}

__forceinline void VolumeElement::setMaterial(_In_ std::shared_ptr<Material> const& material)
{
    m_normalMaterial = material;
}

__forceinline std::shared_ptr<Material> VolumeElement::getMaterial()
{
    return m_normalMaterial;
}

__forceinline void VolumeElement::setMesh(_In_ std::shared_ptr<MeshObject> const& mesh)
{
    m_mesh = mesh;
}

__forceinline DirectX::XMMATRIX VolumeElement::getModelMatrix()
{
    return DirectX::XMLoadFloat4x4(&m_modelMatrix);
}