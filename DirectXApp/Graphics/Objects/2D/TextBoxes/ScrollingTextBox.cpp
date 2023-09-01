#include "pch.h"
#include "ScrollingTextBox.h"
#include "../UIButton.h"
#include "Modes/mode.h" //need access to ModeState enum class

ScrollingTextBox::ScrollingTextBox(DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring text, UIColor backgroundColor, winrt::Windows::Foundation::Size windowSize)
{
	//This is a compound UIElement, featuring two child buttons. There's also a number of 
	//UI shapes, including a white rectangle (and a border for it) where text is rendered,
	//some element text (which has the capability to move), and a foreground UI shape which
	//is used to cover up any text that appears outside of the rendering area.

	//The location and size variables passed in are for the actual text box, the size and location
	//of all other parts of the scroll box are derived from that.
	m_location = location;
	m_size = size;
	m_fontSize = 0.1 * size.y; //set the font height of the text box to be 1/10 the height of the text box
	m_textStart = { location.x - size.x / (float)2.0, location.y - size.y / (float)2.0, }; //the text always starts in the top left of the text box, only scrolling will change this
	m_scrollIntensity = 0.02;

	UIShape background({ 0, 0, 0, 0 }, UIColor::White, UIShapeFillType::Fill, UIShapeType::RECTANGLE); //The white background for the text
	UIShape outline({ 0, 0, 0, 0 }, UIColor::Black, UIShapeFillType::NoFill, UIShapeType::RECTANGLE); //Outline for the text box
	UIShape outline_shadow({ 0, 0, 0, 0 }, UIColor::Black, UIShapeFillType::NoFill, UIShapeType::RECTANGLE); //a second outline to make a slight shadow effect

	m_backgroundShapes.push_back(outline_shadow); //the shadow comes first so it can be drawn over
	m_backgroundShapes.push_back(background);
	m_backgroundShapes.push_back(outline);

	//One of the most important elements of the scroll box is a foreground object that's the same color as the 
	//background of the current page. This goes on top of the text box, so any text that scrolls up and out of 
	//the box will be covered up by this box
	UIShape cover({ 0, 0, 0, 0 }, backgroundColor, UIShapeFillType::Fill, UIShapeType::RECTANGLE);
	m_foregroundShapesNoOverlap.push_back(cover);

	//Add the up and down buttons. The top of the top button and the bottom of the bottom button align
	//with the top and bottom of the text box respectively. Both buttons are flush with the right
	//side of the text box.
	float button_height = 0.1 * size.y;
	float button_width = button_height * windowSize.Height / windowSize.Width; //to make button square we need to factor in difference in window height and width

	float buttonX = (location.x + size.x / (float)2.0 + button_width / (float)2.0); //the butt

	UIButton    top_button({ buttonX, location.y - size.y / (float)2.0 + button_height / (float)2.0 }, { button_width, button_height}, windowSize);
	UIButton bottom_button({ buttonX, location.y + size.y / (float)2.0 - button_height / (float)2.0 }, { button_width, button_height}, windowSize);

	top_button.setParent(this);
	bottom_button.setParent(this);

	p_children.push_back(std::make_shared<UIButton>(top_button));
	p_children.push_back(std::make_shared<UIButton>(bottom_button));

	//After adding the text box and buttons, we add in four more rectangles to fit inbetween the buttons. The first two of these
	//rectangles stretch all the way between the buttons. One is a fill and one is an outline. The next to rectangles represent
	//a percentage of the scroll bar and it's size will depend on how much of the total text in the text box is showing.
	UIShape scroll_rectangle_background_shadow({ 0, 0, 0, 0 }, UIColor::Black, UIShapeFillType::Fill, UIShapeType::RECTANGLE);
	UIShape scroll_rectangle_background({ 0, 0, 0, 0 }, UIColor::Gray, UIShapeFillType::Fill, UIShapeType::RECTANGLE);
	UIShape scroll_rectangle_shadow({ 0, 0, 0, 0 }, UIColor::Black, UIShapeFillType::Fill, UIShapeType::RECTANGLE);
	UIShape scroll_rectangle({ 0, 0, 0, 0 }, UIColor::PaleGray, UIShapeFillType::Fill, UIShapeType::RECTANGLE);

	m_backgroundShapes.push_back(scroll_rectangle_background_shadow);
	m_backgroundShapes.push_back(scroll_rectangle_background);
	m_backgroundShapes.push_back(scroll_rectangle_shadow);
	m_backgroundShapes.push_back(scroll_rectangle);

	//It's impossible to know how large to make the scroll rectangle until we know the height of the text to be rendered.
	//Once we know this value then we can set up the scroll rectangle.
	scrollRectangleInitialized = false;

	addText(text);
	resize(windowSize); //sets the appropriate sizes for both the rectangle and text

	//In order to lock the bottom of the scrolling text to the bottome of the text box, we need to know
	//the height of the text layout. Set the appropriate booleans to true in both this class and
	//the element text in order to update this height when the window gets resized.
	m_needTextRenderHeight = true;
	m_elementText[0].needDPI = true;
}

uint32_t ScrollingTextBox::addText(std::wstring text)
{
	//The text passed in get's converted into a Text class object and is added 
	//to the end of the elementText vector. If any text is currently in the vector it
	//will get cleared out. Added text will just be simple, black text.
	//The start location of the text rendering box and hte font size get filled out in
	//the resize method.
	
	if (m_elementText.size() > 0)
	{
		//we're overwriting existing text, we only need to change the message and leave everything else the same
		m_elementText[0].message = text;
		m_elementText[0].colorLocations.back() = text.length();
	}
	else
	{
		//Any part of the UIText that's dependent on the window size will be automatically set the
		//next time the resize() method is called.
		UIText newText(text, 0, { 0, 0 }, { 0, 0 }, { UIColor::Black }, { 0, text.length() },
			UITextType::ELEMENT_TEXT, UITextJustification::UpperLeft);
		m_elementText.push_back(newText);
	}

	return ModeState::NeedTextUpdate; //alert whoever added the text that the current mode needs to get new text height from the renderer class
	//return 16;
}

void ScrollingTextBox::resize(winrt::Windows::Foundation::Size windowSize)
{
	//First, resize the text box and outline
	DirectX::XMFLOAT2 center_point = { windowSize.Width * m_location.x, windowSize.Height * m_location.y };
	const D2D1_RECT_F rect = D2D1::RectF(
		center_point.x - windowSize.Width * m_size.x / (float)2.0,
		center_point.y - windowSize.Height * m_size.y / (float)2.0,
		center_point.x + windowSize.Width * m_size.x / (float)2.0,
		center_point.y + windowSize.Height * m_size.y / (float)2.0
	);
	m_backgroundShapes[0].m_rectangle = { rect.left + 1, rect.top + 1, rect.right + 1, rect.bottom + 1 }; //shadow is offset 1 pixel to the right and down from standard outline
	m_backgroundShapes[1].m_rectangle = rect;
	m_backgroundShapes[2].m_rectangle = rect; 

	//the foreground cover box is the same width as the text box and located directly above it. It streches
	//all the way to the top of the page.
	m_foregroundShapesNoOverlap[0].m_rectangle = { rect.left, 0, rect.right, rect.top };

	//To acheive the illusion of text scrolling, the starting location of the text drifts upwards and
	//out of the text box (where it get's covered by foreground UI shapes. The bottom of the rendering
	//area for text remains glued to the bottom of the text box to make sure text is clipped properly.
	m_elementText[0].startLocation = { m_textStart.x * windowSize.Width, m_textStart.y * windowSize.Height }; //start location for text moves with the window, not the text box
	m_elementText[0].renderArea = { rect.right - m_elementText[0].startLocation.x, rect.bottom - m_elementText[0].startLocation.y }; //bottom of text rendering area is glued to the bottom of the text box
	m_elementText[0].fontSize = windowSize.Height * m_fontSize;

	//set the amount of pixels the text will move with each tick of the mouse wheel
	textPixelsPerScroll = m_scrollIntensity * windowSize.Height;

	//check to see if text needs to be repositioned as a result of changing the screen size
	repositionElementText(windowSize);

	//After resizing the shapes owned directly by the scroll box, we need to resize the button UI Elements as well.
	for (int i = 0; i < p_children.size(); i++) p_children[i]->resize(windowSize);

	//After the buttons are resized, start resizing the rectangular scroll bar
	auto topButtonPixels = p_children[0]->getPixels(RenderOrder::Background, 0);
	auto bottomButtonPixels = p_children[1]->getPixels(RenderOrder::Background, 0);

	m_backgroundShapes[3].m_rectangle = { rect.right, topButtonPixels.bottom - 1, topButtonPixels.right, bottomButtonPixels.top };
	m_backgroundShapes[4].m_rectangle = { m_backgroundShapes[3].m_rectangle.left + 1, m_backgroundShapes[3].m_rectangle.top + 1, m_backgroundShapes[3].m_rectangle.right - 2, m_backgroundShapes[3].m_rectangle.bottom - 2 };
	m_backgroundShapes[5].m_rectangle = { rect.right + 1, windowSize.Height * (scrollRectangleAbsoluteLocation - scrollRectangleAbsoluteHeight / (float)2.0) + 2, m_backgroundShapes[4].m_rectangle.right, windowSize.Height * (scrollRectangleAbsoluteLocation + scrollRectangleAbsoluteHeight / (float)2.0) };
	m_backgroundShapes[6].m_rectangle = { m_backgroundShapes[5].m_rectangle.left + 1, m_backgroundShapes[5].m_rectangle.top + 1, m_backgroundShapes[5].m_rectangle.right - 2, m_backgroundShapes[5].m_rectangle.bottom - 2 };
}

void ScrollingTextBox::repositionElementText(winrt::Windows::Foundation::Size windowSize)
{
	//It's possible that at one point the screen was small enough that not all text could fit in the text box,
	//but we've since made the screen larger and now all the text should fit, but it's no longer all in the box.
	//Perform a quick check to make sure that the text is all in the correct place.
	if (m_elementText[0].startLocation.y + m_elementText[0].renderDPI.y < m_backgroundShapes[0].m_rectangle.bottom)
	{
		if (m_elementText[0].startLocation.y < m_backgroundShapes[0].m_rectangle.top)
		{
			//we need to scootch the text downwards until the bottom of the text is as low as it can go.
			//Whichever distance is less, either the bottom of the render rectangle to the bottom of the text
			//box, or the top of the render rectangle to the top of the text box, move the render rectangle
			//downwards by that amount.
			float lesserDistance = (m_backgroundShapes[0].m_rectangle.top - m_elementText[0].startLocation.y) < (m_backgroundShapes[0].m_rectangle.bottom - (m_elementText[0].startLocation.y + m_elementText[0].renderDPI.y)) ?
				m_backgroundShapes[0].m_rectangle.top - m_elementText[0].startLocation.y : m_backgroundShapes[0].m_rectangle.bottom - (m_elementText[0].startLocation.y + m_elementText[0].renderDPI.y);

			m_elementText[0].startLocation.y += lesserDistance;
			m_textStart.y += lesserDistance / windowSize.Height;
			m_elementText[0].renderArea.y -= lesserDistance;
		}
	}
}

//the StaticTextBox class has nothing to update but this pure virtual method must be implemented
UIElementState ScrollingTextBox::update(InputState* inputState)
{
	if (!scrollRectangleInitialized)
	{
		//We need to know the size of the text in the scroll box before we can create the scroll progress
		//rectangle. The first time this update method get's called we should have that information and
		//future updates to the rectangle can be handled by the resize() method.
		initializeScrollProgressRectangle();

		scrollRectangleInitialized = true;
	}

	if (inputState->scrollWheelDirection != 0 && checkHover(inputState->mousePosition))
	{
		if (inputState->scrollWheelDirection < 0) onScrollDown();
		else onScrollUp();
	}

	//After checking for a scroll event, see if either of the buttons are being clicked. Clicking one
	//of the buttons has the effect of scrolling the box twice in the appropriate direction.
	if (p_children[0]->update(inputState) == UIElementState::Clicked)
	{
		onScrollUp();
		onScrollUp();
		m_state = UIElementState::Clicked;
		return UIElementState::Clicked;
	}
	else if (p_children[1]->update(inputState) == UIElementState::Clicked)
	{
		onScrollDown();
		onScrollDown();
		m_state = UIElementState::Clicked;
		return UIElementState::Clicked;
	}
	return UIElementState::Idle; //if nothing happened this update then return the idle state
}

void ScrollingTextBox::initializeScrollProgressRectangle()
{
	//First get the current window height by comparing the current height of the text box vs. it's absolute height
	float windowHeight = (m_backgroundShapes[1].m_rectangle.bottom - m_backgroundShapes[1].m_rectangle.top) / m_size.y;
	float scrollBarBacgkroundAbsoluteHeight = (m_backgroundShapes[4].m_rectangle.bottom - m_backgroundShapes[4].m_rectangle.top) / windowHeight;

	//If the text doesn't take up the full text box then make the rectangle 95% of the full height of the background rectangle.
	//Otherwise, the height should be the same as the ratio of the text box height to the total text height
	if (m_elementText[0].renderDPI.y <= (m_backgroundShapes[1].m_rectangle.bottom - m_backgroundShapes[1].m_rectangle.top)) scrollRectangleAbsoluteHeight = 0.98 * scrollBarBacgkroundAbsoluteHeight;
	else scrollRectangleAbsoluteHeight = ((m_backgroundShapes[1].m_rectangle.bottom - m_backgroundShapes[1].m_rectangle.top) / m_elementText[0].renderDPI.y) * scrollBarBacgkroundAbsoluteHeight;

	//Create the shadow first
	m_backgroundShapes[5].m_rectangle = { m_backgroundShapes[1].m_rectangle.right + 1, m_backgroundShapes[4].m_rectangle.top + 2, m_backgroundShapes[4].m_rectangle.right, 0 };
	m_backgroundShapes[5].m_rectangle.bottom = (m_backgroundShapes[4].m_rectangle.bottom - m_backgroundShapes[4].m_rectangle.top) * scrollRectangleAbsoluteHeight / scrollBarBacgkroundAbsoluteHeight + m_backgroundShapes[5].m_rectangle.top;

	//then the fill
	m_backgroundShapes[6].m_rectangle = { m_backgroundShapes[5].m_rectangle.left + 1, m_backgroundShapes[5].m_rectangle.top + 1, m_backgroundShapes[5].m_rectangle.right - 2, m_backgroundShapes[5].m_rectangle.bottom - 2 };

	//set the scroll rectangle absolute location, this is necessary for when the window changes sizes. The absolute size is just the y-coordinate of the top
	//of the rectangle compared to the height of the window.
	scrollRectangleAbsoluteCeiling = m_location.y - (scrollBarBacgkroundAbsoluteHeight - scrollRectangleAbsoluteHeight) / (float)2.0;
	scrollRectangleAbsoluteLocation = scrollRectangleAbsoluteCeiling; //the scroll bar starts all the way at the top

	//the lowest location that the scroll rectangle can go is the same as the starting position, just flipped around the y-axis,
	//relative to the center of the text box
	scrollRectangleAbsoluteFloor = m_location.y + (scrollBarBacgkroundAbsoluteHeight - scrollRectangleAbsoluteHeight) / (float)2.0;
}

bool ScrollingTextBox::checkHover(DirectX::XMFLOAT2 mousePosition)
{
	//For the scroll box element we allow scrolling as long as the mouse is anywhere inside of the box. That includes the text box,
	//the buttons, or the progress bar.
	if (m_backgroundShapes.size() == 0) return false;

	//For the mouse to be considered "in" the button it needs to be within the outline of it, not the shadow
	if ((mousePosition.x >= m_backgroundShapes[1].m_rectangle.left) &&
		(mousePosition.x <= p_children[1]->getPixels(RenderOrder::Background, 0).right) &&
		(mousePosition.y >= m_backgroundShapes[1].m_rectangle.top) &&
		(mousePosition.y <= m_backgroundShapes[1].m_rectangle.bottom))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ScrollingTextBox::onHover()
{
	//We don't actually change anything when hovering over a scrolling text box, it just allows
	//us to actually scroll.
}

void ScrollingTextBox::onScrollUp()
{
	//The render area can't get any smaller than the height of the text box
	if (m_elementText[0].renderDPI.y > (m_backgroundShapes[1].m_rectangle.bottom - m_backgroundShapes[1].m_rectangle.top))
	{
		if (m_elementText[0].startLocation.y + textPixelsPerScroll > m_backgroundShapes[1].m_rectangle.top)
		{
			m_textStart.y = m_location.y - m_size.y / (float)2.0;
			m_elementText[0].startLocation.y = m_backgroundShapes[1].m_rectangle.top;
			m_elementText[0].renderArea.y = m_backgroundShapes[1].m_rectangle.bottom - m_backgroundShapes[1].m_rectangle.top;
		}
		else
		{
			//First move the text
			m_textStart.y += m_scrollIntensity;
			m_elementText[0].startLocation.y = m_elementText[0].startLocation.y + textPixelsPerScroll;
			m_elementText[0].renderArea.y = m_elementText[0].renderArea.y - textPixelsPerScroll; //bottom of text rendering area is glued to the bottom of the text box

			//Then move the scroll progress bar
			calculateScrollBarLocation();
		}
	}
}

void ScrollingTextBox::onScrollDown()
{
	//Scrolling down causes the starting point for the text to move upwards and out of the text box.
	//We can only scroll down until the bottom of the text rendering rectangle reaches the same height
	//as the bottom of the text box. To check this, the full height of the text rendering rectangle 
	//is calculated when the scroll box is first created (and every time the window gets resized). The
	//calculation for this height happens outside of this class.

	//Only attempt to scroll if not all text can fit in the text box.
	if (m_elementText[0].renderDPI.y > (m_backgroundShapes[1].m_rectangle.bottom - m_backgroundShapes[1].m_rectangle.top))
	{
		if (m_elementText[0].renderArea.y + textPixelsPerScroll > m_elementText[0].renderDPI.y)
		{
			m_elementText[0].renderArea.y = m_elementText[0].renderDPI.y;
			m_elementText[0].startLocation.y = m_backgroundShapes[1].m_rectangle.bottom - m_elementText[0].renderDPI.y;

			//We need to calculate the absolute height for the text start (in terms of a ratio of the
			//current window size). To do this, we calculate the window height by looking at the ratio
			//of the current text box height vs. it's window height ratio.
			float currentWindowHeight = (m_backgroundShapes[1].m_rectangle.bottom - m_backgroundShapes[1].m_rectangle.top) / m_size.y;

			m_textStart.y = m_elementText[0].startLocation.y / currentWindowHeight; //need to calculate this exact number
		}
		else
		{
			//First move the text
			m_textStart.y -= m_scrollIntensity;
			m_elementText[0].startLocation.y = m_elementText[0].startLocation.y - textPixelsPerScroll;
			m_elementText[0].renderArea.y = m_elementText[0].renderArea.y + textPixelsPerScroll; //bottom of text rendering area is glued to the bottom of the text box

			//Then move the scroll progress bar
			calculateScrollBarLocation();
		}
	}
}

void ScrollingTextBox::calculateScrollBarLocation()
{
	//Then move the scroll rectangle (moves in the opposite direction as the text). To move the appropriate distance
	//we need to calculate what percentage of the way we are through the text. This is done by comparing the location
	//of the bottom of the text box when the text is all the way down, vs. the current location of the bottom of 
	//the text box.
	float textLayoutFloor = m_elementText[0].renderDPI.y + m_backgroundShapes[1].m_rectangle.top;
	float textLayoutCeiling = m_backgroundShapes[1].m_rectangle.bottom;
	float textLayoutCurrentLocation = m_elementText[0].renderDPI.y + m_elementText[0].startLocation.y;

	float currentPercentage = (textLayoutFloor - textLayoutCurrentLocation) / (textLayoutFloor - textLayoutCeiling);

	//We move the scroll bar by the calculated percentage from its highest location to its lowest location
	float temp = scrollRectangleAbsoluteLocation;
	scrollRectangleAbsoluteLocation = scrollRectangleAbsoluteCeiling + currentPercentage * (scrollRectangleAbsoluteFloor - scrollRectangleAbsoluteCeiling);

	//calculate how many pixels the scroll bar should move
	float currentWindowHeight = (m_backgroundShapes[1].m_rectangle.bottom - m_backgroundShapes[1].m_rectangle.top) / m_size.y;
	float pixels_moved = (scrollRectangleAbsoluteLocation - temp) * currentWindowHeight;

	//finally move the scroll bar
	m_backgroundShapes[5].m_rectangle.top += pixels_moved;
	m_backgroundShapes[5].m_rectangle.bottom += pixels_moved;
	m_backgroundShapes[6].m_rectangle.top += pixels_moved;
	m_backgroundShapes[6].m_rectangle.bottom += pixels_moved;
}

void ScrollingTextBox::setState(UIElementState state)
{
	//The only state that the scroll box currently enters is clicked when one of it's buttons gets clicked.
	//When this method is changed we update the scroll box's state as well as the buttons.
	m_state = state;
	p_children[0]->setState(state);
	p_children[1]->setState(state);
}