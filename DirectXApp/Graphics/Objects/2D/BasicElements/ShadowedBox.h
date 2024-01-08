#pragma once

#include "OutlinedBox.h"

//The shadowed box is an outlined box that also has a slight shadow which appears
//to the bottom right of it.

class ShadowedBox : public UIElement
{
public:
	ShadowedBox() {} //compty default constructor

	ShadowedBox(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, UIColor fillColor = UIColor::White, UIColor outlineColor = UIColor::Black, UIColor shadowColor = UIColor::DarkGray, float shadowPixels = 8.0f)
	{
		//A shadow box is a compound element comprised of an outlined box and a standard box child elements.
		//The standard box gets rendered first and is offset a few pixels down and to the right of the 
		//outlined box, giving the outlined box the appearance of having a shadow.
		m_shadowSizePixels = shadowPixels;

		//The m_size.x variable measures from the left side of the outlined box to the right side
		//of the shadow while the m_size.y variable measures from the top of the outlined box to 
		//the bottom of the shadow. The m_location variable is in the exact middle of the two boxes,
		//meaning the centers of each box is offset a little bit from the center of the entire element.
		updateLocationAndSize(location, size);
		
		DirectX::XMFLOAT2 maximumAbsolutePixelSize = { 1.0f / MAX_SCREEN_WIDTH, 1.0f / MAX_SCREEN_HEIGHT };
		Box shadow(windowSize, { location.x + (maximumAbsolutePixelSize.x * m_shadowSizePixels) / 2.0f, location.y + (maximumAbsolutePixelSize.y * m_shadowSizePixels) / 2.0f },
			{ size.x - maximumAbsolutePixelSize.x * m_shadowSizePixels, size.y - maximumAbsolutePixelSize.y * m_shadowSizePixels }, shadowColor, UIShapeFillType::Fill);
		
		OutlinedBox outline(windowSize, { location.x - (maximumAbsolutePixelSize.x * m_shadowSizePixels) / 2.0f, location.y - (maximumAbsolutePixelSize.y * m_shadowSizePixels) / 2.0f },
			{ size.x - maximumAbsolutePixelSize.x * m_shadowSizePixels, size.y - maximumAbsolutePixelSize.y * m_shadowSizePixels }, false, fillColor, outlineColor);
		
		//The shadow gets added to the child array first so the outline box
		//gets rendered on top of it.
		p_children.push_back(std::make_shared<Box>(shadow));
		p_children.push_back(std::make_shared<OutlinedBox>(outline));

		resize(windowSize);
	}

	float getShadowWidth() { return m_shadowSizePixels; }

protected:
	float m_shadowSizePixels; //The size of the shadow in pixels. Default value is set in constructor
};