#pragma once

#include "Graphics/Objects/2D/UIElement.h"

//This class just represents a 2D ellipse

class Ellipse : public UIElement
{
public:
	Ellipse(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 centerLocation, DirectX::XMFLOAT2 radii, bool circle = false, UIColor color = UIColor::Black);

	virtual void resize() override;

private:
	bool m_circle;
};