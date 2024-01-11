#pragma once

#include "Box.h"

//A compund shape composed of two boxes. As the name implies, the result gives a solid colored
//box with an outline

class OutlinedBox : public Box
{
public:
	OutlinedBox(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, UIColor fillColor = UIColor::White, UIColor outlineColor = UIColor::Black) :
		Box(windowSize, location, size, fillColor, UIShapeFillType::Fill)
	{
		updateLocationAndSize(location, size);

		//Create a second box object that's just an outline and the same shape as the fill box
		Box outline(windowSize, location, size, outlineColor, UIShapeFillType::NoFill);
		p_children.push_back(std::make_shared<Box>(outline));

		m_state &= ~UIElementState::Dummy; //non-default constructed items get the dummy flag removed
	}

	OutlinedBox() {} //empty default constructor

	virtual void setChildrenAbsoluteSize(DirectX::XMFLOAT2 size) override
	{
		//The outlined box consists of two boxes with the exact same location and size
		//so for this method we just need to call the standard setAbsoluteSize() method
		//for the child box
		p_children[0]->setAbsoluteSize(size);
	}
};