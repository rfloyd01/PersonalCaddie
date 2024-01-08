#pragma once

#include "Graphics/Objects/2D/BasicElements/ShadowedBox.h"

//A basic button is no more than a shadowed box that has the ability to be clicked.
//There are more complex button types out there, but this one is just a simple,
//empty box. Clicking it will temporarily change its color. The shadowed box is the
//first and only child element for the button.
class Button : public UIElement, IClickableUI
{
public:
	Button() {}; //empty default constructor
	Button(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size,
		UIColor fillColor = UIColor::ButtonNotPressed, UIColor outlineColor = UIColor::Black, UIColor shadowColor = UIColor::DarkGray, float shadowPixels = 8.0f);

protected:
	virtual void onMouseClick() override;
	virtual void onMouseRelease() override;
	virtual void onMouseRightClick() override;
};