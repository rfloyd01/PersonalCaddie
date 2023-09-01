#pragma once

#include "UIText.h"
#include "UIShape.h"
#include "Interfaces/IScrollableUI.h"
#include "Interfaces/ITextBoxUI.h"
#include "UIConstants.h"
#include "Input/InputProcessor.h"

#include "Graphics/Utilities/UIElementColors.h"

enum class UIElementState
{
	Idle,
	Clicked,
	Scrolled,
	Visible,
	Hovered
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
	~UIElement();

	int getRenderVectorSize(RenderOrder render);
	void* getRenderItem(RenderOrder render, int element);

	UIElementState getState() { return m_state; }
	virtual void setState(UIElementState state) { m_state = state; } //need the ability to manually set the element state from outside the class

	DirectX::XMFLOAT2 getLocation() { return m_location; }
	DirectX::XMFLOAT2 getSize() { return m_size; }

	D2D1_RECT_F getPixels(RenderOrder render, int i);

	virtual void resize(winrt::Windows::Foundation::Size windowSize) = 0; //since all UI elements are different they each need to resize differently
	virtual UIElementState update(InputState* inputState) = 0; //gets called in main render loop to check interactions with UI element

	//heirarchy getters and setters
	void setParent(UIElement*  parent) { p_parent = parent; }
	std::vector<std::shared_ptr<UIElement> > const& getChildrenUIElements() { return p_children; }

	bool isAlert();
	bool needTextRenderHeight() { return m_needTextRenderHeight; }

protected:
	DirectX::XMFLOAT2                        m_location; //location of the center of the element
	DirectX::XMFLOAT2                        m_size; //size of the ui element
	UIElementState                           m_state;
	
	UIElement*                               p_parent; //a pointer to the parent UI element, nullptr if there is no parent
	std::vector<std::shared_ptr<UIElement> > p_children; //pointers to any children UI elements
	
	std::vector<UIShape>                     m_backgroundShapes;
	std::vector<UIText>                      m_elementText;
	std::vector<UIShape>                     m_foregroundShapesNoOverlap;
	std::vector<UIShape>                     m_foregroundShapesCanOverlap;
	std::vector<UIText>                      m_textOverlay;

	bool                                     m_needTextRenderHeight = false; //lets the current mode know if we need the full height for any elementText items in pixels
};