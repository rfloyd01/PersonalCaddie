#pragma once

#include "Box.h"

#define MAX_HEIGHT 2160.0f
#define MAX_WIDTH 3840.0f

class RelativeBox : public Box
{
public:
	RelativeBox(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, UIColor color = UIColor::Black, UIShapeFillType fill = UIShapeFillType::Fill, bool isSquare = false) :
		Box(windowSize, location, size, color, fill, isSquare)
	{
		//Using the maximum window sizes, calculate the relative distance that the right edge
		//of the box would be from a vertical centerline (in terms of percentage of window length)
		//and calculate the relative distance that the top edge of the box would be from a 
		//horizontal center lin (in terms of percentage of window height).
		DirectX::XMFLOAT2 element_pixel_center = { MAX_WIDTH * m_location.x, MAX_HEIGHT * m_location.y }; //ex. {2496, 1404}
		DirectX::XMFLOAT2 screen_pixel_center = { MAX_WIDTH * 0.5f, MAX_HEIGHT * 0.5f }; //ex. {1920, 1080}

		//save absolute ratio of center_point.x to vertical center line and center_point.y
		//to horizontal center line.
		float maximum_pixel_height = m_size.y / (MAX_WIDTH + MAX_HEIGHT) * (2 * MAX_WIDTH * MAX_HEIGHT); //ex. 276.48
		m_maxRelativeDistanceToHorizontal = element_pixel_center.x - screen_pixel_center.x; //ex. 576
		m_maxRelativeDistanceToVertical = (element_pixel_center.y - screen_pixel_center.y) / MAX_HEIGHT; //ex. ???
		m_horizontalMultiplier = m_maxRelativeDistanceToHorizontal / maximum_pixel_height; //ex. 25/12 = 2.083333

		m_maxRelativeDistanceToHorizontal /= MAX_WIDTH; //convert horizontal pixels to absolute units ex. 576/3840 = 0.15
	}

	virtual void resize(winrt::Windows::Foundation::Size windowSize) override
	{
		//For the RelativeBox class the m_size variable now represents {ratio of box width to height, relative ratio of box height to screen height}.
		//The pixel height of the box is calculated using a weighted average of the current ratio of the screen width to height. 
		//Once this pixel height is calculated the pixel width is calculated using size.x for the width to height ratio. The
		//absolute location of the box remains the same when resizing

		DirectX::XMFLOAT2 pixel_center = { windowSize.Width * m_location.x, windowSize.Height * m_location.y };
		DirectX::XMFLOAT2 box_pixel_dimensions;

		box_pixel_dimensions.y = m_size.y / (windowSize.Width + windowSize.Height) * (2 * windowSize.Width * windowSize.Height);
		box_pixel_dimensions.x = m_size.x * box_pixel_dimensions.y;

		//TODO: Add vertical calculations
		float newPixelDistanceToHorizontal = m_horizontalMultiplier * m_size.y / (windowSize.Width + windowSize.Height) * (2 * windowSize.Width * windowSize.Height); //ex. 523.256
		float newPixelDistanceToVertical = m_maxRelativeDistanceToVertical / (windowSize.Width + windowSize.Height) * (2 * windowSize.Width * windowSize.Height); //ex. ???

		//TODO: The below four lines should be able to be combined into two and get rid of some division along the way
		float horizontal_relative_difference = newPixelDistanceToHorizontal / windowSize.Width - m_maxRelativeDistanceToHorizontal; //ex. 523.256 / 3000 - 0.15 = 0.0244187
		float vertical_relative_difference = newPixelDistanceToVertical / windowSize.Height - m_maxRelativeDistanceToVertical; //ex. ???
		
		pixel_center.x += horizontal_relative_difference * windowSize.Width; //i.e += 0.0244187 * 3000 --> += 73.256
		pixel_center.y += vertical_relative_difference * windowSize.Height; // i.e. ???

		m_shape.m_rectangle.left = pixel_center.x - box_pixel_dimensions.x / 2.0f;
		m_shape.m_rectangle.top = pixel_center.y - box_pixel_dimensions.y / 2.0f;
		m_shape.m_rectangle.right = pixel_center.x + box_pixel_dimensions.x / 2.0f;
		m_shape.m_rectangle.bottom = pixel_center.y + box_pixel_dimensions.y / 2.0f;
	}

	float m_maxRelativeDistanceToVertical;
	float m_maxRelativeDistanceToHorizontal;

	float m_horizontalMultiplier;
};