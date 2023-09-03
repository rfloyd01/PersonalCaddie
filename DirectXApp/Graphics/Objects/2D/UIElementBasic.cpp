#include "pch.h"
#include "UIElementBasic.h"
#include "Buttons/Button.h"

void UIElementBasic::resize(winrt::Windows::Foundation::Size windowSize)
{
	//Recalculate the DPI coordinates for m_shape and m_text based on 
	//the current size of the screen and the absolute dimensions (m_location
	//and m_size) of the UI Element. Then resize and child UI Elements
	//owned by this one.

	if (m_shape.m_shapeType != UIShapeType::END)
	{
		m_shape.m_rectangle.left   = windowSize.Width * (m_location.x - m_size.x / (float)2.0);
		m_shape.m_rectangle.top    = windowSize.Height * (m_location.y - m_size.y / (float)2.0);
		m_shape.m_rectangle.right  = windowSize.Width * (m_location.x + m_size.x / (float)2.0);
		m_shape.m_rectangle.bottom = windowSize.Height * (m_location.y + m_size.y / (float)2.0);
	}
	
	if (m_text.textType != UITextType::END)
	{
		m_text.startLocation = { windowSize.Width * (m_location.x - m_size.x / (float)2.0), windowSize.Height * (m_location.y - m_size.y / (float)2.0) }; //text always starts at the top left of the UI Element
		m_text.renderArea = { windowSize.Width * m_size.x, windowSize.Height * m_size.y };
		m_text.fontSize = windowSize.Height * m_fontSize;
	}

	for (int i = 0; i < p_children.size(); i++) p_children[i]->resize(windowSize);
}

uint32_t UIElementBasic::update(InputState* inputState)
{
	//This method get's called on all the top level UI Elements for a given page in every
	//iteration of the main render loop. Each of these top level UI Elements will in turn
	//call this method on each of it's children to give us a snap shot of the state for
	//each element on the page.

	if (m_state & UIElementStateBasic::Invisible) return 0; //Invisible elements don't get updated

	//If the element can't be interacted with in any way then simple return the idle state.
	if (!m_isClickable && !m_isHoverable && !m_isScrollable) return UIElementStateBasic::Idlee;

	uint32_t currentState = 0;
	bool mouseHovered = isMouseHovered(inputState->mousePosition);
	if (!mouseHovered) return UIElementStateBasic::Idlee; //If the mouse isn't over the element then nothing changes

	if (m_isHoverable)
	{
		//cast the current object into an IHoverableUI so we can call it's onHover()
		//method.
		onHover();
		currentState |= UIElementStateBasic::Hovered;
	}

	if (m_isClickable)
	{
		//if the UI Element is clickable, check to see if we're currently clicking it.
		if (inputState->mouseClick)
		{
			onClick();
			currentState |= UIElementStateBasic::Clicked;
		}
	}

	if (m_isScrollable)
	{
		//if the UI Element is scrollable, check to see if the mouse wheel is currently moving.
		if (inputState->scrollWheelDirection != 0)
		{
			//cast the current object into an IScrollableUI so we can call it's onScrollUp()
			//or onScrollDown() method.
			if (inputState->scrollWheelDirection > 0) onScrollUp();
			else onScrollDown();
			currentState |= UIElementStateBasic::Scrolled;
		}
	}

	return currentState;
}

winrt::Windows::Foundation::Size UIElementBasic::getCurrentWindowSize()
{
	//All UI Elements need the capability of figuring out the current window size without help
	//from the master renderer. This is done by comparing the absoulte size variable (m_size)
	//to the current pixel width and height of the m_shape object or m_text object. If the 
	//UI Element doesn't have its own shape or text, then a child's will be used.
	if (m_shape.m_shapeType != UIShapeType::END)
	{
		return { (m_shape.m_rectangle.right - m_shape.m_rectangle.left) / m_size.x, (m_shape.m_rectangle.bottom - m_shape.m_rectangle.bottom) / m_size.y };
	}
	else if (m_text.textType != UITextType::END)
	{
		//Text height isn't always linked to the m_size.y variable so we calculate the screen height by comparing the
		//absolute font height vs. its pixel height. On the other hand text width should always be snapped to the 
		//m_size.x variable
		return { m_text.renderArea.x / m_size.x, m_text.fontSize / m_fontSize };
	}
	else
	{
		//If the UI Element doesn't have a shape are text of its own then it must have children that do
		//so recursively call this method again until we find a suitable child. Looping through all
		//children shouldn't be necessary as we should always be able to find a shape or text with a depth
		//first search.
		return p_children[0]->getCurrentWindowSize();
	}
}

bool UIElementBasic::isMouseHovered(DirectX::XMFLOAT2 mousePosition)
{
	//All UI Elements come with the capability of figuring out whether or not the user's mouse is hovered 
	//over them. Not all Elements care if they're being hovered over though so this method doesn't always
	//get called. Like in the getCurrentWindowSize() method, if the current element doesn't have its own
	//shape or text, we defer to the first element of the child vector. This wont always give us what we
	//want though so this method is virtual, giving more complex elements have the ability to override
	//this method.
	if (m_shape.m_shapeType != UIShapeType::END)
	{
		if ((mousePosition.x >= m_shape.m_rectangle.left) &&
			(mousePosition.x <= m_shape.m_rectangle.right) &&
			(mousePosition.y >= m_shape.m_rectangle.top) &&
			(mousePosition.y <= m_shape.m_rectangle.bottom)) return true;
		else return false;

	}
	else if (m_text.textType != UITextType::END)
	{
		if ((mousePosition.x >= m_text.startLocation.x) &&
			(mousePosition.x <= m_text.startLocation.x + m_text.renderArea.x) &&
			(mousePosition.y >= m_text.startLocation.y) &&
			(mousePosition.y <= m_text.startLocation.y + m_text.renderArea.y)) return true;
		else return false;
	}
	return p_children[0]->isMouseHovered(mousePosition);
}