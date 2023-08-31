#pragma once

#include "../UIElement.h"

//The most basic of all the UI Elements. This is just a blank white rectangle
//intended to have text written on top of it. This class can't be instantiated
//directly, it's used to make more complex text boxes

class TextOverlay : public UIElement, ITextBoxUI
{
public:
	TextOverlay(std::wstring const& text, std::vector<UIColor> const& colors, std::vector<unsigned long long> const& colorLocations, UITextType textType, winrt::Windows::Foundation::Size windowSize);
	TextOverlay(std::wstring const& text, std::vector<UIColor> const& colors, std::vector<unsigned long long> const& colorLocations,
		DirectX::XMFLOAT2 start, DirectX::XMFLOAT2 size, float fontSize, UITextType type, UITextJustification justification);

	virtual void addText(std::wstring text) override;
	virtual UIElementState update(InputState* inputState) override;

	UIText getText();

protected:
	virtual void resize(winrt::Windows::Foundation::Size windowSize) override;

	float m_fontSize;
};