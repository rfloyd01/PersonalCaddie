#include "pch.h"
#include "Ellipse.h"

Ellipse::Ellipse(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 centerLocation, DirectX::XMFLOAT2 radii, bool circle, UIColor color)
{
	//For m_shape, we still use a rectangle struct, however, the first two values are the x and y coordinates of the first
	//point while the second two values are the x and y coordinates for the second point.
	m_location = centerLocation;
	m_size = radii;

	m_shape = { {centerLocation.x, centerLocation.y, radii.x, radii.y}, color, UIShapeFillType::Fill, UIShapeType::ELLIPSE};
	m_circle = circle; //if we want a circle, then on resize we need to make sure that both radii are equal to each other

	resize(windowSize);
}

void Ellipse::resize(winrt::Windows::Foundation::Size windowSize)
{
	//The ellipse class uses the m_shape variable differently than other UI elements so it needs
	//its own resize() method
	m_shape.m_rectangle.left = windowSize.Width * m_location.x;
	m_shape.m_rectangle.top = windowSize.Height * m_location.y;
	m_shape.m_rectangle.right = windowSize.Width * m_size.x;
	
	if (m_circle) m_shape.m_rectangle.bottom = m_shape.m_rectangle.right;
	else m_shape.m_rectangle.bottom = windowSize.Height * m_size.y;
}