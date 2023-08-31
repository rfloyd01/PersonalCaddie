#pragma once

#include "../UIElement.h"

//The most basic of all the UI Elements. This is just a blank white rectangle
//intended to have text written on top of it. This class can't be instantiated
//directly, it's used to make more complex text boxes

class ScrollingTextBox : public UIElement, ITextBoxUI, IScrollableUI
{
public:
	ScrollingTextBox(DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring text, UIColor backgroundColor, winrt::Windows::Foundation::Size windowSize);

	//Depending on the type of text box being created text will be
	//added differently
	virtual void addText(std::wstring text) override;
	virtual UIElementState update(InputState* inputState) override;

protected:
	virtual void resize(winrt::Windows::Foundation::Size windowSize) override;

	bool checkHover(DirectX::XMFLOAT2 mousePosition);
	virtual void onHover();

	virtual void onScrollUp();
	virtual void onScrollDown();

	float m_fontSize;
	float m_scrollIntensity, pixelsPerScroll; //how far will the text move with each scroll
	DirectX::XMFLOAT2   m_textStart; //the absolute starting position for text. This value will change as we scroll up or down in the box
};