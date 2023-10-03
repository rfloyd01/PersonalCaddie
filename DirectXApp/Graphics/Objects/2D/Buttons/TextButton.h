#pragma once

#include "Button.h"

//The text button, as its name implies, is a button with text
//inside of it
class TextButton : public Button
{
public:
	TextButton(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring text,
		bool isSquare = false, UIColor fillColor = UIColor::ButtonNotPressed, UIColor outlineColor = UIColor::Black, UIColor shadowColor = UIColor::DarkGray) :
		Button(windowSize, location, size, isSquare, fillColor, outlineColor, shadowColor)
	{
		//For now make the font size something random
		m_fontSize = 0.35 * m_size.y;

		//Create a text object from the given wstring
		m_text.textType = UITextType::ELEMENT_TEXT;
		m_text.message = text;
		m_text.justification = UITextJustification::CenterCenter;
		m_text.colors = { UIColor::Black };
		m_text.colorLocations = { 0, (unsigned int)text.length() };

		resize(windowSize); //Force a resize so the text is created properly
	}

	virtual void setState(uint32_t state) override
	{
		m_state = state;

		if (state & UIElementState::Disabled)
		{
			//turn the button text gray when it's disabled
			m_text.colors[0] = UIColor::Gray;
		}
	}

	void removeState(uint32_t state)
	{
		Button::removeState(state);

		//When enabling a button we change its text color to black
		if (state & UIElementState::Disabled)
		{
			m_state ^= UIElementState::Disabled;
			m_text.colors[0] = UIColor::Black;
		}
	}

	std::wstring getText()
	{
		//returns the text inside the button
		return m_text.message;
	}
};