#pragma once

#include "Graphics/Objects/2D/UIElement.h"

//A line is defined by two points, so it goes to reason that to
//create a line we need to pass in the absolute location for two
//points on the window. This UIElement is used for drawing very 
//rudimentary shapes, like arrows for buttons.

class Line : public UIElement
{
public:
	Line(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 firstPointLlocation, DirectX::XMFLOAT2 secondPointLlocation, UIColor color = UIColor::Black);

};