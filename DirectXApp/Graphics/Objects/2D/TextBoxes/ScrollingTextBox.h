#pragma once

#include "../UIElement.h"

//The most basic of all the UI Elements. This is just a blank white rectangle
//intended to have text written on top of it. This class can't be instantiated
//directly, it's used to make more complex text boxes

class ScrollingTextBox : public UIElement, ITextBoxUI, IScrollableUI
{
public:
	ScrollingTextBox(DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring text, winrt::Windows::Foundation::Size windowSize);

	//Depending on the type of text box being created text will be
	//added differently
	virtual void addText(std::wstring text) override;
	virtual UIElementState update(DirectX::XMFLOAT2 mousePosition, bool mouseClick) override;

protected:
	virtual void resize(winrt::Windows::Foundation::Size windowSize) override;

	virtual void onScrollUp();
	virtual void onScrollDown();

	float m_fontSize;
};