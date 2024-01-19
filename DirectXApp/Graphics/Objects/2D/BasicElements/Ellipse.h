#pragma once

#include "Graphics/Objects/2D/UIElement.h"

//This class just represents a 2D ellipse

class Ellipse : public UIElement
{
public:
	Ellipse(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 centerLocation, DirectX::XMFLOAT2 radii, bool circle = false, UIColor color = UIColor::Black);

	virtual void resize(DirectX::XMFLOAT2 pixel_shift = { 0.0f, 0.0f }) override;
	UIColor getColor() const { return m_color; }

private:
	bool m_circle;
	UIColor m_color;
};