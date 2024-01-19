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

std::pair<DirectX::XMFLOAT2, DirectX::XMFLOAT2> Line::getPointsRelative()
{
	//NOTE: I've been using the terms "Relative" and "Absolute" in a pretty cavalier way.
	//I've swapped them at times, and at other times just plain used the terms incorrectly.
	//With that said, what this method does is give the absolute points of the line. The 
	//getPointsAbsolute() method above is really giving the relative coordinates. I'll
	//definitely need to go through all of the UI Elements at some point and clean all of 
	//these references up, but for now I'm burnt out on working on UI stuff so I'll do this
	//at some point in the future.

	//Take the pixel coordinates for the line and divide them by the current screen size to
	//get the true absolute coordinates
	auto location = getPixelLocation();
	auto size = getPixelSize();

	location.x /= m_screenSize->Width;
	location.y /= m_screenSize->Height;
	size.x /= m_screenSize->Width;
	size.y /= m_screenSize->Height;

	return { {location.x - size.x / 2.0f, location.y - size.y / 2.0f}, {location.x + size.x / 2.0f, location.y + size.y / 2.0f} };
}