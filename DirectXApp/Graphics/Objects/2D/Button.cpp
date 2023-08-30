#include "pch.h"
#include "Button.h"

Button::Button(DirectX::XMFLOAT2 location, std::wstring text)
{
	//A button is made up of three different rectangles. There are two outlines (one of which is
	//offset from the first by a little to give the impression of a shadow) and a background color.

	//All buttons get resized with the main application window, so all dimensions and locations
	//are given in terms of a relative window location (i.e. button length = 10% of window length).

	//The size and state for all new buttons will be the same, the location of the button is create
	//when instatiated though.

	//The order of the objects is: outline1, outline2, fill
	m_dimensions = { {0.10, 0.10}, {0.10 - 0.004, 0.10 - 0.004}, {0.10, 0.10} };
	m_states = { MenuObjectState::PassiveOutline, MenuObjectState::PassiveOutline, MenuObjectState::NotPressed };
	m_locations = { {location.x, location.y}, {location.x + (float)0.002, location.y + (float)0.002}, {location.x, location.y} };
	m_text = text;
}

bool Button::inSpace(DirectX::XMFLOAT2 const & mousePosition, winrt::Windows::Foundation::Size windowSize)
{
	//For the mouse to be considered "in" the button it needs to be within the first outline

	//TODO: need to pass in the current window size as the button dimensions are only ratios of the window
	if ((mousePosition.x >= windowSize.Width * (m_locations[0].x - m_dimensions[0].x / 2.0)) &&
		(mousePosition.x <= windowSize.Width * (m_locations[0].x + m_dimensions[0].x / 2.0)) &&
		(mousePosition.y >= windowSize.Height * (m_locations[0].y - m_dimensions[0].y / 2.0)) &&
		(mousePosition.y <= windowSize.Height * (m_locations[0].y + m_dimensions[0].y / 2.0)))
	{
		return true;
	}
	else
	{
		return false;
	}
}

MenuObjectState Button::update(DirectX::XMFLOAT2 mousePosition, bool mouseClick, winrt::Windows::Foundation::Size windowSize)
{
	//If a button is clicked we return a value of true which initiates two things. First, the button will change it's 
	//color to reflect that it's being pressed (this color change will be put into a timer and reverted back when the timer
	//expires). It will also send a signale to the current mode to see how to handle the button press.

	if (this->inSpace(mousePosition, windowSize))
	{
		if (mouseClick)
		{
			m_states[2] = MenuObjectState::Pressed;
			return MenuObjectState::Pressed;
		}
	}

	return MenuObjectState::None; //there's nothing to update
}

MenuObjectState Button::getReleventState()
{
	//the relevent state for the button is the third state which let's us know if the button is 
	//being pressed or not
	return m_states[2];
}

void Button::setReleventState(MenuObjectState state)
{
	//the relevent state for the button is the third state which let's us know if the button is 
	//being pressed or not
	m_states[2] = state;
}