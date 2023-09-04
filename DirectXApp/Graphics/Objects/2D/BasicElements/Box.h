#pragma once

#include "Graphics/Objects/2D/UIElementBasic.h"

class Box : public UIElementBasic
{
public:
	Box(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, UIColor color = UIColor::Black, UIShapeFillType fill = UIShapeFillType::Fill, bool isSquare = false);

	virtual void resize(winrt::Windows::Foundation::Size windowSize) override;

	void setBackgrounColor(UIColor color) { m_shape.m_color = color; }

	float fixSquareBoxDrift(winrt::Windows::Foundation::Size const& currentWindowSize);

protected:
	bool m_isSquare;
};