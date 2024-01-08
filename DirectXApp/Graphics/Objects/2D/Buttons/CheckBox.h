#pragma once

#include "Button.h"
#include "Graphics/Objects/2D/BasicElements/Line.h"

/*
The check box is a form of button that either holds the
value true or false. When the value is true an 'X' will
be visible inside the button. If not then the button will
just be a blank white box. Check Boxes are squares by default
*/
class CheckBox : public Button
{
public:
	CheckBox(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size) :
		Button(windowSize, location, size, true, UIColor::White, UIColor::Black, UIColor::White)
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

		//Make the children elements of the underlying box object
		//invisible so only the outermost outline is visible
		auto button_children = p_children[0]->getChildren();
		button_children[0]->updateState(UIElementState::Invisible);
		button_children[1]->updateState(UIElementState::Invisible);

		//Force the X to be a square shape
		//setAbsoluteSize(size);
	}

	CheckBox() {} //empty default constructor

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