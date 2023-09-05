#include "pch.h"
#include "HighlightableTextOverlayBasic.h"

HighlightableTextOverlayBasic::HighlightableTextOverlayBasic(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring message,
	float fontSize, std::vector<UIColor> colors, std::vector<unsigned long long> colorLocations, UITextJustification justification) :
	TextOverlayBasic(windowSize, location, size, message, fontSize, colors, colorLocations, justification)
{
	//Set the screen size dependent variables
	m_size = size;
	m_location = location;
	m_fontSize = fontSize;
}

void HighlightableTextOverlayBasic::onHover()
{
	//when hovering over the text, we swap the current color to the secondary color
	UIColor temp = m_text.colors[0];
	m_text.colors[0] = m_secondaryColor;
	m_secondaryColor = temp;
}

void HighlightableTextOverlayBasic::removeState(uint32_t state)
{
	//When the hover state is removed fwe change its color back
	if (state == UIElementStateBasic::Hovered)
	{
		onHover();
	}
	m_state ^= state;
}