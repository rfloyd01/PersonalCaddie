#pragma once

#include "OutlinedBox.h"

//The shadowed box is an outlined box that also has a slight shadow which appears
//to the bottom right of it.

class ShadowedBox : public OutlinedBox
{
public:
	ShadowedBox(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, bool isSquare, UIColor fillColor = UIColor::White, UIColor outlineColor = UIColor::Black, UIColor shadowColor = UIColor::DarkGray) :
		OutlinedBox(windowSize, location, size, isSquare, fillColor, outlineColor)
	{
		//Create a second box that's the same shape as the fill box, but is offset slightly to the right and down
		DirectX::XMFLOAT2 absolutePixelSize = { (float)1.0 / windowSize.Width, (float)1.0 / windowSize.Height };
		Box outline(windowSize, { location.x + (absolutePixelSize.x * m_shadowSizePixels) / (float)2.0, location.y + (absolutePixelSize.y * m_shadowSizePixels) / (float)2.0 },
			{ size.x + absolutePixelSize.x * m_shadowSizePixels, size.y + absolutePixelSize.y * m_shadowSizePixels }, shadowColor, UIShapeFillType::Fill, isSquare);
		p_children.push_back(std::make_shared<Box>(outline));
	}

protected:
	const float m_shadowSizePixels = 3.0; //The size of the shadow in pixels. At some point a may want to make this a changeable value but for now keep it const

};