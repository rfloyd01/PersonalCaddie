#pragma once

#include "Graphics/Objects/2D/UIElement.h"

class Box : public UIElement
{
public:
	Box() {} //empty default constructor
	Box(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size,
		UIColor color = UIColor::Black, UIShapeFillType fill = UIShapeFillType::Fill);

	void setBackgrounColor(UIColor color) { m_shape.m_color = color; }
};