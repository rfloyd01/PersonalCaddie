#include "pch.h"
#include "Button.h"

#include <ppltasks.h>
#include <chrono>

Button::Button(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size,
	bool isSquare, UIColor fillColor, UIColor outlineColor, UIColor shadowColor)
{
	//Set the window size dependent variables
	m_location = location;
	m_size = size;
	ShadowedBox box(windowSize, location, size, isSquare, fillColor, outlineColor, shadowColor);
	p_children.push_back(std::make_shared<ShadowedBox>(box));
	m_isClickable = true; //buttons can be clicked so we set this variable to true
}

void Button::onMouseClick()
{
	//When clicking a button we change its state to the clicked state, and
	//we change its background color. This alerts the main part of the program
	//to start a timer. When that timer goes off both the state and color will
	//revert.
	((Box*)p_children[0].get())->setBackgrounColor(UIColor::ButtonPressed);
}

void Button::onMouseRelease()
{
	//Releasing the button causes the color to return to it's natrual state
	((Box*)p_children[0].get())->setBackgrounColor(UIColor::ButtonNotPressed);
}