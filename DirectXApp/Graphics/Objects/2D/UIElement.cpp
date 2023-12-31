#include "pch.h"
#include "UIElement.h"

void UIElement::resize(winrt::Windows::Foundation::Size windowSize)
{
	//Recalculate the DPI coordinates for m_shape and m_text based on 
	//the current size of the screen and the absolute dimensions (m_location
	//and m_size) of the UI Element. Then resize and child UI Elements
	//owned by this one.

	if (m_needTextRenderDimensions && !(m_state & UIElementState::NeedTextPixels))
	{
		//This if block will only execute from a window resize and this UI Element
		//needs pixel height from the renderer (which can only happen in the main
		//thread). Update the state of the UI Element and leave this function. The
		//method will need to be called again manually from somewhere else.
		m_state |= UIElementState::NeedTextPixels;
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
		m_state ^= UIElementState::NeedTextPixels;
	}
}

uint32_t UIElement::update(InputState* inputState)
{
	//This method get's called on all the top level UI Elements for a given page in every
	//iteration of the main render loop. Each of these top level UI Elements will in turn
	//call this method on each of it's children to give us a snap shot of the state for
	//each element on the page.

	if ((m_state & UIElementState::Invisible) || (m_state & UIElementState::Disabled)) return 0; //Invisible and disabled elements don't get updated

	//If the element can't be interacted with in any way then simple return the idle state.
	if (!m_isClickable && !m_isHoverable && !m_isScrollable) m_state |= UIElementState::Idlee;

	//Only check for hover, click and scroll events if the mouse is
	//actually over the element
	bool mouseHovered = isMouseHovered(inputState->mousePosition);
	if (mouseHovered)
	{
		if (m_isHoverable && !(m_state & UIElementState::Hovered))
		{
			onHover();
			m_state |= UIElementState::Hovered;
		}

		if (m_isClickable)
		{
			//if the UI Element is clickable, check to see if we're currently clicking it
			//or releasing the mouse on it.
			if (inputState->mouseClickState == MouseClickState::MouseClicked)
			{
				onMouseClick();
				m_state |= UIElementState::Clicked; //add the clicked state to the element
			}
			else if (inputState->mouseClickState == MouseClickState::MouseRightClick)
			{
				//As of right now there are very few uses for right clicks with very
				//basic functionality so performing a right click won't effect the
				//current state of the UIElement
				onMouseRightClick();
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
				m_state |= UIElementState::Scrolled;
			}
		}
	}
	else
	{
		if (m_isHoverable && (m_state & UIElementState::Hovered))
		{
			removeState(UIElementState::Hovered);
		}
	}

	//Unlike the clicking action, the release action can occur when the mouse isn't
	//hovering directly over the element. This is because you can click the element,
	//move the mouse with the button held down, and then release somewhere else. We 
	//put the element into a different state upon release depending on wheter or not
	//the mouse is still hovering over the element or not.
	if (m_isClickable && (m_state & UIElementState::Clicked) && (inputState->mouseClickState == MouseClickState::MouseReleased))
	{
		onMouseRelease();
		m_state &= ~UIElementState::Clicked; //remove the clicked state from the element
		if (mouseHovered) m_state |= UIElementState::Released; //add the release state if the mouse is hovering the element (this triggers action elsewhere)
	}

	//After checking the current element for updates, check all of it's children
	uint32_t children_state = 0;
	for (int i = 0; i < p_children.size(); i++) children_state |= p_children[i]->update(inputState);

	//State flags (with the exception of the invisible, disabled, and idle flags) should bubble up from
	//child elements to their parent element. In cases where children implement an interface that the
	//parent doesn't, the parent's flag should equal that of the children's flag. If the parent and 
	//children do implement the same interface then the flag will be set if either the parent or any
	//children call for it, and won't be set if neither parent or children do.
	if (!m_isHoverable)
	{
		if (children_state & UIElementState::Hovered) m_state |= UIElementState::Hovered; //at least one child is hovered so add it to the parent's state
		else m_state &= ~UIElementState::Hovered; //no children are hovered so remove the hovered flag (if it's there) from the parent
	}
	else  m_state |= (children_state & UIElementState::Hovered); //If either the parent or child has the hover flag the parent will get it, otherwise the parent won't
	
	if (!m_isClickable)
	{
		//Check for both clicks and releases
		if (children_state & UIElementState::Clicked) m_state |= UIElementState::Clicked; //at least one child is clicked so add it to the parent's state
		else m_state &= ~UIElementState::Clicked; //no children are clicked so remove the clicked flag (if it's there) from the parent

		if (children_state & UIElementState::Released) m_state |= UIElementState::Released; //at least one child is released so add it to the parent's state
		else m_state &= ~UIElementState::Released; //no children are released so remove the released flag (if it's there) from the parent
	}
	else  m_state |= ((children_state & UIElementState::Clicked) | ((children_state & UIElementState::Released))); //If either the parent or child has the click/release flag the parent will get it, otherwise the parent won't

	if (!m_isScrollable)
	{
		if (children_state & UIElementState::Scrolled) m_state |= UIElementState::Scrolled; //at least one child is scrolled so add it to the parent's state
		else m_state &= ~UIElementState::Scrolled; //no children are scrolled so remove the scrolled flag (if it's there) from the parent
	}
	else  m_state |= (children_state & UIElementState::Scrolled); //If either the parent or child has the scroll flag the parent will get it, otherwise the parent won't


	return m_state;
}

winrt::Windows::Foundation::Size UIElement::getCurrentWindowSize()
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

bool UIElement::isMouseHovered(DirectX::XMFLOAT2 mousePosition)
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

void UIElement::setAbsoluteSize(DirectX::XMFLOAT2 size)
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

void UIElement::setAbsoluteLocation(DirectX::XMFLOAT2 location)
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

int UIElement::pixelCompare(float pixelOne, float pixelTwo)
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

void UIElement::removeState(uint32_t state)
{
	//removes the given state from UI Elements and any children that have the state as well
	m_state &= ~state;

	for (int i = 0; i < p_children.size(); i++)
	{
		if (p_children[i]->getState() & state) p_children[i]->removeState(state);
	}
}

void UIElement::setState(uint32_t state)
{ 
	//A pretty standard set function, however, some child elements need
	//the ability to make changes when their state is changed (for example,
	//setting a button into the disabled state will change its text color)
	//so this is a virtual method that can be overriden.
	m_state = state;
}

void UIElement::updateState(uint32_t state)
{
	//Unlike the setState method which changes the entire state, this method
	//updates the current state with the given flags without overwritting
	//anything. Also like the setState method, we leave this is a virtual
	//method in the case any children need a slightly different implementation
	m_state |= state;
}