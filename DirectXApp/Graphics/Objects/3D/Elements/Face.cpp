#include "pch.h"
#include "Face.h"

using namespace DirectX;

Face::Face()
{
    SetPlane(
        XMFLOAT3(0.0f, 0.0f, 0.0f),
        XMFLOAT3(1.0f, 0.0f, 0.0f),
        XMFLOAT3(0.0f, 1.0f, 0.0f)
        );
}

Face::Face(
    XMFLOAT3 origin,
    XMFLOAT3 p1,
    XMFLOAT3 p2
    )
{
    SetPlane(origin, p1, p2);
}

void Face::SetPlane(
    XMFLOAT3 origin,
    XMFLOAT3 p1,
    XMFLOAT3 p2
    )
{
    m_position = origin;
    m_location = {(origin.x + p1.x) / 2.0f, (origin.y + p2.y) / 2.0f, (p1.z + p2.z) / 2.0f }; //m_location represents the exact center of the face
    XMStoreFloat3(&m_widthVector, XMLoadFloat3(&p1) - XMLoadFloat3(&origin));
    XMStoreFloat3(&m_heightVector, XMLoadFloat3(&p2) - XMLoadFloat3(&origin));

    m_point[0] = origin;
    m_point[1] = p1;
    m_point[3] = p2;
    XMStoreFloat3(&m_point[2], XMLoadFloat3(&p1) + XMLoadFloat3(&m_heightVector));

    //figure out the max and min values for x, y and z from the points and use 
    //these values to set the position of the face to be the center of the face
    float minX = m_point[0].x, maxX = m_point[0].x, minY = m_point[0].y, maxY = m_point[0].y, minZ = m_point[0].z, maxZ = m_point[0].z;
    for (int i = 0; i < 4; i++)
    {
        if (m_point[i].x < minX) minX = m_point[i].x;
        if (m_point[i].x > maxX) maxX = m_point[i].x;

        if (m_point[i].y < minY) minY = m_point[i].y;
        if (m_point[i].y > maxY) maxY = m_point[i].y;

        if (m_point[i].z < minZ) minZ = m_point[i].z;
        if (m_point[i].z > maxZ) maxZ = m_point[i].z;
    }

    XMStoreFloat(&m_width, XMVector3Length(XMLoadFloat3(&m_widthVector)));
    XMStoreFloat(&m_height, XMVector3Length(XMLoadFloat3(&m_heightVector)));

    XMStoreFloat3(
        &m_normal,
        XMVector3Normalize(
            XMVector3Cross(
                XMLoadFloat3(&m_widthVector),
                XMLoadFloat3(&m_heightVector)
                )
            )
        );
    UpdateMatrix();
}

void Face::UpdatePosition()
{
    m_point[0] = m_position;
    XMStoreFloat3(&m_point[1], XMLoadFloat3(&m_position) + XMLoadFloat3(&m_widthVector));
    XMStoreFloat3(&m_point[3], XMLoadFloat3(&m_position) + XMLoadFloat3(&m_heightVector));
    XMStoreFloat3(&m_point[2], XMLoadFloat3(&m_point[1]) + XMLoadFloat3(&m_heightVector));

    XMStoreFloat4x4(
        &m_modelMatrix,
        XMMatrixScaling(m_width, m_height, 1.0f) *
        XMLoadFloat4x4(&m_rotationMatrix) * 
        XMMatrixTranslation(m_position.x, m_position.y, m_position.z)
        );
}

void Face::UpdateMatrix()
{
    // Determine the Model transform for the cannonical face to align with the position
    // and orientation of the defined face. The cannonical face geometry is
    // a rectangle at the origin extending 1 unit in the +X and 1 unit in the +Y direction.

    XMVECTOR w = XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);
    XMVECTOR h = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
    XMVECTOR epsilon = XMVectorReplicate(0.0001f);
    XMVECTOR width = XMVectorScale(XMLoadFloat3(&m_widthVector), 1.0f / m_width);
    XMVECTOR height = XMVectorScale(XMLoadFloat3(&m_heightVector), 1.0f / m_height);
    XMMATRIX mat1 = XMMatrixIdentity();
    XMMATRIX mat2 = XMMatrixIdentity();

    // Determine the necessary rotation to align the widthVector with the +X axis.
    // Then apply the rotation to the cannonical H vector (+Y).
    if (!XMVector3NearEqual(w, width, epsilon))
    {
        float angle1 = XMVectorGetX(
            XMVector3AngleBetweenVectors(w, width)
            );

        if (XMVector3NearEqual(w, XMVectorNegate(width), epsilon))
        {
            // The angle between w and width is ~180 degrees, so
            // pick a axis of rotation perpendicular to the W vector.
            mat1 = XMMatrixRotationAxis(XMVector3Orthogonal(w), angle1);
            h = XMVector3TransformCoord(h, mat1);
        }
        else if ((angle1 * angle1) > 0.025)
        {
            // Take the cross product between the w and width vectors to
            // determine the axis to rotate that is perpendicular to both vectors.
            XMVECTOR axis1 = XMVector3Cross(w, width);
            mat1 = XMMatrixRotationAxis(axis1, angle1);
            h = XMVector3TransformCoord(h, mat1);
        }
    }

    // Determine the necessary rotation to align the transformed heightVector
    // with the transformed H vector.
    if (!XMVector3NearEqual(h, height, epsilon))
    {
        float angle2 = XMVectorGetX(
            XMVector3AngleBetweenVectors(h, height)
            );

        if (XMVector3NearEqual(h, XMVectorNegate(height), epsilon))
        {
            // The angle between the h' and height vectors is ~180 degrees,
            // so rotate around the width vector to keep it aligned in the right
            // place.
            mat2 = XMMatrixRotationAxis(width, angle2);
        }
        else if ((angle2 * angle2) > 0.025)
        {
            // Take the cross product between the h' and height vectors
            // to determine the axis to rotate that is perpendicular to both vectors.
            XMVECTOR axis2 = XMVector3Cross(h, height);
            mat2 = XMMatrixRotationAxis(axis2, angle2);
        }
    }
    XMStoreFloat4x4(&m_rotationMatrix, mat1 * mat2);

    // Generate the composite matrix to scale, rotate and translate to cannonical
    // face geometry to be positioned correctly in world space.
    XMStoreFloat4x4(
        &m_modelMatrix,
        XMMatrixScaling(m_width, m_height, 1.0f) *
        mat1 *
        mat2 *
        XMMatrixTranslation(m_position.x, m_position.y, m_position.z)
        );
}

//void Face::rotateFace(float pitch, float yaw, float roll)
//{
//    //Rotates the face about the given point using the given Euler Angles.
//    /*XMStoreFloat4x4(
//        &m_modelMatrix,
//        XMMatrixScaling(m_width, m_height, 1.0f) *
//        XMMatrixRotationRollPitchYaw(pitch, yaw, roll) *
//        XMMatrixTranslation(0.5f, 0.0f, 1.0f)
//    );*/
//
//    //For the rotation to work correctly we need to translate the model 
//    //to the front of the screen, rotate it, and then translate it back
//    //to it's current location
//    XMFLOAT3 yo = { 0.5f, 0.5f, 0.5f };
//    //setPosition(yo); //move to origin
//    auto currentModel = XMLoadFloat4x4(&m_modelMatrix);
//    auto yeet = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
//
//    XMStoreFloat4x4(
//        &m_modelMatrix,
//        currentModel *
//        XMMatrixTranslation(0, 0, -m_position.z) *
//        XMMatrixRotationRollPitchYaw(pitch, yaw, roll) *
//        XMMatrixTranslation(0, 0, m_position.z)
//    );
//
//    /*XMStoreFloat4x4(
//        &m_modelMatrix,
//        currentModel *
//        XMMatrixRotationRollPitchYaw(pitch, yaw, roll)
//    );
//
//    XMStoreFloat4x4(
//        &m_modelMatrix,
//        currentModel *
//        XMMatrixTranslation(m_position.x, m_position.y, m_position.z)
//    );*/
//
//    //setPosition(m_position); //move back to current location
//    //Draw a line through the center of the face that's parallel to the Z-axis (this is using
//    //model coordinates)
//    /*auto zzTop = XMVectorSet(0.5f, 0.5f, 0.0f, 1.0f);
//    auto yeet = XMVector4Transform(zzTop, XMLoadFloat4x4(&m_modelMatrix));
//    int x = 5;*/
//}

void Face::rotateFaceAboutVector(DirectX::XMVECTOR axis, float degrees)
{
    //When the Face volume element is created, it happens in the x-y plane with a z value
    //of 0 (basically, it gets created right on top of the screen). It then get's scaled, 
    //translated and rotated accordingly. In order to carry out further rotations on the
    //face it must be translated back into the x-y plane at the screen. We then carry out 
    //the appropriate rotation before translating it back to its correct location.

    translateFace({ -m_location.x, -m_location.y, -m_location.z }); //move back to the origin
    //translateFace({ 0, 0, -m_location.z }); //move back to the origin

    //then carry out the rotation
    auto currentModel = XMLoadFloat4x4(&m_modelMatrix);
    XMStoreFloat4x4(
        &m_modelMatrix,
        currentModel *
        XMMatrixRotationAxis(axis, degrees)
    );

    translateFace({ m_location.x, m_location.y, m_location.z }); //and finally move the Face back to the correct location
    //translateFace({ 0, 0, m_location.z }); //move back to the origin
}

void Face::translateFace(DirectX::XMFLOAT3 location)
{
    //Translates the face in space by the amount dictated in the location vector
    auto currentModel = XMLoadFloat4x4(&m_modelMatrix);
    /*m_location.x += location.x;
    m_location.y += location.y;
    m_location.z += location.z;*/

    /*XMStoreFloat4x4(
        &m_modelMatrix,
        currentModel *
        XMMatrixTranslation(location.x, location.y, location.z)
    );*/

    //Add new translation on top of existing one
    XMStoreFloat4x4(
        &m_modelMatrix,
        XMMatrixScaling(m_width, m_height, 1.0f) *
        XMLoadFloat4x4(&m_rotationMatrix) *
        XMMatrixTranslation(m_position.x, m_position.y, m_position.z) *
        XMMatrixTranslation(location.x, location.y, location.z)
    );
}

void Face::translateAndRotateFace(DirectX::XMFLOAT3 location, DirectX::XMVECTOR axis, float degrees)
{
    //Add new translation and rotatino on top of existing ones
    XMStoreFloat4x4(
        &m_modelMatrix,
        XMMatrixScaling(m_width, m_height, 1.0f) *
        XMLoadFloat4x4(&m_rotationMatrix) *
        XMMatrixTranslation(m_location.x, m_location.y, m_location.z ) * 
        XMMatrixRotationAxis(axis, degrees) *
        XMMatrixTranslation(location.x, location.y, location.z)
    );
}