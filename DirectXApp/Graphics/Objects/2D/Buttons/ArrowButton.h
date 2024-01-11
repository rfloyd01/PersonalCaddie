#pragma once

#include "Button.h"
#include "Graphics/Objects/2D/BasicElements/Line.h"

//The arrow button is a button with either an up arrow or down arrow
//in the center of it. Typically used as a button in something like 
//a scroll box
class ArrowButton : public Button
{
public:
	ArrowButton(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, bool isInverted = false,
		float shadowSize = 8.0f, UIColor fillColor = UIColor::ButtonNotPressed, UIColor outlineColor = UIColor::Black, UIColor shadowColor = UIColor::DarkGray) :
		Button(windowSize, location, size, fillColor, outlineColor, shadowColor, shadowSize)
	{
		//The arrow is made up of three separate lines, these all get added as children elements
		Line centerline(windowSize, {location.x, location.y - size.y / 4.0f}, { location.x, location.y + size.y / 4.0f });
		p_children.push_back(std::make_shared<Line>(centerline));

		m_isInverted = isInverted;

		if (!m_isInverted)
		{
			//if isInverted is set to true it means we create a downwards facing arrow.
			Line upRight(windowSize, { location.x, location.y - size.y / 4.0f }, { location.x - size.x / 8.0f, location.y });
			Line upLeft(windowSize, { location.x, location.y - size.y / 4.0f }, { location.x + size.x / 8.0f, location.y });
			p_children.push_back(std::make_shared<Line>(upRight));
			p_children.push_back(std::make_shared<Line>(upLeft));
		}
		else
		{
			//if isInverted is set to true it means we create a downwards facing arrow.
			Line downRight(windowSize, { location.x, location.y + size.y / 4.0f }, { location.x - size.x / 8.0f, location.y });
			Line downLeft(windowSize, { location.x, location.y + size.y / 4.0f }, { location.x + size.x / 8.0f, location.y });
			p_children.push_back(std::make_shared<Line>(downRight));
			p_children.push_back(std::make_shared<Line>(downLeft));
		}
	}

	ArrowButton() {} //empty default constructor

	virtual void setChildrenAbsoluteSize(DirectX::XMFLOAT2 size) override
	{
		//The Arrow Button has three line child elements. One of the lines passes straight
		//through the center of the button so nothing extra needs to be done to it. The
		//other two lines are offset though so will need to be repositioned upon
		//resizing.
		Button::setChildrenAbsoluteSize(size);
		p_children[1]->setAbsoluteSize({ 0.0f, size.y / 2.0f });

		//The other two lines need to shrink two match the new button size,
		//and also be shifted towards or away from the center.
		int inversionMultiplier = 1 - 2 * m_isInverted;
		p_children[2]->setAbsoluteSize({ -size.x / 8.0f, inversionMultiplier * size.y / 4.0f });
		p_children[3]->setAbsoluteSize({ size.x / 8.0f, inversionMultiplier * size.y / 4.0f });

		auto absoluteLocation = getAbsoluteLocation();
		if (!m_isInverted)
		{
			p_children[2]->setAbsoluteLocation({ absoluteLocation.x - size.x / 16.0f, absoluteLocation.y - size.y / 8.0f });
			p_children[3]->setAbsoluteLocation({ absoluteLocation.x + size.x / 16.0f, absoluteLocation.y - size.y / 8.0f });
		}
		else
		{
			p_children[2]->setAbsoluteLocation({ absoluteLocation.x - size.x / 16.0f, absoluteLocation.y + size.y / 8.0f });
			p_children[3]->setAbsoluteLocation({ absoluteLocation.x + size.x / 16.0f, absoluteLocation.y + size.y / 8.0f });
		}
	}

private:
	bool m_isInverted;
};