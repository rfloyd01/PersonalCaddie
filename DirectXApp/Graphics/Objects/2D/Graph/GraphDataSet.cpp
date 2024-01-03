#include "pch.h"
#include "GraphDataSet.h"
#include "Graphics/Objects/2D/BasicElements/TextOverlay.h"

GraphDataSet::GraphDataSet(DirectX::XMFLOAT2 minimalDataPoint, DirectX::XMFLOAT2 maximalDataPoint)
{
	m_minimalDataPoint = minimalDataPoint;
	m_maximalDataPoint = maximalDataPoint;
}
