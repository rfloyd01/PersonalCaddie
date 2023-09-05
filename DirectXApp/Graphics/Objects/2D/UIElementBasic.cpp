#include "pch.h"
#include "UIElementBasic.h"
#include "Buttons/Button.h"

void UIElementBasic::resize(winrt::Windows::Foundation::Size windowSize)
{
	//Recalculate the DPI coordinates for m_shape and m_text based on 
	//the current size of the screen and the absolute dimensions (m_location
	//and m_size) of the UI Element. Then resize and child UI Elements
	//owned by this one.

	if (m_needTextRenderDimensions && !(m_state & UIElementStateBasic::NeedTextPixels))
	{
		//This if block will only execute from a window resize and this UI Element
		//needs pixel height from the renderer (which can only happen in the main
		//thread). Update the state of the UI Element and leave this function. The
		//method will need to be called again manually from somewhere else.
		m_state |= UIElementStateBasic::NeedTextPixels;
		return; //if we need text pixel height, don't resize until we get it
	}

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

	if (m_needTextRenderDimensions)
	{
		//If we're at this part of the resize method and this element has a dependency on 
		//text pixel dimentions it means that the update has occured and we can remove the
		//need for pixels from the current state
		m_state ^= UIElementStateBasic::NeedTextPixels;
	}
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

	//uint32_t currentState = 0;

	if (m_state & UIElementStateBasic::NeedTextPixels)
	{
		m_state |= UIElementStateBasic::NeedTextPixels;
	}

	//Only check for hover, click and scroll events if the mouse is
	//actually over the element
	bool mouseHovered = isMouseHovered(inputState->mousePosition);
	if (mouseHovered)
	{
		if (m_isHoverable && !(m_state & UIElementStateBasic::Hovered))
		{
			onHover();
			m_state |= UIElementStateBasic::Hovered;
		}

		if (m_isClickable)
		{
			//if the UI Element is clickable, check to see if we're currently clicking it.
			if (inputState->mouseClick)
			{
				onClick();
				m_state |= UIElementStateBasic::Clicked;
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
				m_state |= UIElementStateBasic::Scrolled;
			}
		}
	}
	else
	{
		if (m_isHoverable && (m_state & UIElementStateBasic::Hovered))
		{
			removeState(UIElementStateBasic::Hovered);
		}
	}

	//After checking the current element for update, check all of it's children
	for (int i = 0; i < p_children.size(); i++) m_state |= p_children[i]->update(inputState);

	return m_state;
}

winrt::Windows::Foundation::Size UIElementBasic::getCurrentWindowSize()
{
	//All UI Elements need the capability of figuring out the current window size without help
	//from the master renderer. This is done by comparing the absoulte size variable (m_size)
	//to the current pixel width and height of the m_shape object or m_text object. If the 
	//UI Element doesn't have its own shape or text, then a child's will be used.
	if (m_shape.m_shapeType != UIShapeType::END)
	{
		return { (m_shape.m_rectangle.right - m_shape.m_rectangle.left) / m_size.x, (m_shape.m_rectangle.bottom - m_shape.m_rectangle.top) / m_size.y };
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

void UIElementBasic::setAbsoluteSize(DirectX::XMFLOAT2 size)
{
	//Changing the absolute size for an element needs to change the absolute size of
	//itself, and, all of its children. We don't apply the same size parameter to the
	//children though. Instead we compare the old size of the current element to the 
	//new size to get a ratio. We increase the absolute size of the children by the
	//same ratio.
	DirectX::XMFLOAT2 sizeRatio = { size.x / m_size.x, size.y / m_size.y };
	m_size = size;

	for (int i = 0; i < p_children.size(); i++)
	{
		auto childAbsoluteSize = p_children[i]->getAbsoluteSize();
		p_children[i]->setAbsoluteSize({ childAbsoluteSize.x * sizeRatio.x, childAbsoluteSize.y * sizeRatio.y});
	}
}

void UIElementBasic::setAbsoluteLocation(DirectX::XMFLOAT2 location)
{
	//In the same vein as the setAbsoluteSize() method, this method must
	//also set the location of all child elements. We don't apply the same
	//location to each child (as some children have locations offset from their
	//parents). Instead we compare the current location and the new location
	//to get a difference. This same difference is then applied to the children.
	DirectX::XMFLOAT2 locationDifference = { location.x - m_location.x, location.y - m_location.y };
	m_location = location;

	for (int i = 0; i < p_children.size(); i++)
	{
		auto childAbsoluteLocation = p_children[i]->getAbsoluteLocation();
		p_children[i]->setAbsoluteLocation({ childAbsoluteLocation.x + locationDifference.x, childAbsoluteLocation.y + locationDifference.y });
	}
}

int UIElementBasic::pixelCompare(float pixelOne, float pixelTwo)
{
	//Since floating point numbers are used for all pixel related math then it's possible that
	//two pixels will occupy the same space, but have slightly different values (for example
	//pixel one has an x location of 125.00235 and pixel two has an x location of 125.014).
	//Some actions require the comparison of two pixel locations to decide the outcome,
	//so instead of using raw <, <=, ==, >= and > symbols, this method should get called
	//instead.

	//If the pixels are within 0.1 of each other then they're considered equal. Otherwise,
	//a negative number means pixelTwo is larger while a positive number means that
	//pixelOne is largber.
	float difference = pixelOne - pixelTwo;
	if (difference <= 0.1 && difference >= -0.1) return 0;
	return difference;
}

void UIElementBasic::removeState(uint32_t state)
{
	//removes the given state from UI Elements and any children that have the state as well
	m_state ^= state;

	for (int i = 0; i < p_children.size(); i++)
	{
		if (p_children[i]->getState() & state) p_children[i]->removeState(state);
	}
}