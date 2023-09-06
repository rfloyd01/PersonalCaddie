#pragma once

#include "Graphics/Objects/2D/TextBoxes/TextBox.h"

//The partial scrolling text box is a text box that only has a single text
//element, and can scroll in such a way that the text can be partially
//clipped (i.e. half a line of text can be showing). This element is useful 
//when there's a lot to say and not a large area to display it in.

//This element has two children, a shadowed box which is used as a background
//for text and a standard box which is the same color as the background of
//the current page and is used to hide any text that appears above the
//scroll box. Since the background color is dependent on the current mode,
//it has to be passed in as a variable in the constructor.

class PartialScrollingTextBox : public UIElement, IScrollableUI, ITextDimensionsUI
{
public:
	PartialScrollingTextBox(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, UIColor backgroundColor, std::wstring message, float fontSize,
		std::vector<UIColor> textColor = { UIColor::Black }, std::vector<unsigned long long> textColorLocations = {}, UITextJustification justification = UITextJustification::UpperLeft, 
		UIColor textFillColor = UIColor::White, bool isSquare = false,  UIColor outlineColor = UIColor::Black, UIColor shadowColor = UIColor::DarkGray);

	uint32_t update(InputState* inputState);

protected:
	float getCurrentTextStartingHeight();

	virtual void onScrollUp() override;
	virtual void onScrollDown() override;
	void calcualteScrollBarLocation(winrt::Windows::Foundation::Size windowSize);

	virtual std::vector<UIText*> setTextDimension() override;
	virtual void repositionText() override;

	float m_scrollIntensity; //the distance scrolling makes the text move relative to the current size of the window
	float m_buttonSize; //the size of the scroll buttons relative to the height of the text box
};