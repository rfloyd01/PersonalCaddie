#include "pch.h"
#include "Ellipse.h"

Ellipse::Ellipse(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 centerLocation, DirectX::XMFLOAT2 radii, bool circle, UIColor color)
{
	//For m_shape, we still use a rectangle struct, however, the first two values are the x and y coordinates of the first
	//point while the second two values are the x and y coordinates for the second point.
	updateLocationAndSize(centerLocation, radii);

	m_shape = { {centerLocation.x, centerLocation.y, radii.x, radii.y}, color, UIShapeFillType::Fill, UIShapeType::ELLIPSE};
	m_circle = circle; //if we want a circle, then on resize we need to make sure that both radii are equal to each other

	resize(windowSize);
}

void Ellipse::resize(winrt::Windows::Foundation::Size windowSize)
{
	//The ellipse class uses the m_shape variable differently than other UI elements so it needs
	//its own resize() method
	if (m_useAbsoluteCoordinates)
	{
		m_shape.m_rectangle.left = windowSize.Width * m_location.x;
		m_shape.m_rectangle.top = windowSize.Height * m_location.y;
		m_shape.m_rectangle.right = windowSize.Width * m_size.x;
		m_shape.m_rectangle.bottom = windowSize.Height * m_size.y;
	}
	else
	{
		//For more details on these calculations look at the normal
		//resize method for the UIElement class
		DirectX::XMFLOAT2 element_pixel_center = { windowSize.Width * m_location.x, windowSize.Height * m_location.y };
		DirectX::XMFLOAT2 element_pixel_dimensions;

		element_pixel_dimensions.y = m_size.y / (windowSize.Width + windowSize.Height) * (2 * windowSize.Width * windowSize.Height);
		element_pixel_dimensions.x = m_sizeMultiplier.x * element_pixel_dimensions.y;

		float newPixelDistanceToHorizontal = m_horizontalDriftMultiplier * element_pixel_dimensions.y;
		float newPixelDistanceToVertical = m_absoluteDistanceToScreenCenter.y * element_pixel_dimensions.y / m_size.y;

		element_pixel_center.x += newPixelDistanceToHorizontal - m_absoluteDistanceToScreenCenter.x * windowSize.Width;
		element_pixel_center.y += newPixelDistanceToVertical - m_absoluteDistanceToScreenCenter.y * windowSize.Height;

		m_shape.m_rectangle.left = element_pixel_center.x;
		m_shape.m_rectangle.top = element_pixel_center.y;
		m_shape.m_rectangle.right = element_pixel_dimensions.x;
		m_shape.m_rectangle.bottom = element_pixel_dimensions.y;
	}

	//Resize all child elements as well
	for (int i = 0; i < p_children.size(); i++) p_children[i]->resize(windowSize);
}