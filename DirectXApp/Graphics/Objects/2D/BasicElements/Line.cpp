#include "pch.h"
#include "Line.h"

Line::Line(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 firstPointLlocation, DirectX::XMFLOAT2 secondPointLlocation, UIColor color)
{
	//For m_shape, we still use a rectangle struct, however, the first two values are the x and y coordinates of the first
	//point while the second two values are the x and y coordinates for the second point.
	m_location = {(firstPointLlocation.x + secondPointLlocation.x) / (float)2.0, (firstPointLlocation.y + secondPointLlocation.y) / (float)2.0 };
	m_size = { secondPointLlocation.x - firstPointLlocation.x, secondPointLlocation.y - firstPointLlocation.y };

	m_shape = { {0, 0, 0, 0}, color, UIShapeFillType::NoFill, UIShapeType::LINE };
	resize(windowSize);
}