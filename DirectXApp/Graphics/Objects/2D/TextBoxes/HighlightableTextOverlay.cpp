#include "pch.h"
#include "HighlightableTextOverlay.h"

HighlightableTextOverlay::HighlightableTextOverlay(std::wstring const& text, std::vector<UIColor> const& colors, std::vector<unsigned long long> const& colorLocations, UITextType textType, winrt::Windows::Foundation::Size windowSize) :
	TextOverlay(text, colors, colorLocations, textType, windowSize)
{

}

HighlightableTextOverlay::HighlightableTextOverlay(std::wstring const& text, std::vector<UIColor> const& colors, std::vector<unsigned long long> const& colorLocations,
	DirectX::XMFLOAT2 start, DirectX::XMFLOAT2 size, float fontSize, UITextType type, UITextJustification justification) : TextOverlay(text, colors, colorLocations, start, size, fontSize, type, justification)
{

}

UIElementState HighlightableTextOverlay::update(InputState* inputState)
{
	bool hover = checkHover(inputState->mousePosition);
	if (hover && m_state == UIElementState::Idle) onHover();
	else if (!hover && m_state == UIElementState::Hovered) removeHover();
	return UIElementState::Idle;
}

bool HighlightableTextOverlay::checkHover(DirectX::XMFLOAT2 mousePosition)
{
	//check to see if the mouse is anywhere in the text overlay box
	if (m_textOverlay.size() == 0) return false; //can't be hovering over text that isn't there!

	//For the mouse to be considered "in" the button it needs to be within the outline of it, not the shadow
	if ((mousePosition.x >= m_textOverlay[0].startLocation.x) &&
		(mousePosition.x <= m_textOverlay[0].startLocation.x + m_textOverlay[0].renderArea.x) &&
		(mousePosition.y >= m_textOverlay[0].startLocation.y) &&
		(mousePosition.y <= m_textOverlay[0].startLocation.y + m_textOverlay[0].renderArea.y))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void HighlightableTextOverlay::onHover()
{
	//We change the color of the text to the secondary color to show that we're hovering over it.
	UIColor temp = m_textOverlay[0].colors[0];
	m_textOverlay[0].colors[0] = m_secondaryColor;
	m_secondaryColor = temp;
	m_state = UIElementState::Hovered;
}

void HighlightableTextOverlay::removeHover()
{
	//We change the color of the text back to the primary color to show that we're hovering over it.
	UIColor temp = m_textOverlay[0].colors[0];
	m_textOverlay[0].colors[0] = m_secondaryColor;
	m_secondaryColor = temp;
	m_state = UIElementState::Idle;
}