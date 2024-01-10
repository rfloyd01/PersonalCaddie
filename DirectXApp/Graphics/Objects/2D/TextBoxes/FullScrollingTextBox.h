#pragma once

#include "Graphics/Objects/2D/TextBoxes/TextBox.h"

//The full scrolling text works a little bit differently from the partial
//scrolling text box. Every time you scroll up or down the text will move
//by an amount equal to the height of one line of text. Since there's no 
//need to show any partial line, every line of text becomes its own UI
//Element. As we scroll, lines that are no longer in the box become invisible.
//With this approach there's no need for a box at the top of the scroll box
//to hide any text.

//There are two options that can be selected for the FullScrollingTextBox.
//The first option lets you select highlightable text for the box, which is
//good for something like a drop down box. The second option mandates whether
//the size of the box is determined by the user, or by the longest line of text.
//The longest line of text option is also good for something like a drop down
//menu

class FullScrollingTextBox : public UIElement, IScrollableUI, ITextDimensionsUI
{
public:
	FullScrollingTextBox(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring message, float fontSize,
		bool highlightableText = true, bool dynamicSize = true,
		UITextJustification justification = UITextJustification::UpperLeft, UIColor textFillColor = UIColor::White, bool isSquare = false,  UIColor outlineColor = UIColor::Black, UIColor shadowColor = UIColor::DarkGray);

	FullScrollingTextBox() {} //empty default constructor

	void addText(std::wstring text, bool highlightable = false, bool existingText = true);
	void clearText();

	virtual uint32_t update(InputState* inputState) override;
	virtual std::vector<UIText*> setTextDimension() override;

	virtual void setAbsoluteSize(DirectX::XMFLOAT2 size) override;
	virtual void repositionText() override;

	std::wstring getLastSelectedText() { return m_lastSelectedText; }
	void setLastSelectedText(std::wstring text) { m_lastSelectedText = text; } //TODO: May want to look at all text overlays in the child array and confirm the given text actually exists

protected:
	float getCurrentTextStartingHeight();

	virtual void onScrollUp() override;
	virtual void onScrollDown() override;
	void calcualteScrollBarLocation();

	float m_buttonHeight; //the height of the scroll buttons relative to the height of the text box
	float m_buttonRatio = 0.125f; //the ratio of the buttons' height to the overall height of the scroll box
	bool m_highlightableText;
	bool m_dynamicSize;

	int m_topText; //Represent which line of text is currently at the top of the scroll box
	int m_displayedText; //Represents how many lines of text are currently displayed in the scroll box
	int m_clickedTextIndex; //Used to make sure the mouse is released on the same option that gets clicked
	std::wstring m_lastSelectedText;
};