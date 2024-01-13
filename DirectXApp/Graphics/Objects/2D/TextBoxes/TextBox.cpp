#include "pch.h"
#include "TextBox.h"

TextBox::TextBox(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring message,
	float fontSize, std::vector<UIColor> textColor, std::vector<unsigned long long> textColorLocations, UITextJustification justification, UIColor textFillColor, UIColor outlineColor, UIColor shadowColor)
{
	//Set the screen size dependent variables
	m_screenSize = windowSize;
	updateLocationAndSize(location, size);

	//First create the background of the text box. Normally the background for this box is white, although it
	//can be changed.
	ShadowedBox textBackground(windowSize, location, size, textFillColor, outlineColor, shadowColor);

	//Then create the text which gets overlayed over the text box. The text overlay has the same
	//dimensions and center as the text box. The defautl value for textColorLocations is an empty
	//vector so make sure that it has at least two values before creating the textoverly object.
	if (textColorLocations.size() == 0)
	{
		textColorLocations.push_back(0);
		textColorLocations.push_back(message.length());
	}

	//Create a text overlay which holds the actual text. To keep the text in the box properly
	//the text overlay should be set to use relative coordinates instead of absolute coordinates.
	TextOverlay text(windowSize, location, size, message, fontSize, textColor, textColorLocations, justification, false);

	//The order of the child elements is important here. The text background must be first, then the text.
	p_children.push_back(std::make_shared<ShadowedBox>(textBackground));
	p_children.push_back(std::make_shared<TextOverlay>(text));
}

void TextBox::setChildrenAbsoluteSize(DirectX::XMFLOAT2 size)
{
	//The textbox consists of two child elements, a shadowed box and a text
	//overlay. The shadowed box has the exact same dimensions as the text box
	//itself. The text overlay will have the same width as the text box, however,
	//the height and location are completely independent. It's possible for the
	//text overlay to even be outside of the textbox itself (like in the partial
	//scrolling text box element). For this reason, it will be up to any mode,
	//or other UI Element implementing a text box to make sure the text is in
	//the correct location.
	p_children[0]->setAbsoluteSize(size);

	//Change the text overlay height proporitionally to the width
	auto textOverlayAbsoluteSize = p_children[1]->getAbsoluteSize();
	p_children[1]->setAbsoluteSize({ size.x, textOverlayAbsoluteSize.y * size.x / textOverlayAbsoluteSize.x });
}