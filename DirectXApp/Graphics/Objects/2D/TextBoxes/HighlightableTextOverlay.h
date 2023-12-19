#pragma once

#include "Graphics/Objects/2D/BasicElements/TextOverlay.h"

//This is the same as the text overlay class, however, it has the capability
//to change text when it's hovered over. This class is meant for small uniform
//text, not large groups with potentially different fonts, colros, etc.

class HighlightableTextOverlay : public TextOverlay, IHoverableUI
{
public:
	HighlightableTextOverlay(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring message,
		float fontSize, std::vector<UIColor> colors, std::vector<unsigned long long> colorLocations, UITextJustification justification);

	HighlightableTextOverlay() {} //empty default constructor

	virtual void removeState(uint32_t state) override;
	void updateSecondaryColor(UIColor color) { m_secondaryColor = color; }

	void select();

protected:
	//virtual bool checkHover(DirectX::XMFLOAT2 mousePosition) override; //checks to see if the element is being hovered over
	virtual void onHover() override;

	UIColor m_primaryColor = UIColor::Black; //This is the default text color
	UIColor m_secondaryColor = UIColor::Blue; //This is the text color when the Text overlay is being hovered over by the mouse
	UIColor m_tertiaryColor = UIColor::Yellow; //Some UI Elements (but not all) allow for the selection of text. This is the selected text color
};