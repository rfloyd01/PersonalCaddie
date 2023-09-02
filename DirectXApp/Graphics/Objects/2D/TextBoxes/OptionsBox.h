#pragma once

#include "ScrollingTextBox.h"

//The options box is a variation of the scrolling text box. It's still a text box that
//can scroll with two buttons and a progress bar, however, the way that text is added
//and displayed is slightly different. In the options box the text isn't one continous
//block, each line represents a single option that can be selected. Hovering over any
//line with the mouse will cause the text to change color and clicking the text will
//emit the text string somewhere else.

//Unlike the standard scroll box which requires a foreground element to hide text that
//goes above the box, we dynamically change which option is the first in the box as 
//we scroll. By doing this we can achieve the illusion of scrolling without having to 
//physically move the text. Each turn of the scroll wheel or click of the buttons will
//goes the text to move up or down by a single line.

class OptionsBox : public ScrollingTextBox
{
public:
	OptionsBox(DirectX::XMFLOAT2 location, float height, std::wstring text, UIColor backgroundColor, winrt::Windows::Foundation::Size windowSize);

	//Depending on the type of text box being created text will be
	//added differently
	virtual uint32_t addText(std::wstring text) override;
	virtual UIElementState update(InputState* inputState) override;
	/*
	virtual void setState(UIElementState state) override;*/

protected:
	uint32_t addText(std::wstring text, winrt::Windows::Foundation::Size windowSize);
	virtual void resize(winrt::Windows::Foundation::Size windowSize) override;
	//void repositionElementText(winrt::Windows::Foundation::Size windowSize);

	virtual void initializeScrollProgressRectangle() override;
	void setOptionText(winrt::Windows::Foundation::Size windowSize);
	//void calculateScrollBarLocation();

	//bool checkHover(DirectX::XMFLOAT2 mousePosition);
	//virtual void onHover();

	virtual void onScrollUp();
	virtual void onScrollDown();

	//float m_fontSize;
	//float m_scrollIntensity, textPixelsPerScroll, rectanglePixelsPerScroll; //how far will the text move with each scroll
	//DirectX::XMFLOAT2 m_textStart; //the absolute starting position for text. This value will change as we scroll up or down in the box

	//bool scrollRectangleInitialized;
	//float scrollRectangleAbsoluteHeight, scrollRectangleAbsoluteLocation, scrollRectangleAbsoluteCeiling, scrollRectangleAbsoluteFloor;
	bool needTextResize;
	int topOption = 0, optionsDisplayed = 0; //keeps track of which option is currently at the top of the text box, and how many total options to display
	std::vector<std::wstring> m_options; //holds all of the options for the box in order, not just the options currently being displayed
};