#pragma once

#include "UIText.h"
#include "UIShape.h"
#include "Interfaces/IScrollableUI.h"
#include "Interfaces/ITextBoxUI.h"

enum class UIElementState
{
	Idle,
	Clicked,
	Scrolled,
	Visible
};

//used to make sure all parts of the UI element are rendered
//in the proper order
enum class RenderOrder
{
	Background = 0,
	ElementText = 1,
	ForegroundNoOverlap = 2,
	ForegroundWithOverlap = 3,
	TextOverlay = 4,
	End = 5
};

class UIElement
{
public:
	//UIElement() {}
	~UIElement() {}

	int getRenderVectorSize(RenderOrder render);
	void* render(RenderOrder render, int element);

	virtual void resize(winrt::Windows::Foundation::Size windowSize) = 0; //since all UI elements are different they each need to resize differently

protected:
	DirectX::XMFLOAT2                        m_location; //location of the center of the element
	DirectX::XMFLOAT2                        m_size; //size of the ui element
	UIElementState                           m_state;
	
	std::shared_ptr<UIElement>               p_parent; //a pointer to the parent UI element, nullptr if there is no parent
	std::vector<std::shared_ptr<UIElement> > p_children; //pointers to any children UI elements
	
	std::vector<UIShape>                     m_backgroundShapes;
	std::vector<UIText>                      m_elementText;
	std::vector<UIShape>                     m_foregroundShapesNoOverlap;
	std::vector<UIShape>                     m_foregroundShapesCanOverlap;
	std::vector<UIText>                      m_textOverlay;
};