#pragma once

// Face:
// This class is a specialization of GameObject that represents a parallelogram primitive.
// The face is defined by three points. It is positioned at 'origin'. The four corners
// of the face are defined at (p1.x, p1.y), (p2.x, p1.y), (p1.x, p2.y), (p2.x, p2.y)  

#include "VolumeElement.h"

class Face : public VolumeElement
{
public:
    Face();
    Face(
        DirectX::XMFLOAT3 origin,
        DirectX::XMFLOAT3 p1,
        DirectX::XMFLOAT3 p2
        );

    void SetPlane(
        DirectX::XMFLOAT3 origin,
        DirectX::XMFLOAT3 p1,
        DirectX::XMFLOAT3 p2
        );

    void rotateFaceAboutVector(DirectX::XMVECTOR axis, float degrees);
    void translateFace(DirectX::XMFLOAT3 location);
    void translateAndRotateFace(DirectX::XMFLOAT3 location, DirectX::XMVECTOR axis, float degrees);
    void translateAndRotateFace(DirectX::XMFLOAT3 location, DirectX::XMVECTOR quat);

    /*void setRotation(DirectX::XMFLOAT4X4 rotation);
    void setRotation(DirectX::XMMATRIX rotation);*/

protected:
    virtual void UpdatePosition() override;

private:
    void UpdateMatrix();

    DirectX::XMFLOAT3   m_widthVector;
    DirectX::XMFLOAT3   m_heightVector;
    DirectX::XMFLOAT3   m_normal;
    DirectX::XMFLOAT3   m_point[4];
    float               m_width;
    float               m_height;
    DirectX::XMFLOAT4X4 m_rotationMatrix;
    DirectX::XMFLOAT3   m_location;
};

//__forceinline void Face::setRotation(DirectX::XMFLOAT4X4 rotation)
//{
//    m_rotationMatrix = rotation;
//    // Update any internal states that are dependent on the position.
//    // UpdatePosition is a virtual function that is specific to the derived class.
//    UpdatePosition();
//}
//
//__forceinline void Face::setRotation(DirectX::XMMATRIX rotation)
//{
//    XMStoreFloat4x4(&m_rotationMatrix, rotation);
//    // Update any internal states that are dependent on the position.
//    // UpdatePosition is a virtual function that is specific to the derived class.
//    UpdatePosition();
//}