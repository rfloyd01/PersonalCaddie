#include "pch.h"
#include "TextOverlay.h"

TextOverlay::TextOverlay(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring message,
	float fontSize, std::vector<UIColor> colors, std::vector<unsigned long long> colorLocations, UITextJustification justification)
{
	//Set the screen size dependent variables
	m_size = size;
	m_location = location;
	m_fontSize = fontSize;

	//Create a text object from the given wstring
	m_text.textType = UITextType::ELEMENT_TEXT;
	m_text.message = message;
	m_text.justification = justification;
	m_text.colors = colors;
	m_text.colorLocations = colorLocations;

	resize(windowSize);
}