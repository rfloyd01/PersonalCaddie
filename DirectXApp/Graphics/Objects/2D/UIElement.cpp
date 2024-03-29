#include "pch.h"
#include "UIElement.h"

int UIElement::resize_count = 0;

void UIElement::resize(DirectX::XMFLOAT2 pixel_shift)
{
	//The resize() method gets called whenever the size of the physical screen changes.
	//When the screen size change two things happen to UI Elements. First, they obviously will
	//shirnk or grow. Second, and less obvious, elements will appear to drift away from or towards
	//each other. This resize() method handles the changing of the element's size so that the 
	//width and height stay the same relative size to each other, as well as to the size of 
	//the screen. It also prevents elements from drifting apart from each other.
	if (m_useAbsoluteCoordinates)
	{
		//If the element uses absolute coordinates then no complex
		//drift calculations need to be carried out. This applies to 
		//elements (like large text overlays) which are placed relative 
		//to the edges of the screen as opposed to the center of the screen.
		if (m_shape.m_shapeType != UIShapeType::END)
		{
			m_shape.m_rectangle.left = m_screenSize->Width * (m_location.x - m_size.x / 2.0f);
			m_shape.m_rectangle.top = m_screenSize->Height * (m_location.y - m_size.y / 2.0f);
			m_shape.m_rectangle.right = m_screenSize->Width * (m_location.x + m_size.x / 2.0f);
			m_shape.m_rectangle.bottom = m_screenSize->Height * (m_location.y + m_size.y / 2.0f);
		}
		
		if (m_text.textType != UITextType::END)
		{
			m_text.startLocation = { m_screenSize->Width * (m_location.x - m_size.x / 2.0f), m_screenSize->Height * (m_location.y - m_size.y / 2.0f) }; //text always starts at the top left of the UI Element
			m_text.renderArea = { m_screenSize->Width * m_size.x, m_screenSize->Height * m_size.y };
			m_text.fontSize = m_screenSize->Height * m_fontSize; //absolute text is sized against the screen height
		}
	}
	else
	{
		//If the element uses relative coordinates then drift calculations need to
		//be carried out. First calculate the center location of the element, its
		//height and its width in pixels
		DirectX::XMFLOAT2 element_pixel_center = { m_screenSize->Width * m_location.x, m_screenSize->Height * m_location.y };
		DirectX::XMFLOAT2 element_pixel_dimensions;

		//The height of the element is calculated by using a weighted ratio of the screen's current
		//height to the screen's current width, with the screen height getting the heigher weigtht.
		//The pixel width of the element is just a simple multiple of the height after it's calculated.
		element_pixel_dimensions.y = m_size.y / (m_screenSize->Width + m_screenSize->Height) * (2 * m_screenSize->Width * m_screenSize->Height);
		element_pixel_dimensions.x = m_sizeMultiplier.x * element_pixel_dimensions.y;

		//Second, account for the "drift" effect of the center of the element. This effect occurs
		//since the dimensions are co-dependent on the height AND width of the current screen as opposed
		//to the element height only being dependent on screen height, and the element width only 
		//being dependent on screen width.
		float newPixelDistanceToHorizontal = m_horizontalDriftMultiplier * element_pixel_dimensions.y;
		float newPixelDistanceToVertical = m_absoluteDistanceToScreenCenter.y * element_pixel_dimensions.y / m_size.y;

		element_pixel_center.x += newPixelDistanceToHorizontal - m_absoluteDistanceToScreenCenter.x * m_screenSize->Width;
		element_pixel_center.y += newPixelDistanceToVertical - m_absoluteDistanceToScreenCenter.y * m_screenSize->Height;

		//Once the correct location has been found, check to make sure that no part of the element will be 
		//outside of the screen. If it is, then the element will need to be shrunk so that it remains in 
		//screen.
		if ((pixel_shift.x == 0.0f) && (pixel_shift.y == 0.0f))
		{
			//Since parent UI Elements are checked before child elements, and the dimensions for
			//all child elements are contained within the dimensions of the parent element, only
			//parent elements will actually trigger the screen boundary check. Shift the parent
			//element
			screenBoundaryCheck(element_pixel_center, element_pixel_dimensions, pixel_shift);
		}

		//Once the center drift has been accounted for, size either the m_shape or m_text variable so that
		//the new pixel center is the exact center of the element. The m_shape variable gets preference over
		//the m_text variable so check for this variable first.
		if (m_shape.m_shapeType != UIShapeType::END)
		{
			m_shape.m_rectangle.left = element_pixel_center.x - element_pixel_dimensions.x / 2.0f + pixel_shift.x;
			m_shape.m_rectangle.top = element_pixel_center.y - element_pixel_dimensions.y / 2.0f + pixel_shift.y;
			m_shape.m_rectangle.right = element_pixel_center.x + element_pixel_dimensions.x / 2.0f + pixel_shift.x;
			m_shape.m_rectangle.bottom = element_pixel_center.y + element_pixel_dimensions.y / 2.0f + pixel_shift.y;
		}
		
		if (m_text.textType != UITextType::END)
		{
			m_text.startLocation = { element_pixel_center.x - element_pixel_dimensions.x / 2.0f + pixel_shift.x, element_pixel_center.y - element_pixel_dimensions.y / 2.0f + pixel_shift.y }; //text always starts at the top left of the UI Element
			m_text.renderArea = { element_pixel_dimensions.x, element_pixel_dimensions.y };
			m_text.fontSize = element_pixel_dimensions.y * m_fontSize; //relative text is sized against the element's height
		}
	}

	//Resize all child elements as well
	for (int i = 0; i < p_children.size(); i++) p_children[i]->resize(pixel_shift);
}

void UIElement::screenBoundaryCheck(DirectX::XMFLOAT2& pix_location, DirectX::XMFLOAT2& pix_size, DirectX::XMFLOAT2& pixel_shift)
{
	//Elements with relative dimensions are placed and sized relative to the center of
	//the screen, which means the they don't care about their location with respect to
	//the edges of the screen. If the screen is shrunk too small then elements may 
	//actually shift out of the viewing area. When that happens this method will detect
	//it and srink elements accordingly to prevent this from happening.

	//Check the width first
	if (pix_location.x - pix_size.x / 2.0f < m_edgeThreshold)
	{
		if (m_location.x == 0.5f)
		{
			//If the center of the element is in the center of the screen
		    //then we shrink the element enough so that both edges are within
		    //the threshold
			pix_size.x -= 2 * (m_edgeThreshold - pix_location.x) + pix_size.x;
			pix_size.y = pix_size.x / m_sizeMultiplier.x;
		}
		else
		{
			//If the element isn't in the center of the screen, we shift it so 
			//that it stays glued to the boundary
			pixel_shift.x = (m_edgeThreshold - pix_location.x + pix_size.x / 2.0f);
		}
	}
	else if (pix_location.x + pix_size.x / 2.0f > (m_screenSize->Width - m_edgeThreshold))
	{
		//Shift the element so that it stays glued to the edge boundary
		pixel_shift.x = (m_screenSize->Width - m_edgeThreshold) - (pix_location.x + pix_size.x / 2.0f);
	}

	//It's possible that after shrinking the width, the height could
	//still be out of bounds. Or it's possible the width was fine to
	//being with and the height wasn't. Either way, check the height 
	//as well.
	if (pix_location.y - pix_size.y / 2.0f < m_edgeThreshold)
	{
		if (m_location.y == 0.5f)
		{
			//If the center of the element is in the center of the screen
			//then we shrink the element enough so that both edges are within
			//the threshold
			pix_size.y -= 2 * (m_edgeThreshold - pix_location.y) + pix_size.y;
			pix_size.x = pix_size.y * m_sizeMultiplier.x;
		}
		else
		{
			//Shift the element so that it stays glued to the edge boundary
			pixel_shift.y = (m_edgeThreshold - pix_location.y + pix_size.y / 2.0f);
		}
	}
	else if (pix_location.y + pix_size.y / 2.0f > (m_screenSize->Height - m_edgeThreshold))
	{
		//Shift the element so that it stays glued to the edge boundary
		pixel_shift.y = (m_screenSize->Height - m_edgeThreshold) - (pix_location.y + pix_size.y / 2.0f);
	}
}

uint32_t UIElement::update(InputState* inputState)
{
	//This method get's called on all the top level UI Elements for a given page in every
	//iteration of the main render loop. Each of these top level UI Elements will in turn
	//call this method on each of it's children to give us a snap shot of the state for
	//each element on the page.

	if (m_state & UIElementState::Invisible || m_state & UIElementState::Idlee) return 0; //Invisible and Idle elements don't get updated

	//If the element can't be interacted with in any way then simple return the idle state.
	//if (!m_isClickable && !m_isHoverable && !m_isScrollable) m_state |= UIElementState::Idlee;

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
				//If the element isn't disabled then call its onMouseClick() method.
				//Even when a method is disabled it can still enter the clicked state
				if (!(m_state & UIElementState::Disabled)) onMouseClick();
				m_state |= UIElementState::Clicked; //add the clicked state to the element
			}
			else if (inputState->mouseClickState == MouseClickState::MouseRightClick)
			{
				//As of right now there are very few uses for right clicks with very
				//basic functionality so performing a right click won't effect the
				//current state of the UIElement
				if (!(m_state & UIElementState::Disabled)) onMouseRightClick();
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
		if (!(m_state & UIElementState::Disabled))
		{
			onMouseRelease();
			if (mouseHovered) m_state |= UIElementState::Released; //add the release state if the mouse is hovering the element (this triggers action elsewhere)
		}
		m_state &= ~UIElementState::Clicked; //remove the clicked state from the element
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

void UIElement::updateLocationAndSize(DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size)
{
	//Before doing anything, convert the absolute coordinates entered for the size and location
	//parameters to the relative coordinates needed to correct screen placement.
	m_size = size;
	m_location = location;

	if (m_useAbsoluteCoordinates) return; //Any elements using absolute coordinates don't need a conversion

	convertAbsoluteCoordinatesToRelativeCoordinates();

	//Using the maximum window sizes, calculate the relative distance that the right edge
	//of the box would be from a vertical centerline (in terms of percentage of window length)
	//and calculate the relative distance that the top edge of the box would be from a 
	//horizontal center lin (in terms of percentage of window height).
	DirectX::XMFLOAT2 element_pixel_center = { MAX_SCREEN_WIDTH * m_location.x, MAX_SCREEN_HEIGHT * m_location.y };
	DirectX::XMFLOAT2 screen_pixel_center = { MAX_SCREEN_WIDTH * 0.5f, MAX_SCREEN_HEIGHT * 0.5f };

	//Calculate the absolute distance between the element center and screen center when 
	//the screen size is at its maximum. A horizontal drift multiplier is also calculated,
	//which is the ratio of the horizontal pixel distance from the center of the element to the
	//center of the screen and the pixel distance of the element's height.
	float maximum_pixel_height = m_size.y / (MAX_SCREEN_WIDTH + MAX_SCREEN_HEIGHT) * (2 * MAX_SCREEN_WIDTH * MAX_SCREEN_HEIGHT);
	m_absoluteDistanceToScreenCenter.x = element_pixel_center.x - screen_pixel_center.x;
	m_horizontalDriftMultiplier = m_absoluteDistanceToScreenCenter.x / maximum_pixel_height;

	m_absoluteDistanceToScreenCenter.x /= MAX_SCREEN_WIDTH; //convert horizontal pixels to absolute units
	m_absoluteDistanceToScreenCenter.y = (element_pixel_center.y - screen_pixel_center.y) / MAX_SCREEN_HEIGHT;
}

void UIElement::convertAbsoluteCoordinatesToRelativeCoordinates()
{
	//When the relative box class is created the user gives its size and location in units relative to the
	//size of the screen. For example a size of (0.1, 0.1) means that user wants to create a shape that's
	//1/10th the height and 1/10th the width of the screen and a location of (0.5, 0.5) means the
	//element will be have its center in the middle of the screen. To make sure that elements retain
	//their shape and location when the screen is resized, their height is locked to a ratio of the 
	//current screen size, and their width will be some constant multiplier of the height.
	//Since the element is sized in this manner, the absolute coordinates input by the user won't give the 
	//exact results that are desired, so this method is used to convert them into the relative
	//coordinates that will give the desired result.
	float desired_pixel_height = m_size.y * MAX_SCREEN_HEIGHT;
	float actual_pixel_height = m_size.y / (MAX_SCREEN_WIDTH + MAX_SCREEN_HEIGHT) * (2 * MAX_SCREEN_WIDTH * MAX_SCREEN_HEIGHT);
	m_sizeMultiplier.y = desired_pixel_height / actual_pixel_height;

	//Since the relative height was just changed, the relative Y location
	//must also be changed to reflect this. This is done by taking the absolute
	//distance between the center of the element and the
	//(horizontal) center of the screen, multiplying it by the new relative
	//height variable (which is normalized to the height of the entire screen
	//by dividing it by the current element height) and then subtracting this
	//value from the absolute midpoint of the screen. The line below shows
	//that this reduces down to a very simple equation, although, it was actually
	//pretty tricky to figure out.
	m_location.y = 0.5f - m_sizeMultiplier.y * (0.5f - m_location.y);
	m_size.y *= m_sizeMultiplier.y;

	//Set the horizontal multiplier based on the original absolute coordinates
	float desired_pixel_width = m_size.x * MAX_SCREEN_WIDTH;
	m_sizeMultiplier.x = desired_pixel_width / desired_pixel_height;
}

DirectX::XMFLOAT2 UIElement::getAbsoluteSize()
{
	//If the UI Element uses relative coordinates instead of absolute coordinates
	//then the y component of the m_size variable needs to be put back to its
	//original value before returning.
	if (m_useAbsoluteCoordinates) return m_size;
	else return { m_size.x, m_size.y / m_sizeMultiplier.y };
}

DirectX::XMFLOAT2 UIElement::getPixelSize()
{
	//Returns the dimensions of the element as it would be rendered on screen 
	//given the current screen dimensions.
	if (m_useAbsoluteCoordinates) return { m_size.x * m_screenSize->Width, m_size.y * m_screenSize->Height };

	float pixel_height = m_size.y / (m_screenSize->Width + m_screenSize->Height) * (2 * m_screenSize->Width * m_screenSize->Height);
	return { m_sizeMultiplier.x * pixel_height, pixel_height };
}

void UIElement::setAbsoluteSize(DirectX::XMFLOAT2 size, bool resize_element)
{
	//The setAbsoluteSize() method is similar to the resize() method
	//in that it changes the size of the current UI Element. Unlike the
	//resize() method though (which gets called when the screen size
	//changes), the setAbsoluteSize() method must be overloaded for 
	//most different UIElements. Compound (and even individual) UI Elements
	//all have different shapes, and therefore centers. All UI Elements
	//are situated and resized about their centers, so it goes to follow
	//that manually resizing elements will need to be different depending
	//on they layout, size and location of any child elements. Since the
	//resize() method shifts all elements with the center of the screen 
	//as a reference, the logic is the exact same regardless of what the
	//element is, how many children it has, etc.
	
	//First, the size parameter (which is given in absolute coordinates)
	//is converted into relative coordinates and internal variables are
	//updated.
	updateLocationAndSize(getAbsoluteLocation(), size);

	//Then the element can choose how to handle resizing its children. As
	//mentioned above, since all UI Elements have different layouts and sizes
	//for their children, the setChildrenAbsoluteSize() method is virtual and
	//can be overriden by any UIElement class.
	setChildrenAbsoluteSize(size);

	//While this method will resize all elements with respect to themselves,
	//it does nothing for the element in terms if its relative size to the
	//window. To make sure that the newly resized element will stay in the 
	//correct location with respect to the current screen, the resize() 
	//method needs to be called at the end of it. Since resize() called on 
	//a top level parent element will automatically call the resize() method
	//for all of it's children, a boolean can be set to skip resize() calls
	//of child elements. The resize bool should be set to true at the highest
	//level call for an element (i.e. when called from outside of the class)
	if (resize_element) resize();
}

DirectX::XMFLOAT2 UIElement::getAbsoluteLocation()
{
	//If the UI Element uses relative coordinates instead of absolute coordinates
	//then the y component of the m_location variable needs to be shifted back to its
	//original value before returning.
	if (m_useAbsoluteCoordinates) return m_location;
	else return { m_location.x, 0.5f - (0.5f - m_location.y) / m_sizeMultiplier.y };
}

DirectX::XMFLOAT2 UIElement::getPixelLocation(DirectX::XMFLOAT2 pixel_shift)
{
	//NOTE: Although this method can be used on any UI Element, it should only be used on top level
	//parent UI Elements. Calculating the amount of pixel shift for child elements that would normally
	//be rendered outside of the screen needs to be done at the parent level (this is to ensure that
	//all children get shifted appropriately).

	//TODO: Consider adding a UIElement* field to the UIElement class which will keep track of the 
	//outer most parent for each child element.

	//This method uses the same logic that's in the resize method so comments are left our for 
	//brevity from this point on.
	DirectX::XMFLOAT2 element_pixel_center = { m_screenSize->Width * m_location.x, m_screenSize->Height * m_location.y };

	if (!m_useAbsoluteCoordinates)
	{
		DirectX::XMFLOAT2 element_pixel_dimensions = getPixelSize();

		float newPixelDistanceToHorizontal = m_horizontalDriftMultiplier * element_pixel_dimensions.y;
		float newPixelDistanceToVertical = m_absoluteDistanceToScreenCenter.y * element_pixel_dimensions.y / m_size.y;

		element_pixel_center.x += newPixelDistanceToHorizontal - m_absoluteDistanceToScreenCenter.x * m_screenSize->Width;
		element_pixel_center.y += newPixelDistanceToVertical - m_absoluteDistanceToScreenCenter.y * m_screenSize->Height;

		if ((pixel_shift.x == 0.0f) && (pixel_shift.y == 0.0f))
		{
			screenBoundaryCheck(element_pixel_center, element_pixel_dimensions, pixel_shift);
		}

		element_pixel_center.x += pixel_shift.x;
		element_pixel_center.y += pixel_shift.y;
	}

	return element_pixel_center;
}

void UIElement::setAbsoluteLocation(DirectX::XMFLOAT2 location)
{
	//In the same vein as the setAbsoluteSize() method, this method must
	//also set the location of all child elements. We don't apply the same
	//location to each child (as some children have locations offset from their
	//parents). Instead we compare the current location and the new location
	//to get a difference. This same difference is then applied to the children.
	DirectX::XMFLOAT2 originalAbsoluteLocation = getAbsoluteLocation();
	updateLocationAndSize(location, getAbsoluteSize());

	DirectX::XMFLOAT2 locationDifference = { location.x - originalAbsoluteLocation.x, location.y - originalAbsoluteLocation.y };

	for (int i = 0; i < p_children.size(); i++)
	{
		auto childAbsoluteLocation = p_children[i]->getAbsoluteLocation();
		p_children[i]->setAbsoluteLocation({ childAbsoluteLocation.x + locationDifference.x, childAbsoluteLocation.y + locationDifference.y });
	}
}

float UIElement::absoluteCompare(float pixelOne, float pixelTwo)
{
	//Since floating point numbers are used for all absolute coordinate related math then it's possible that
	//two coordinates will occupy the same space, but have slightly different values (for example
	//pixel one has an x location of 125.00235 and pixel two has an x location of 125.014).
	//Some actions require the comparison of two pixel locations to decide the outcome,
	//so instead of using raw <, <=, ==, >= and > symbols, this method should get called
	//instead.

	//If the pixels are within 0.001 of each other then they're considered equal. Otherwise,
	//a negative number means pixelTwo is larger while a positive number means that
	//pixelOne is largber.
	float difference = pixelOne - pixelTwo;
	if (difference <= 0.001f && difference >= -0.001f) return 0.0f;
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

void UIElement::setFontSize(float size)
{
	m_fontSize = size;
	/*if (m_useAbsoluteCoordinates) m_fontSize = size;
	else
	{
		m_fontSize = size / (m_screenSize->Width + m_screenSize->Height) * (2 * m_screenSize->Width * m_screenSize->Height);
	}*/
}