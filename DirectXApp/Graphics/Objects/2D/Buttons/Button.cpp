#include "pch.h"
#include "Button.h"

Button::Button(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size,
	bool isSquare, UIColor fillColor, UIColor outlineColor, UIColor shadowColor) :
	ShadowedBox(windowSize, location, size, isSquare, fillColor, outlineColor, shadowColor)
{
	
}

void Button::onClick()
{
	//When clicking a button we change its state to the clicked state, and
	//we change its background color. This alerts the main part of the program
	//to start a timer. When that timer goes off both the state and color will
	//revert.
	setState(UIElementStateBasic::Clicked);
	((Box*)p_children[0]->getChildren()[0].get())->setBackgrounColor(UIColor::ButtonPressed);

}