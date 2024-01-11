#pragma once

#include "Button.h"
#include "Graphics/Objects/2D/BasicElements/Line.h"

/*
The check box is a form of button that either holds the
value true or false. When the value is true an 'X' will
be visible inside the button. If not then the button will
just be a blank white box. The main element is a button
with a shadow width of 0px, making it just appear to be 
a plain white box with a thin black outline.
*/
class CheckBox : public Button
{
public:
	CheckBox(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size) :
		Button(windowSize, location, size, UIColor::White, UIColor::Black, UIColor::White, 0.0f)
	{
		//The X is made up of two lines. One from the bottom left corner to the top right of the button, and the other from the top
		//left to the bottom right
		Line diagonal_one(windowSize, {location.x - size.x / 2.0f, location.y + size.y / 2.0f}, { location.x + size.x / 2.0f, location.y - size.y / 2.0f });
		Line diagonal_two(windowSize, {location.x - size.x / 2.0f, location.y - size.y / 2.0f }, { location.x + size.x / 2.0f, location.y + size.y / 2.0f });

		//By default checked boxes aren't checked
		m_isChecked = false;
		diagonal_one.updateState(UIElementState::Invisible);
		diagonal_two.updateState(UIElementState::Invisible);

		p_children.push_back(std::make_shared<Line>(diagonal_one));
		p_children.push_back(std::make_shared<Line>(diagonal_two));
	}

	CheckBox() {} //empty default constructor

	virtual void setChildrenAbsoluteSize(DirectX::XMFLOAT2 size) override
	{
		//The Check Box is a white button with two lines that cross
		//through the center. Since all elements have the same center
		//then nothing special needs to be done here except setting
		//the same absolute size for everything
		Button::setChildrenAbsoluteSize(size);
		p_children[1]->setAbsoluteSize({ size.x, size.y });
		p_children[2]->setAbsoluteSize({ -size.x, size.y });
	}

	virtual void onMouseClick() override
	{
		//Unlike a standard button we don't change the background color
		//when clicking a check box. Override the button's onMouseClick()
		//method and do nothing
	}

	virtual void onMouseRelease() override
	{
		//Toggle the m_isChecked boolean and either add or remove
		//the X in the center of the button
		if (m_isChecked)
		{
			p_children[1]->updateState(UIElementState::Invisible);
			p_children[2]->updateState(UIElementState::Invisible);
		}
		else
		{
			p_children[1]->removeState(UIElementState::Invisible);
			p_children[2]->removeState(UIElementState::Invisible);
		}

		m_isChecked = !m_isChecked;
	}

private:
	bool m_isChecked; //check boxes aren't checked by default
};