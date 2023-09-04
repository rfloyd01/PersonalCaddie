#include "pch.h"
#include "TextBoxBasic.h"

TextBoxBasic::TextBoxBasic(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring message,
	float fontSize, std::vector<UIColor> textColor, std::vector<unsigned long long> textColorLocations, UITextJustification justification, UIColor textFillColor, bool isSquare, UIColor outlineColor, UIColor shadowColor)
{
	//First create the background of the text box. Normally the background for this box is white, although it
	//can be changed.
	ShadowedBox textBackground(windowSize, location, size, isSquare, textFillColor, outlineColor, shadowColor);

	//Then create the text which gets overlayed over the text box. The text overlay has the same
	//dimensions and center as the text box. The defautl value for textColorLocations is an empty
	//vector so make sure that it has at least two values before creating the textoverly object.
	if (textColorLocations.size() == 0)
	{
		textColorLocations.push_back(0);
		textColorLocations.push_back(message.length());
	}

	TextOverlayBasic text(windowSize, location, size, message, fontSize, textColor, textColorLocations, justification);

	//The order of the child elements is important here. The text background must be first, then the text.
	p_children.push_back(std::make_shared<ShadowedBox>(textBackground));
	p_children.push_back(std::make_shared<TextOverlayBasic>(text));

	//Set the screen size dependent information for the TextBox
	m_size = size;
	m_location = location;
}