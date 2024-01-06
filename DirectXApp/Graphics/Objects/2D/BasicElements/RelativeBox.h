#pragma once

#include "Box.h"

#define MAX_HEIGHT 2160.0f
#define MAX_WIDTH 3840.0f
//#define MAX_HEIGHT 1368.0f
//#define MAX_WIDTH 2560.0f


class RelativeBox : public Box
{
public:
	RelativeBox(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, UIColor color = UIColor::Black, UIShapeFillType fill = UIShapeFillType::Fill, bool isSquare = false) :
		Box(windowSize, location, size, color, fill, isSquare)
	{
		convertAbsoluteCoordinatesToRelativeCoordinates(size);
		//Using the maximum window sizes, calculate the relative distance that the right edge
		//of the box would be from a vertical centerline (in terms of percentage of window length)
		//and calculate the relative distance that the top edge of the box would be from a 
		//horizontal center lin (in terms of percentage of window height).
		DirectX::XMFLOAT2 element_pixel_center = { MAX_WIDTH * m_location.x, MAX_HEIGHT * m_location.y }; //ex. {2496, 1404}
		DirectX::XMFLOAT2 screen_pixel_center = { MAX_WIDTH * 0.5f, MAX_HEIGHT * 0.5f }; //ex. {1920, 1080}

		//Calculate the ratio of the elements width in pixels to its height in pixels when
		//the screen size is at its maximum resolution.
		//m_horizontalSizeMultiplier = (MAX_WIDTH * m_size.x) / (MAX_HEIGHT * m_size.y);

		//save absolute ratio of center_point.x to vertical center line and center_point.y
		//to horizontal center line.
		float maximum_pixel_height = m_size.y / (MAX_WIDTH + MAX_HEIGHT) * (2 * MAX_WIDTH * MAX_HEIGHT); //ex. 276.48
		m_absoluteDistanceToScreenCenter.x = element_pixel_center.x - screen_pixel_center.x; //ex. 576
		m_horizontalDriftMultiplier = m_absoluteDistanceToScreenCenter.x / maximum_pixel_height; //ex. 25/12 = 2.083333
		m_absoluteDistanceToScreenCenter.x /= MAX_WIDTH; //convert horizontal pixels to absolute units ex. 576/3840 = 0.15

		m_absoluteDistanceToScreenCenter.y = (element_pixel_center.y - screen_pixel_center.y) / MAX_HEIGHT; //ex. ???

		//Once the relative parameters have been set, resize the element
		resize(windowSize);
	}

	void convertAbsoluteCoordinatesToRelativeCoordinates(DirectX::XMFLOAT2 absolute_size)
	{
		//When the relative box class is created the user gives its size and location in units relative to the
		//size of the screen. For example a size of (0.1, 0.1) means that user wants to create a shape that's
		//1/10th the height and 1/10th the width of the screen and a location of (0.5, 0.5) means the
		//element will be have its center in the middle of the screen. To make sure that elements retain
		//their shape and location when the screen is resized, their height is locked to a ratio of the 
		//current screen size, and their width will be some constant multiplier of the height.
		//Since the element is sized in this manner, the absolute coordinates input by the user won't give the 
		//exact results that are desired, so this method is used to convert them into the relative
		//coordinates that will give the desired result.
		float desired_pixel_height = absolute_size.y * MAX_HEIGHT;
		float actual_pixel_height = absolute_size.y / (MAX_WIDTH + MAX_HEIGHT) * (2 * MAX_WIDTH * MAX_HEIGHT);
		
		float height_multiplier = desired_pixel_height / actual_pixel_height;
		float relative_height = absolute_size.y * height_multiplier;

		//Since the relative height was just changed, the relative Y location
		//must also be changed to reflect this. This is done by taking the absolute
		//distance between the center of the element and the
		//(horizontal) center of the screen, multiplying it by the new relative
		//height variable (which is normalized to the height of the entire screen
		//by dividing it by the current element height) and then subtracting this
		//value from the absolute midpoint of the screen. The line below shows
		//that this reduces down to a very simple equation, although, it was actually
		//pretty tricky to figure out.
		m_location.y = 0.5f - relative_height / m_size.y * (0.5f - m_location.y);
		m_size.y = relative_height;

		//Set the horizontal multiplier based on the original absolute coordinates
		float desired_pixel_width = absolute_size.x * MAX_WIDTH;
		m_horizontalSizeMultiplier = desired_pixel_width / desired_pixel_height;
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
		box_pixel_dimensions.x = m_horizontalSizeMultiplier * box_pixel_dimensions.y;

		float newPixelDistanceToHorizontal = m_horizontalDriftMultiplier * m_size.y / (windowSize.Width + windowSize.Height) * (2 * windowSize.Width * windowSize.Height); //ex. 523.256
		float newPixelDistanceToVertical = m_absoluteDistanceToScreenCenter.y / (windowSize.Width + windowSize.Height) * (2 * windowSize.Width * windowSize.Height); //ex. ???

		//TODO: The below four lines should be able to be combined into two and get rid of some division along the way
		float horizontal_relative_difference = newPixelDistanceToHorizontal / windowSize.Width - m_absoluteDistanceToScreenCenter.x; //ex. 523.256 / 3000 - 0.15 = 0.0244187
		float vertical_relative_difference = newPixelDistanceToVertical / windowSize.Height - m_absoluteDistanceToScreenCenter.y; //ex. ???
		
		pixel_center.x += horizontal_relative_difference * windowSize.Width; //i.e += 0.0244187 * 3000 --> += 73.256
		pixel_center.y += vertical_relative_difference * windowSize.Height; // i.e. ???

		m_shape.m_rectangle.left = pixel_center.x - box_pixel_dimensions.x / 2.0f;
		m_shape.m_rectangle.top = pixel_center.y - box_pixel_dimensions.y / 2.0f;
		m_shape.m_rectangle.right = pixel_center.x + box_pixel_dimensions.x / 2.0f;
		m_shape.m_rectangle.bottom = pixel_center.y + box_pixel_dimensions.y / 2.0f;
	}

	DirectX::XMFLOAT2 m_absoluteDistanceToScreenCenter; //Represents the absolute distance from the center of the element to the center of the screen when it's fully maximized

	//Width tracking variables
	float m_horizontalSizeMultiplier; //the ratio of the element's width to its height
	float m_horizontalDriftMultiplier; //the ratio of the horizontal distance from the center of the element to the center vertical axis and the elements height when the screen is maximized
};