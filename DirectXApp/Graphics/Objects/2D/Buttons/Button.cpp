#include "pch.h"
#include "Button.h"
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

void Button::onClick()
{
	//When clicking a button we change its state to the clicked state, and
	//we change its background color. This alerts the main part of the program
	//to start a timer. When that timer goes off both the state and color will
	//revert.
	//setState(UIElementStateBasic::Clicked);
	((Box*)p_children[0].get())->setBackgrounColor(UIColor::ButtonPressed);

	//Create a timer and Asynchronously wait for it to complete. When it does,
	//change the button back to its original color
	concurrency::task<void> timer([this]()
		{
			auto timer = std::chrono::steady_clock::now();
			while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timer).count() < clickTimer) {}

			removeState(UIElementState::Clicked);
			((Box*)p_children[0].get())->setBackgrounColor(UIColor::ButtonNotPressed);
		});
}

//void Button::removeState(uint32_t state)
//{
//	//When the clicked state is removed from the button we change its color back
//	if (state & UIElementState::Clicked)
//	{
//		m_state ^= UIElementState::Clicked;
//		((Box*)p_children[0].get())->setBackgrounColor(UIColor::ButtonNotPressed);
//	}
//}