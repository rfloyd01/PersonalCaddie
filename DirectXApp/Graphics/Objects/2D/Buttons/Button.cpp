#include "pch.h"
#include "Button.h"

#include <ppltasks.h>
#include <chrono>

Button::Button(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size,
	UIColor fillColor, UIColor outlineColor, UIColor shadowColor, float shadowPixels)
{
	//Set the window size dependent variables
	updateLocationAndSize(location, size);

	ShadowedBox box(windowSize, location, size, fillColor, outlineColor, shadowColor, shadowPixels);
	p_children.push_back(std::make_shared<ShadowedBox>(box));
	m_isClickable = true; //buttons can be clicked so we set this variable to true

	m_state &= ~UIElementState::Dummy; //anything created without the default constructor shouldn't have the dummy state
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

void Button::onMouseRightClick()
{
	//Right clicking has no effect on buttons currently, this is just here
	//as the pure virtual method must be overridden.
}