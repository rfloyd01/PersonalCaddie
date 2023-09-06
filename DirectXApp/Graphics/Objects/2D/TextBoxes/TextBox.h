#pragma once

#include "Graphics/Objects/2D/BasicElements/ShadowedBox.h"
#include "Graphics/Objects/2D/BasicElements/TextOverlay.h"

//The basic text box consists of two children UI Elements. There's a shadowed
//box which is meant as the background for text (default color is white) and
//then there's a TextOverlay element which displays text on top of the
//background. The text box doesn't have any interactions you can do with it
//so if too much text is added the box will need to be made bigger to display
//all of the words.

class TextBox : public UIElement
{
public:
	TextBox(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring message, float fontSize,
		std::vector<UIColor> textColor = { UIColor::Black }, std::vector<unsigned long long> textColorLocations = {}, UITextJustification justification = UITextJustification::UpperLeft, 
		UIColor textFillColor = UIColor::White, bool isSquare = false,  UIColor outlineColor = UIColor::Black, UIColor shadowColor = UIColor::DarkGray);

};