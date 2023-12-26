#pragma once

#include "Graphics/Objects/2D/UIElement.h"

class Box : public UIElement
{
public:
	Box(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, UIColor color = UIColor::Black, UIShapeFillType fill = UIShapeFillType::Fill, bool isSquare = false);

	virtual void resize(winrt::Windows::Foundation::Size windowSize) override;

	void setBackgrounColor(UIColor color) { m_shape.m_color = color; }

	bool isSquare() { return m_isSquare; }

	float fixSquareBoxDrift(winrt::Windows::Foundation::Size const& currentWindowSize);

protected:
	//Sometimes it's useful to have boxes that only scale in a single direction. For
	//example, if you make the screen twice as wide you don't really need a vertical
	//scroll bar to double its width. By setting the m_isSquare boolean to true it makes
	//both the height and width of a box dependent on the screen height. The height and
	//width don't need to be the same, they just both become dependent on the window height.
	bool m_isSquare;
};