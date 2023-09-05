#pragma once

#include "Graphics/Objects/2D/BasicElements/TextOverlayBasic.h"

//This is the same as the text overlay class, however, it has the capability
//to change text when it's hovered over. This class is meant for small uniform
//text, not large groups with potentially different fonts, colros, etc.

class HighlightableTextOverlayBasic : public TextOverlayBasic, IHoverableUI
{
public:
	HighlightableTextOverlayBasic(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring message,
		float fontSize, std::vector<UIColor> colors, std::vector<unsigned long long> colorLocations, UITextJustification justification);

	virtual void removeState(uint32_t state) override;

protected:
	//virtual bool checkHover(DirectX::XMFLOAT2 mousePosition) override; //checks to see if the element is being hovered over
	virtual void onHover() override;

	UIColor m_secondaryColor = UIColor::White; //Default to a white secondary color
};