#pragma once

#include "Graphics/Objects/2D/UIElement.h"

//This class represents free floating text without a background. The text can 
//all be one solid color, or multi-colored. Like a rendered shape, the m_size
//variable represents a point in the exact center of the rendering area.

class TextOverlay : public UIElement
{
public:
	TextOverlay(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring message,
		float fontSize, std::vector<UIColor> colors, std::vector<unsigned long long> colorLocations, UITextJustification justification);
};