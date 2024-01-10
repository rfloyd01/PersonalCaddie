#include "pch.h"
#include "HighlightableTextOverlay.h"

HighlightableTextOverlay::HighlightableTextOverlay(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring message,
	float fontSize, std::vector<UIColor> colors, std::vector<unsigned long long> colorLocations, UITextJustification justification, bool useAbsolute) :
	TextOverlay(windowSize, location, size, message, fontSize, colors, colorLocations, justification, useAbsolute)
{
	m_isHoverable = true;
	m_primaryColor = colors[0];
}

void HighlightableTextOverlay::select()
{
	m_state |= UIElementState::Selected;
	m_text.colors[0] = m_tertiaryColor;
}

void HighlightableTextOverlay::onHover()
{
	//when hovering over the text, we swap the current color to the secondary color
	//unless the text is currently selected (in which case it stays the same).
	if (!(m_state & UIElementState::Selected))
	{
		m_text.colors[0] = m_secondaryColor;
	}
}

void HighlightableTextOverlay::removeState(uint32_t state)
{
	if (state == UIElementState::Hovered)
	{
		//When the hover state is removed we change back to the primary
		//color (unless the text is currently in the selected state)
		if (!(m_state & UIElementState::Selected))
		{
			m_text.colors[0] = m_primaryColor;
		}
	}
	else if (state == UIElementState::Selected)
	{
		//Switch back to either the primary or secondary color 
		//depending on whether or not the text is currently being hovered.
		if (m_state & UIElementState::Hovered) m_text.colors[0] = m_secondaryColor;
		else m_text.colors[0] = m_primaryColor;
	}

	m_state ^= state;
}