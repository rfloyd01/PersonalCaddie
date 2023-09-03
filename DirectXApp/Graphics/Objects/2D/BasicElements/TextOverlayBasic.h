#pragma once

#include "Graphics/Objects/2D/UIElementBasic.h"

class TextOverlayBasic : public UIElementBasic
{
public:
	TextOverlayBasic(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, UIColor color = UIColor::Black, UIShapeFillType fill = UIShapeFillType::Fill);

protected:

};