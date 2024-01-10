#pragma once

#include "Graphics/Objects/2D/UIElement.h"

//This class represents free floating text without a background. The text can 
//all be one solid color, or multi-colored. Like a rendered shape, the m_size
//variable represents a point in the exact center of the rendering area.

class TextOverlay : public UIElement
{
public:
	TextOverlay(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring message,
		float fontSize, std::vector<UIColor> colors, std::vector<unsigned long long> colorLocations, UITextJustification justification, bool useAbsolute = true);

	TextOverlay() {} //empty default constructor

	void updateText(std::wstring message);
	void updateColorLocations(std::vector<unsigned long long> const& newLocations)
	{
		//this method assumes newLocations and colorLocations vectors are the same length
		for (int i = 0; i < newLocations.size(); i++)
		{
			m_text.colorLocations[i] = newLocations[i];
		}
	}


};