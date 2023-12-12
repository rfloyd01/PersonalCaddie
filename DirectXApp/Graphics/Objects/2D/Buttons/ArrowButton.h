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

	ArrowButton() {} //empty default constructor

	virtual void setAbsoluteSize(DirectX::XMFLOAT2 size) override
	{
		//Since the sides of the arrows are tied to either the top or bottom of the central arrow,
		//resizing will cause them to shift a touch. Call the standard size setting method, and then
		//shift the sides of the arrows accordingly/
		DirectX::XMFLOAT2 leftArrowOriginalSize  = p_children[2]->getAbsoluteSize();
		DirectX::XMFLOAT2 rightArrowOriginalSize = p_children[3]->getAbsoluteSize();
		UIElement::setAbsoluteSize(size);

		DirectX::XMFLOAT2 leftArrowNewSize = p_children[2]->getAbsoluteSize();
		DirectX::XMFLOAT2 rightArrowNewSize = p_children[3]->getAbsoluteSize();

		DirectX::XMFLOAT2 leftArrowLocation = p_children[2]->getAbsoluteLocation();
		DirectX::XMFLOAT2 rightArrowLocation = p_children[3]->getAbsoluteLocation();

		p_children[2]->setAbsoluteLocation({ leftArrowLocation.x - (leftArrowOriginalSize.x - leftArrowNewSize.x) / 2.0f, leftArrowLocation.y + (leftArrowOriginalSize.y - leftArrowNewSize.y) / 2.0f });
		p_children[3]->setAbsoluteLocation({ rightArrowLocation.x - (rightArrowOriginalSize.x - rightArrowNewSize.x) / 2.0f, rightArrowLocation.y + (rightArrowOriginalSize.y - rightArrowNewSize.y) / 2.0f });
	}
};