#pragma once

#include "Button.h"
#include "Graphics/Objects/2D/BasicElements/Line.h"

//The arrow button is a button with either an up arrow or down arrow
//in the center of it. Typically used as a button in something like 
//a scroll box
class ArrowButton : public Button
{
public:
	ArrowButton(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, bool isInverted = false,
		bool isSquare = false, UIColor fillColor = UIColor::ButtonNotPressed, UIColor outlineColor = UIColor::Black, UIColor shadowColor = UIColor::DarkGray) :
		Button(windowSize, location, size, isSquare, fillColor, outlineColor, shadowColor)
	{
		//The arrow is made up of three separate lines, these all get added as children elements
		Line centerline(windowSize, {location.x, location.y - size.y / (float)4.0}, { location.x, location.y + size.y / (float)4.0 });
		p_children.push_back(std::make_shared<Line>(centerline));

		if (!isInverted)
		{
			//if isInverted is set to true it means we create a downwards facing arrow.
			Line upLeft(windowSize, { location.x, location.y - size.y / (float)4.0 }, { location.x - size.x / (float)8.0, location.y });
			Line upRight(windowSize, { location.x, location.y - size.y / (float)4.0 }, { location.x + size.x / (float)8.0, location.y });
			p_children.push_back(std::make_shared<Line>(upLeft));
			p_children.push_back(std::make_shared<Line>(upRight));
		}
		else
		{
			//if isInverted is set to true it means we create a downwards facing arrow.
			Line downLeft(windowSize, { location.x, location.y + size.y / (float)4.0 }, { location.x - size.x / (float)8.0, location.y });
			Line downRight(windowSize, { location.x, location.y + size.y / (float)4.0 }, { location.x + size.x / (float)8.0, location.y });
			p_children.push_back(std::make_shared<Line>(downLeft));
			p_children.push_back(std::make_shared<Line>(downRight));
		}
	}
};