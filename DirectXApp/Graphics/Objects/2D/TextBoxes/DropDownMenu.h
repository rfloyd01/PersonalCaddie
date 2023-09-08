#pragma once

#include "Graphics/Objects/2D/Buttons/ArrowButton.h"
#include "FullScrollingTextBox.h"
#include "TextBox.h"

//The drop down box is a combination of a few things. First, there's a text
//box which displys the text of the currently selected option. Then, there's
//a buton which when pressed, brings up a full scrolling text box full of 
//other options to choose from. Selecting one of these options from the 
//scroll box will cause the scroll box to dissapear and the selected text
//will appear in the main text box.

class DropDownMenu : public UIElement, ITextDimensionsUI
{
public:
	DropDownMenu(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring message, float fontSize,
		int optionsDisplayed = 5, bool isInverted = false, std::vector<UIColor> textColor = { UIColor::Black }, std::vector<unsigned long long> textColorLocations = {}, UITextJustification justification = UITextJustification::UpperLeft, 
		UIColor textFillColor = UIColor::White, bool isSquare = false,  UIColor outlineColor = UIColor::Black, UIColor shadowColor = UIColor::DarkGray);

	virtual uint32_t update(InputState* inputState) override;
	virtual std::vector<UIText*> setTextDimension() override;

	std::wstring getSelectedOption() { return m_currentlySelectedOption; }

protected:
	virtual void repositionText() override;
	int m_optionsDisplayed;
	bool m_inverted;
	std::wstring m_currentlySelectedOption;
};