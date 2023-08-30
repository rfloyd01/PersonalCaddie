#include "pch.h"
#include "UIButton.h"

UIButton::UIButton(DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, winrt::Windows::Foundation::Size windowSize, std::wstring text)
{
	//A button is one of the basic UI Elements and therefore has no children. It's made up of three different
	//rectangles. There are two outlines (one of which is offset from the first by a little to give the
	//impression of a shadow) and a background color.

	//Buttons are created with a size, a location, and some optional text to go inside of them
	m_location = location; //represents the center of the button
	m_size = size;
	m_state = UIElementState::Idle; //buttons don't start in the clicked position
	m_fontSize = 0.33;  //button text size as a factor of the button height, if present

	//Create the shapes and add them to the backgroundshapes vector, just use generic coordinates
	//for the rectangles as they will get overriden by the resize() methhod. The render order for
	//the button is shadow, fill and then the outline
	m_backgroundShapes.push_back({ {1, 1, 1, 1}, UIShapeColor::Black, UIShapeFillType::NoFill });
	m_backgroundShapes.push_back({ {1, 1, 1, 1}, UIShapeColor::UnpressedButton, UIShapeFillType::Fill });
	m_backgroundShapes.push_back({ {1, 1, 1, 1}, UIShapeColor::Black, UIShapeFillType::NoFill });

	//Also add text if it's present
	if (text != L"")
	{
		//any variables dependent on windowSize will get updated in the resize() method
		UIText buttonText(text, 1, { 1, 1 }, { 1, 1 }, { UITextColor::Black }, { 0, (unsigned int)text.length() },
			UITextType::ELEMENT_TEXT, UITextJustification::CenterCenter);
		m_elementText.push_back(buttonText);
	}

	resize(windowSize);
}

void UIButton::resize(winrt::Windows::Foundation::Size windowSize)
{
	if (m_backgroundShapes.size() == 0) return; //only resize if there are actually elements to resize!

	D2D1_RECT_F outline_rectangle = {
		windowSize.Width * (m_location.x - m_size.x / 2.0),
		windowSize.Height * (m_location.y - m_size.y / 2.0),
		windowSize.Width * (m_location.x + m_size.x / 2.0),
		windowSize.Height * (m_location.y + m_size.y / 2.0)
	};

	//The shadow shape comes first. It is offset 1 pixel down and to the right of the main outline
	m_backgroundShapes[0].m_rectangle.left = outline_rectangle.left + 1;
	m_backgroundShapes[0].m_rectangle.top = outline_rectangle.top + 1;
	m_backgroundShapes[0].m_rectangle.right = outline_rectangle.right + 1;
	m_backgroundShapes[0].m_rectangle.bottom = outline_rectangle.bottom + 1;

	//Then comes the button fill shape
	m_backgroundShapes[1].m_rectangle.left = outline_rectangle.left;
	m_backgroundShapes[1].m_rectangle.top = outline_rectangle.top;
	m_backgroundShapes[1].m_rectangle.right = outline_rectangle.right;
	m_backgroundShapes[1].m_rectangle.bottom = outline_rectangle.bottom;

	//And finally the button outline
	m_backgroundShapes[2].m_rectangle.left = outline_rectangle.left;
	m_backgroundShapes[2].m_rectangle.top = outline_rectangle.top;
	m_backgroundShapes[2].m_rectangle.right = outline_rectangle.right;
	m_backgroundShapes[2].m_rectangle.bottom = outline_rectangle.bottom;

	//If there's any text in the button it gets resized as well
	if (m_elementText.size() != 0)
	{
		//The text render area and font are sized off of the button's fill area
		m_elementText[0].startLocation = { outline_rectangle.left,  outline_rectangle.top };
		m_elementText[0].renderArea = { outline_rectangle.right - outline_rectangle.left,  outline_rectangle.bottom - outline_rectangle.top };
		m_elementText[0].fontSize = m_fontSize * m_elementText[0].renderArea.y;
	}
}

UIElementState UIButton::update(DirectX::XMFLOAT2 mousePosition, bool mouseClick)
{
	//if we're hovering over the button and the mouseClick bool is true, then we update the
	//buttons state. Check mouseClick first so the checkHover method is only 
	//invoked when necessary

	//To make sure we don't keep clicking the button on and off with a single press, 
	//we only return the clicked state if the mouseClick bool is true
	if (mouseClick && checkHover(mousePosition))
	{
		onClick();
		return UIElementState::Clicked;
	}
	else return UIElementState::Idle;
}

void UIButton::updateButtonText(std::wstring text)
{
	//Change the text inside the button. This won't change the font, size, color, etc.
	//Just the text will change.
	if (m_backgroundShapes.size() == 0) return; //only update text if the button is present

	if (m_elementText.size() == 0)
	{
		//There isn't any text yet so we need to create new text. Size variables are dependent on the 
		//size of the button fill
		auto button_fill = &m_backgroundShapes[1];
		DirectX::XMFLOAT2 startLocation = { button_fill->m_rectangle.left , button_fill->m_rectangle.top };
		DirectX::XMFLOAT2 renderArea = { button_fill->m_rectangle.right - button_fill->m_rectangle.left , button_fill->m_rectangle.bottom - button_fill->m_rectangle.top };
		float font_size = m_fontSize * renderArea.y;

		UIText buttonText(text, font_size, startLocation, renderArea, { UITextColor::Black }, { 0, (unsigned int)text.length() },
			UITextType::ELEMENT_TEXT, UITextJustification::CenterCenter);
		m_elementText.push_back(buttonText);
	}
	else if (text == L"")
	{
		//passing in an empty string will remove element text
		m_elementText.clear();
	}
	else
	{
		//simply uptate the existing text
		m_elementText[0].message = text;
		m_elementText[0].colorLocations.back() = text.length(); //make sure the existing color applies to the length of the new message
	}
}

bool UIButton::checkHover(DirectX::XMFLOAT2 mousePosition)
{
	if (m_backgroundShapes.size() == 0) return false; //can't be hovering over the button if it isn't there!

	//For the mouse to be considered "in" the button it needs to be within the outline of it, not the shadow
	if ((mousePosition.x >= m_backgroundShapes[2].m_rectangle.left) &&
		(mousePosition.x <= m_backgroundShapes[2].m_rectangle.right) &&
		(mousePosition.y >= m_backgroundShapes[2].m_rectangle.top) &&
		(mousePosition.y <= m_backgroundShapes[2].m_rectangle.bottom))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void UIButton::onHover()
{
	//We don't actually do anything when hovering over a button, although in the future
	//I may change the color of the button when hovering over it. Currently this method
	//doesn't get called but needs to be here as it's overriding a pure virtual function
}

void UIButton::onClick()
{
	setState(UIElementState::Clicked);
}

void UIButton::setState(UIElementState state)
{
	//When changing the state of the button, we also change its background color
	if (state == UIElementState::Idle) m_backgroundShapes[1].m_color = UIShapeColor::UnpressedButton;
	else if (state == UIElementState::Clicked) m_backgroundShapes[1].m_color = UIShapeColor::PressedButton;
	m_state = state;
}