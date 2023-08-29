#pragma once

#include "Graphics/Utilities/Text.h"

enum class UIElementState
{
	PassiveOutline,
	PassiveBackground,
	ActiveBackground,
	CoverBackground,
	Pressed,
	NotPressed,
	None
};

class UIElement
{
public:
	UIElement() {}
	~UIElement() {}

	virtual void render() = 0;

protected:
	DirectX::XMFLOAT2                        m_location; //location of the center of the element
	DirectX::XMFLOAT2                        m_size; //size of the ui element
	UIElementState                           m_state;
	
	std::shared_ptr<UIElement>               p_parent; //a pointer to the parent UI element, nullptr if there is no parent
	std::vector<std::shared_ptr<UIElement> > p_children; //pointers to any children UI elements
	
	std::vector<D2D1_RECT_F>                 m_backgroundShapes;
	std::vector<Text>                        m_elementText;
	std::vector<D2D1_RECT_F>                 m_foregroundShapesNoOverlap;
	std::vector<D2D1_RECT_F>                 m_foregroundShapesCanOverlap;
	std::vector<Text>                        m_textOverlay;
};