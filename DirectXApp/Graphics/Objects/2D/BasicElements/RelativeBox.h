#pragma once

#include "Box.h"

#define MAX_HEIGHT 2160.0f
#define MAX_WIDTH 3840.0f

class RelativeBox : public Box
{
public:
	RelativeBox(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, UIColor color = UIColor::Black, UIShapeFillType fill = UIShapeFillType::Fill, bool isSquare = false) :
		Box(windowSize, location, size, color, fill)
	{
		//Before doing anything, convert the absolute coordinates entered for the size and location
		//parameters to the relative coordinates needed to correct screen placement.
		convertAbsoluteCoordinatesToRelativeCoordinates();

		//Using the maximum window sizes, calculate the relative distance that the right edge
		//of the box would be from a vertical centerline (in terms of percentage of window length)
		//and calculate the relative distance that the top edge of the box would be from a 
		//horizontal center lin (in terms of percentage of window height).
		DirectX::XMFLOAT2 element_pixel_center = { MAX_WIDTH * m_location.x, MAX_HEIGHT * m_location.y };
		DirectX::XMFLOAT2 screen_pixel_center = { MAX_WIDTH * 0.5f, MAX_HEIGHT * 0.5f };

		//Calculate the absolute distance between the element center and screen center when 
		//the screen size is at its maximum. A horizontal drift multiplier is also calculated,
		//which is the ratio of the horizontal pixel distance from the center of the element to the
		//center of the screen and the pixel distance of the element's height.
		float maximum_pixel_height = m_size.y / (MAX_WIDTH + MAX_HEIGHT) * (2 * MAX_WIDTH * MAX_HEIGHT);
		m_absoluteDistanceToScreenCenter.x = element_pixel_center.x - screen_pixel_center.x;
		m_horizontalDriftMultiplier = m_absoluteDistanceToScreenCenter.x / maximum_pixel_height;
		
		m_absoluteDistanceToScreenCenter.x /= MAX_WIDTH; //convert horizontal pixels to absolute units
		m_absoluteDistanceToScreenCenter.y = (element_pixel_center.y - screen_pixel_center.y) / MAX_HEIGHT;

		//Once the relative parameters have been set, resize the element
		resize();
	}

	void convertAbsoluteCoordinatesToRelativeCoordinates()
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
		float desired_pixel_height = m_size.y * MAX_HEIGHT;
		float actual_pixel_height = m_size.y / (MAX_WIDTH + MAX_HEIGHT) * (2 * MAX_WIDTH * MAX_HEIGHT);
		
		float height_multiplier = desired_pixel_height / actual_pixel_height;
		float relative_height = m_size.y * height_multiplier;

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
		float desired_pixel_width = m_size.x * MAX_WIDTH;
		m_horizontalSizeMultiplier = desired_pixel_width / desired_pixel_height;
	}

	virtual void resize() override
	{
		//First, calculate the center location of the element and its height and width in pixels
		DirectX::XMFLOAT2 element_pixel_center = { m_screenSize->Width * m_location.x, m_screenSize->Height * m_location.y };
		DirectX::XMFLOAT2 element_pixel_dimensions;

		//The height of the element is calculated by using a weighted ratio of the screen's current
		//height to the screen's current width, with the screen height getting the heigher weigtht.
		//The pixel width of the element is just a simple multiple of the height after it's calculated.
		element_pixel_dimensions.y = m_size.y / (m_screenSize->Width + m_screenSize->Height) * (2 * m_screenSize->Width * m_screenSize->Height);
		element_pixel_dimensions.x = m_horizontalSizeMultiplier * element_pixel_dimensions.y;

		//Second, account for the "drift" effect of the center of the element. This effect occurs
		//since the dimensions are co-dependent on the height AND width of the current screen as opposed
		//to the element height only being dependent on screen height, and the element width only 
		//being dependent on screen width.
		float newPixelDistanceToHorizontal = m_horizontalDriftMultiplier * element_pixel_dimensions.y;
		float newPixelDistanceToVertical = m_absoluteDistanceToScreenCenter.y * element_pixel_dimensions.y / m_size.y;

		element_pixel_center.x += newPixelDistanceToHorizontal - m_absoluteDistanceToScreenCenter.x * m_screenSize->Width;
		element_pixel_center.y += newPixelDistanceToVertical - m_absoluteDistanceToScreenCenter.y * m_screenSize->Height;

		//Once the center drift has been accounted for, size the m_shape rectangle so that
		//the new pixel center is the exact center of the element.
		m_shape.m_rectangle.left = element_pixel_center.x - element_pixel_dimensions.x / 2.0f;
		m_shape.m_rectangle.top = element_pixel_center.y - element_pixel_dimensions.y / 2.0f;
		m_shape.m_rectangle.right = element_pixel_center.x + element_pixel_dimensions.x / 2.0f;
		m_shape.m_rectangle.bottom = element_pixel_center.y + element_pixel_dimensions.y / 2.0f;
	}

	DirectX::XMFLOAT2 m_absoluteDistanceToScreenCenter; //Represents the absolute distance from the center of the element to the center of the screen when it's fully maximized

	//Width tracking variables
	float m_horizontalSizeMultiplier; //the ratio of the element's width to its height
	float m_horizontalDriftMultiplier; //the ratio of the horizontal distance from the center of the element to the center vertical axis and the elements height when the screen is maximized
};