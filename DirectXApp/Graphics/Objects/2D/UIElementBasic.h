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

enum UIElementStateBasic
{
	Idlee = 1,
	Clicked = 2,
	Scrolled = 4,
	Invisible = 8,
	Hovered = 16,
	NeedTextPixels = 32,
	Selected = 64,
	Disabled = 128
};

//used to make sure all parts of the UI element are rendered
//in the proper order

class UIElementBasic
{
public:
	UIElementBasic() { } //empty default initializer
	~UIElementBasic()
	{
		//Deleting a UI Element should also delete any children that it has.
		for (int i = 0; i < p_children.size(); i++) p_children[i] = nullptr;
		p_children.clear();
	}

	virtual void resize(winrt::Windows::Foundation::Size windowSize);

	virtual uint32_t update(InputState* inputState);

	//Getters and Setters
	uint32_t getState() { return m_state; }
	virtual void setState(uint32_t state);
	virtual void removeState(uint32_t state);

	void setFontSize(float size) { m_fontSize = size; }

	UIShape* getShape() { return &m_shape; }
	UIText* getText() { return &m_text; }

	DirectX::XMFLOAT2 getAbsoluteSize() { return m_size; }
	void setAbsoluteSize(DirectX::XMFLOAT2 size);

	DirectX::XMFLOAT2 getAbsoluteLocation() { return m_location; }
	void setAbsoluteLocation(DirectX::XMFLOAT2 location);

	std::vector<std::shared_ptr<UIElementBasic> > const& getChildren() { return p_children; }

	virtual std::vector<UIText*> setTextDimension() { return {}; }; //empty getTextDimension method can be overriden by ITextDimension element users
	virtual void repositionText() {}; //empty repositionText method can be overriden by ITextDimension element users

	bool isAlert() { return m_isAlert; }
	void setAlert() { m_isAlert = true; }

protected:
	int pixelCompare(float pixelOne, float pixelTwo);

	winrt::Windows::Foundation::Size getCurrentWindowSize();
	virtual bool isMouseHovered(DirectX::XMFLOAT2 mousePosition);

	virtual void onClick() {} //empty onClick method can be overriden by IClickable element users
	virtual void onScrollUp() {} //empty onClick method can be overriden by IClickable element users
	virtual void onScrollDown() {} //empty onClick method can be overriden by IClickable element users
	virtual void onHover() {} //empty onClick method can be overriden by IClickable element users

	//Screen size dependent variables
	DirectX::XMFLOAT2                             m_location; //location of the center of the element  as a ratio of the current screen size
	DirectX::XMFLOAT2                             m_size; //size of the ui element as a ratio of the current screen size
	float                                         m_fontSize; //The desired font size for text as a ratio of the current screen size

	//State variables
	uint32_t                          m_state;
	
	std::shared_ptr<UIElementBasic>               p_parent; //a pointer to the parent UI element, nullptr if there is no parent
	std::vector<std::shared_ptr<UIElementBasic> > p_children; //pointers to any children UI elements
	
	UIShape                                       m_shape; //A shape specific to this UI Element
	UIText                                        m_text;  //Any text that's specific to this UI Element
	
	bool                                          m_isClickable = false;
	bool                                          m_isScrollable = false;
	bool                                          m_isHoverable = false;
	bool                                          m_isAlert = false;

	bool                                          m_needTextRenderDimensions = false; //lets the current mode know if we need the full height for any elementText items in pixels
	//bool                                          m_gotTextRenderDimensions = false; //lets the current mode know when elementText pixels have been received
};