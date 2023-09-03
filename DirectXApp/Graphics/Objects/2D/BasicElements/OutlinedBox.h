#pragma once

#include "Box.h"

//A compund shape composed of two boxes. As the name implies, the result gives a solid colored
//box with an outline

class OutlinedBox : public Box
{
public:
	OutlinedBox(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, bool isSquare, UIColor fillColor = UIColor::White, UIColor outlineColor = UIColor::Black) :
		Box(windowSize, location, size, fillColor, UIShapeFillType::Fill, isSquare)
	{
		//Create a second box object that's just an outline and the same shape as the fill box
		Box outline(windowSize, location, size, outlineColor, UIShapeFillType::NoFill, isSquare);
		p_children.push_back(std::make_shared<Box>(outline));
	}
};