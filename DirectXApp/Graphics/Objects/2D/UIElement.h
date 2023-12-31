#pragma once

#include "pch.h"

#include "UIText.h"
#include "UIShape.h"
#include "UIConstants.h"

#include "Input/InputProcessor.h"

#include "Graphics/Utilities/UIElementColors.h"

//include interfaces that can be inherited
#include "Interfaces/IClickableUI.h"
#include "Interfaces/IHoverableUI.h"
#include "Interfaces/IScrollableUI.h"
#include "Interfaces/ITextDimensionsUI.h"

enum UIElementState
{
	Idlee = 1,
	Clicked = 2,
	Scrolled = 4,
	Invisible = 8,
	Hovered = 16,
	NeedTextPixels = 32,
	Selected = 64,
	Disabled = 128,
	Dummy = 256,
	Released = 512
};

//used to make sure all parts of the UI element are rendered
//in the proper order

class UIElement
{
public:
	UIElement()
	{
		m_size = { 0.0f, 0.0f };
		m_location = { 0.0f, 0.0f };
		m_fontSize = 0.0f;
		m_state = Dummy;
	}
	~UIElement()
	{
		//Deleting a UI Element should also delete any children that it has.
		for (int i = 0; i < p_children.size(); i++) p_children[i] = nullptr;
		p_children.clear();
	}

	virtual void resize(winrt::Windows::Foundation::Size windowSize);

	virtual uint32_t update(InputState* inputState);

	winrt::Windows::Foundation::Size getCurrentWindowSize();

	//Methods for modifying the state of the UI Element
	uint32_t getState() { return m_state; }
	void updateState(UIElementState state) { m_state |= state; } //add the given state to the overall state
	virtual void setState(uint32_t state);
	virtual void updateState(uint32_t state);
	virtual void removeState(uint32_t state);

	void setFontSize(float size) { m_fontSize = size; }

	UIShape* getShape() { return &m_shape; }
	UIText* getText() { return &m_text; }

	DirectX::XMFLOAT2 getAbsoluteSize() { return m_size; }
	virtual void setAbsoluteSize(DirectX::XMFLOAT2 size); //some elements that are tied to specific locations of other elements need to override this method

	DirectX::XMFLOAT2 getAbsoluteLocation() { return m_location; }
	void setAbsoluteLocation(DirectX::XMFLOAT2 location);

	std::vector<std::shared_ptr<UIElement> > const& getChildren() { return p_children; }

	virtual std::vector<UIText*> setTextDimension() { return {}; }; //empty getTextDimension method can be overriden by ITextDimension element users
	virtual void repositionText() {}; //empty repositionText method can be overriden by ITextDimension element users

	bool isAlert() { return m_isAlert; } //deprecated
	void setAlert() { m_isAlert = true; } //deprecated

	void getAllTextNeedingDimensions(std::vector<UIText*>* text);

protected:
	int pixelCompare(float pixelOne, float pixelTwo);

	virtual bool isMouseHovered(DirectX::XMFLOAT2 mousePosition);

	//Default IClickable Implementations
	virtual void onMouseClick() {} //empty onMouseClick method can be overriden by IClickable element users
	virtual void onMouseRelease() {} //empty onMouseRelease method can be overriden by IClickable element users
	virtual void onMouseRightClick() {} //empty onMouseRightClick method can be overriden by IClickable element users

	//Default IScrollable Implementations
	virtual void onScrollUp() {} //empty onScrollUp method can be overriden by IScrollable element users
	virtual void onScrollDown() {} //empty onScrollDown method can be overriden by IScrollable element users

	//Default IHoverable Implementations
	virtual void onHover() {} //empty onHoever method can be overriden by IHoverable element users

	//Screen size dependent variables
	DirectX::XMFLOAT2                             m_location; //location of the center of the element  as a ratio of the current screen size
	DirectX::XMFLOAT2                             m_size; //size of the ui element as a ratio of the current screen size
	float                                         m_fontSize; //The desired font size for text as a ratio of the current screen size

	//State variables
	uint32_t                                      m_state;
	
	std::vector<std::shared_ptr<UIElement> >      p_children; //pointers to any children UI elements
	
	UIShape                                       m_shape; //A shape specific to this UI Element
	UIText                                        m_text;  //Any text that's specific to this UI Element
	
	bool                                          m_isClickable = false;
	bool                                          m_isScrollable = false;
	bool                                          m_isHoverable = false;
	bool                                          m_isAlert = false;

	bool                                          m_needTextRenderDimensions = false; //lets the current mode know if we need the full height for any elementText items in pixels
	//bool                                          m_gotTextRenderDimensions = false; //lets the current mode know when elementText pixels have been received
};