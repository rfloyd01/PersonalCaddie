#pragma once

#include "Button.h"

//The text button, as its name implies, is a button with text
//inside of it
class TextButton : public Button
{
public:
	TextButton(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring text,
		bool isSquare = false, UIColor fillColor = UIColor::ButtonNotPressed, UIColor outlineColor = UIColor::Black, UIColor shadowColor = UIColor::DarkGray) :
		Button(windowSize, location, size, fillColor, outlineColor, shadowColor)
	{
		//Font size is expressed as a ratio of the overall height of the button.
		//For now just make it 35% of the button height
		m_fontSize = 0.35f;

		//Create a text object from the given wstring
		m_text.textType = UITextType::ELEMENT_TEXT;
		m_text.message = text;
		m_text.justification = UITextJustification::CenterCenter;
		m_text.colors = { UIColor::Black };
		m_text.colorLocations = { 0, (unsigned int)text.length() };

		resize(); //Force a resize so the text is created properly
	}

	TextButton() {}; //use the Button default constructor

	virtual void setState(uint32_t state) override
	{
		if (state & UIElementState::Disabled)
		{
			//turn the button text gray when it's disabled
			m_text.colors[0] = UIColor::Gray;
		}

		//Call the parent method to actually update the state
		UIElement::setState(state);
	}

	virtual void updateState(uint32_t state) override
	{
		if (state & UIElementState::Disabled)
		{
			//turn the button text gray when it's disabled
			m_text.colors[0] = UIColor::Gray;
		}

		//Call the parent method to actually update the state
		UIElement::setState(state);
	}

	void removeState(uint32_t state)
	{
		//When enabling a button we change its text color to black
		if (state & UIElementState::Disabled)
		{
			//m_state ^= UIElementState::Disabled;
			m_text.colors[0] = UIColor::Black;
		}

		//Call the parent method to actually update the state
		UIElement::removeState(state);
	}

	std::wstring getText()
	{
		//returns the text inside the button
		return m_text.message;
	}

	void updateText(std::wstring newText)
	{
		//changes the text inside the button
		m_text.message = newText;

		//Also update the color position vector to match the new length.
		m_text.colorLocations.back() = newText.length();
	}
};