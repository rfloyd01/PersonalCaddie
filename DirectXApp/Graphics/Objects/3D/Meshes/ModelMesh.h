#pragma once

// FaceMesh:
// This class derives from MeshObject and creates a ID3D11Buffer of
// vertices and indices to represent a canonical face defined as a
// rectangle at the origin extending 1 unit in the +X and
// 1 unit in the +Y direction.
// The face is defined to be two sided, so it is visible from either
// side.

#include "MeshObject.h"
#include "Graphics/Rendering/ConstantBuffers.h"

class ModelMesh : public MeshObject
{
public:
    ModelMesh(_In_ winrt::com_ptr<ID3D11Device3> const& device, std::vector<PNTVertex> const& vertices, std::vector<uint16_t> const& indices);
};
