#pragma once

#include "TextOverlay.h"

//This is the same as the text overlay class, however, it has the capability
//to change text when it's hovered over. This class is meant for small uniform
//text, not large groups with potentially different fonts, colros, etc.

class HighlightableTextOverlay : public TextOverlay, IHoverableUI
{
public:
	HighlightableTextOverlay(std::wstring const& text, std::vector<UIColor> const& colors, std::vector<unsigned long long> const& colorLocations, UITextType textType, winrt::Windows::Foundation::Size windowSize);
	HighlightableTextOverlay(std::wstring const& text, std::vector<UIColor> const& colors, std::vector<unsigned long long> const& colorLocations,
		DirectX::XMFLOAT2 start, DirectX::XMFLOAT2 size, float fontSize, UITextType type, UITextJustification justification);

	virtual UIElementState update(InputState* inputState) override;
	void updateSecondaryColor(UIColor color) { m_secondaryColor = color; }

	void updateLocation(DirectX::XMFLOAT2 location) { m_location = location; }
	void updateSize(DirectX::XMFLOAT2 size) { m_size = size; }
	void updateFontSize(float font_size) { m_fontSize = font_size; }

protected:
	//virtual bool checkHover(DirectX::XMFLOAT2 mousePosition) override; //checks to see if the element is being hovered over
	bool checkHover(DirectX::XMFLOAT2 mousePosition); //checks to see if the element is being hovered over
	virtual void onHover() override;
	void removeHover();

	UIColor m_secondaryColor = UIColor::White; //Default to a white secondary color
};