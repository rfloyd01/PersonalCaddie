#include "pch.h"
#include "Line.h"

Line::Line(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 firstPointLlocation, DirectX::XMFLOAT2 secondPointLlocation,
	UIColor color, float width, bool useAbsolute)
{
	m_screenSize = windowSize;

	//For m_shape, we still use a rectangle struct, however, the first two values are the x and y coordinates of the first
	//point while the second two values are the x and y coordinates for the second point. The m_location variable becomes
	//the mid-point of the line
	DirectX::XMFLOAT2 location = { (firstPointLlocation.x + secondPointLlocation.x) / 2.0f, (firstPointLlocation.y + secondPointLlocation.y) / 2.0f };
	DirectX::XMFLOAT2 size = { secondPointLlocation.x - firstPointLlocation.x, secondPointLlocation.y - firstPointLlocation.y };

	m_useAbsoluteCoordinates = useAbsolute;

	//Horizontal lines will have a height component of zero, which will mess up some calculations
	//in other places. In this case simply give the line a very minimal height.
	if (size.y == 0.0f) size.y = 0.00001f;
	
	updateLocationAndSize(location, size);

	m_lineColor = color;

	//TODO: If the second point comes before the first point along the x or y axis then the 
	//size variable will contain negative numbers. Is this an issue?

	m_shape = { {0, 0, 0, 0}, color, UIShapeFillType::NoFill, UIShapeType::LINE, width };
	resize();
}

std::pair<DirectX::XMFLOAT2, DirectX::XMFLOAT2> Line::getPointsAbsolute()
{
	//Returns the two points making up the line with their absolute window coordinates
	auto location = getAbsoluteLocation();
	auto size = getAbsoluteSize();
	return { {location.x - size.x / 2.0f, location.y - size.y / 2.0f}, {location.x + size.x / 2.0f, location.y + size.y / 2.0f} };
}