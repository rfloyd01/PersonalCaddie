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
	m_isHoverable = true;
	m_primaryColor = colors[0];
}

void HighlightableTextOverlayBasic::select()
{
	m_state |= UIElementStateBasic::Selected;
	m_text.colors[0] = m_tertiaryColor;
}

void HighlightableTextOverlayBasic::onHover()
{
	//when hovering over the text, we swap the current color to the secondary color
	//unless the text is currently selected (in which case it stays the same).
	if (!(m_state & UIElementStateBasic::Selected))
	{
		m_text.colors[0] = m_secondaryColor;
	}
}

void HighlightableTextOverlayBasic::removeState(uint32_t state)
{
	if (state == UIElementStateBasic::Hovered)
	{
		//When the hover state is removed we change back to the primary
		//color (unless the text is currently in the selected state)
		if (!(m_state & UIElementStateBasic::Selected))
		{
			m_text.colors[0] = m_primaryColor;
		}
	}
	else if (state == UIElementStateBasic::Selected)
	{
		//Switch back to either the primary or secondary color 
		//depending on whether or not the text is currently being hovered.
		if (m_state & UIElementStateBasic::Hovered) m_text.colors[0] = m_secondaryColor;
		else m_text.colors[0] = m_primaryColor;
	}

	m_state ^= state;
}