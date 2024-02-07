#include "pch.h"
#include "TextOverlay.h"
#include "Graphics/Objects/2D/BasicElements/Box.h"

TextOverlay::TextOverlay(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring message,
	float fontSize, std::vector<UIColor> colors, std::vector<unsigned long long> colorLocations, UITextJustification justification, bool useAbsolute)
{
	m_screenSize = windowSize;

	//Set the screen size dependent variables. Unlike most UI Elements, there are times when it makes sense
	//for the TextOverlay class to use the edges of the screen as a reference instead of the middle of the screen.
	//For example, we always want the title at the top of the screen to stay in the same relative position to the
	//top of the screen. There are times though when it makes sense to use the standard relative coordinates, like 
	//when text appears inside of a button for example. We can set the m_useAbsoluteCoordinates variable accordingly
	//use whichever paradigm is necessary.
	m_useAbsoluteCoordinates = useAbsolute;
	updateLocationAndSize(location, size);
	m_fontSize = fontSize;

	//Create a text object from the given wstring
	m_text.textType = UITextType::ELEMENT_TEXT;
	m_text.message = message;
	m_text.justification = justification;
	m_text.colors = colors;
	m_text.colorLocations = colorLocations;

	//Any UI Element created with a non-default constructor
	//has the 'Dummy' flag removed from its state
	removeState(UIElementState::Dummy);

	resize();
}

void TextOverlay::updateText(std::wstring message)
{
	//replaces the current text of the overlay
	m_text.message = message;
	m_text.colorLocations.back() = message.length();
}