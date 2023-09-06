#pragma once

#include "Graphics/Objects/2D/BasicElements/ShadowedBox.h"

//A basic button is no more than a shadowed box that has the ability to be clicked.
//There are more complex button types out there, but this one is just a simple,
//empty box. Clicking it will temporarily change its color. The shadowed box is the
//first and only child element for the button.
class Button : public UIElement, IClickableUI
{
public:
	Button(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size,
		bool isSquare = false, UIColor fillColor = UIColor::ButtonNotPressed, UIColor outlineColor = UIColor::Black, UIColor shadowColor = UIColor::DarkGray);

	virtual void removeState(uint32_t state) override;

protected:
	virtual void onClick() override;
};