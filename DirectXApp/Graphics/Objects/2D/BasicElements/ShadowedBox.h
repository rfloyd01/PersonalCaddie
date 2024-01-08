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
		//Create a second box that's the same shape as the fill box, but is offset slightly to the right and down.
		//This will give the effect of there being a shadown behind the element
		DirectX::XMFLOAT2 maximumAbsolutePixelSize = { 1.0f / MAX_SCREEN_WIDTH, 1.0f / MAX_SCREEN_HEIGHT };
		Box outline(windowSize, { location.x + (maximumAbsolutePixelSize.x * m_shadowSizePixels) / 2.0f, location.y + (maximumAbsolutePixelSize.y * m_shadowSizePixels) / 2.0f },
			{ size.x + maximumAbsolutePixelSize.x * m_shadowSizePixels, size.y + maximumAbsolutePixelSize.y * m_shadowSizePixels }, shadowColor, UIShapeFillType::Fill, isSquare);
		
		p_children.push_back(std::make_shared<Box>(outline));
	}

	float getShadowWidth() { return m_shadowSizePixels; }

protected:
	const float m_shadowSizePixels = 5.0f; //The size of the shadow in pixels. At some point a may want to make this a changeable value but for now keep it const

};