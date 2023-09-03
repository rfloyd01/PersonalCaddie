#pragma once

#include "Button.h"
#include "Graphics/Objects/2D/BasicElements/Line.h"

//The arrow button is a button with either an up arrow or down arrow
//in the center of it. Typically used as a button in something like 
//a scroll box
class ArrowButton : public Button
{
public:
	ArrowButton(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size,
		bool isSquare = false, UIColor fillColor = UIColor::ButtonNotPressed, UIColor outlineColor = UIColor::Black, UIColor shadowColor = UIColor::DarkGray) :
		Button(windowSize, location, size, isSquare, fillColor, outlineColor, shadowColor)
	{
		//The arrow is made up of three separate lines, these all get added is children elements
		//Line(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 firstPointLlocation, DirectX::XMFLOAT2 secondPointLlocation, UIColor color = UIColor::Black);
		Line centerline(windowSize, {location.x, location.y - size.y / (float)4.0}, { location.x, location.y + size.y / (float)4.0 });
		p_children.push_back(std::make_shared<Line>(centerline));
		resize(windowSize);
	}
};