#pragma once

#include "UIText.h"
#include "UIShape.h"

#include "Input/InputProcessor.h"

#include "Graphics/Utilities/UIElementColors.h"

//include interfaces that can be inherited
#include "Interfaces/IClickableUI.h"
#include "Interfaces/IHoverableUI.h"
#include "Interfaces/IScrollableUI.h"

enum UIElementStateBasic
{
	Idlee = 1,
	Clicked = 2,
	Scrolled = 4,
	Invisible = 8,
	Hovered = 16
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
	UIElementStateBasic getState() { return m_state; }
	void setState(UIElementStateBasic state) { m_state = state; }

	const UIShape* getShape() { return &m_shape; }
	const UIText* getText() { return &m_text; }

	std::vector<std::shared_ptr<UIElementBasic> > const& getChildren() { return p_children; }

	

protected:
	winrt::Windows::Foundation::Size getCurrentWindowSize();
	virtual bool isMouseHovered(DirectX::XMFLOAT2 mousePosition);

	//Screen size dependent variables
	DirectX::XMFLOAT2                             m_location; //location of the center of the element  as a ratio of the current screen size
	DirectX::XMFLOAT2                             m_size; //size of the ui element as a ratio of the current screen size
	float                                         m_fontSize; //The desired font size for text as a ratio of the current screen size

	//State variables
	UIElementStateBasic                              m_state;
	
	std::shared_ptr<UIElementBasic>               p_parent; //a pointer to the parent UI element, nullptr if there is no parent
	std::vector<std::shared_ptr<UIElementBasic> > p_children; //pointers to any children UI elements
	
	UIShape                                       m_shape; //A shape specific to this UI Element
	UIText                                        m_text;  //Any text that's specific to this UI Element
	

	bool                                          m_needTextRenderHeight = false; //lets the current mode know if we need the full height for any elementText items in pixels
	bool                                          m_isClickable = false;
	bool                                          m_isScrollable = false;
	bool                                          m_isHoverable = false;
};