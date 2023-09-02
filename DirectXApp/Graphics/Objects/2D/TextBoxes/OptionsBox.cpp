#include "pch.h"
#include "OptionsBox.h"
#include "HighlightableTextOverlay.h"
#include "Modes/mode.h"

OptionsBox::OptionsBox(DirectX::XMFLOAT2 location, float height, std::wstring text, UIColor backgroundColor, winrt::Windows::Foundation::Size windowSize)
{
	//The location and size variables passed in are for the actual text box, the size and location
	//of all other parts of the option box are derived from that.
	m_location = location;
	m_size.x = 0.0; //this will get overriden at a later point
	m_size.y = height; //the width of the option box is dictated by the longest option in the text, so we only set the height of the box
	m_fontSize = 0.12 * m_size.y; //set the font height of the text box to be 1/10 the height of the text box
	//m_textStart = { location.x - m_size.x / (float)2.0, location.y - size.y / (float)2.0, }; //the text always starts in the top left of the text box, only scrolling will change this
	//m_scrollIntensity = 0.02;

	//We won't know anything about the width of the box until we figure out which text option is the widest. This won't happen until after the
	//text box is initialized and the text width is calculated by the master renderer. For now, we just create all of the necessary
	//shape objects.
	UIShape background({ 0, 0, 0, 0 }, UIColor::White, UIShapeFillType::Fill, UIShapeType::RECTANGLE); //The white background for the text
	UIShape outline({ 0, 0, 0, 0 }, UIColor::Black, UIShapeFillType::NoFill, UIShapeType::RECTANGLE); //Outline for the text box
	UIShape outline_shadow({ 0, 0, 0, 0 }, UIColor::Black, UIShapeFillType::NoFill, UIShapeType::RECTANGLE); //a second outline to make a slight shadow effect

	m_backgroundShapes.push_back(outline_shadow); //the shadow comes first so it can be drawn over
	m_backgroundShapes.push_back(background);
	m_backgroundShapes.push_back(outline);

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

	addText(text, windowSize);
	//resize(windowSize); //sets the appropriate sizes for both the rectangle and text

	//In order to lock the bottom of the scrolling text to the bottome of the text box, we need to know
	//the height of the text layout. Set the appropriate booleans to true in both this class and
	//the element text in order to update this height when the window gets resized.
	m_needTextRenderHeight = true;
}

uint32_t OptionsBox::addText(std::wstring text, winrt::Windows::Foundation::Size windowSize)
{
	//We can either add one option at a time to the OptionsBox, or multiple. If multiple options
	//are passed in at the same time, it's done with a single string where each of the options is
	//separated by a '\n' character. Each individual option will get placed as a separate string 
	//in the m_options vector.

	//Every time a new option(s) is added to the box, we need to figure out whether or not its
	//width is longer than the current longest option since the longest option dictates the 
	//width of the scroll box itself.

	//First split the string by the newline charcter and add all new options onto the back of
	//the existing vector.
	int i = 0, j = 0;
	while (j != std::string::npos)
	{
		j = text.find(L'\n', i);

		std::wstring option = text.substr(i, j - i);
		m_options.push_back(option);

		//we also create a HighlightableTextOverlay for each option and add it to the
		//list of children elements. When first creating the text overlay set the layout size to be the
		//same size as the screen, this will allow us to get the correct text width.
		HighlightableTextOverlay childElement(option, { UIColor::Black }, { 0, (unsigned int)option.length() }, { 0, 0 }, { windowSize.Width, windowSize.Height }, m_fontSize * windowSize.Height, UITextType::ELEMENT_TEXT, UITextJustification::CenterLeft);
		((UIText*)childElement.getRenderItem(RenderOrder::TextOverlay, 0))->needDPI = true;
		childElement.setNeedTextRenderHeight(true);
		childElement.updateSecondaryColor(UIColor::Gray);
		p_children.push_back(std::make_shared<HighlightableTextOverlay>(childElement));

		i = j + 1;
	}

	needTextResize = true; //alerts the update function that we need to potentially resize our box width because of the new text
	return ModeState::NeedTextUpdate; //alert whoever added the text that the current mode needs to get new text height from the renderer class
}

void OptionsBox::resize(winrt::Windows::Foundation::Size windowSize)
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

	//Resize text
	setOptionText(windowSize);

	//check to see if text needs to be repositioned as a result of changing the screen size
	//repositionElementText(windowSize); TODO: Uncomment this when ready

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

void OptionsBox::initializeScrollProgressRectangle()
{
	//This method overrides the Scrollbox version. Not only does this method create the scroll progress 
	//rectangle, but it also sets the width of the main text box. We don't know what the width should
	//be until this method first gets called.
	
	//First, see which option has the widest width in pixels and set the m_optionsBoxWidth based on this.
	for (int i = 0; i < p_children.size(); i++)
	{
		UIText* overlayText = ((UIText*)p_children[i]->getRenderItem(RenderOrder::TextOverlay, 0));
		float textLayoutWidthInPixels = overlayText->renderDPI.x;
		if (textLayoutWidthInPixels / overlayText->renderArea.x > m_size.x)
		{
			//set the width of the text box in terms of a percentage of the screen width
			m_size.x = textLayoutWidthInPixels / overlayText->renderArea.x; //the current renderArea is the same width as the sceen at this stage
		}
	}

	//Add a little padding at the end of the longest word so it isn't right up against the border
	m_size.x += 0.01;

	//Get the size of the current window from one of the child TextOverlay objects
	winrt::Windows::Foundation::Size windowSize = { ((UIText*)p_children[0]->getRenderItem(RenderOrder::TextOverlay, 0))->renderArea.x, ((UIText*)p_children[0]->getRenderItem(RenderOrder::TextOverlay, 0))->renderArea.y };

	//In order to make everything look clean, set the absolute height of the scroll box to 
	//be a multiple of the text height of the options. Round down when doing this. We also
	//change the absolute font size
	m_fontSize /= m_size.y; //get the font size as a percentage of the text box height
	optionsDisplayed = m_size.y * windowSize.Height / ((UIText*)p_children[0]->getRenderItem(RenderOrder::TextOverlay, 0))->renderDPI.y; //the division gets floored automatically here by converting to an int
	m_size.y = (float)optionsDisplayed * ((UIText*)p_children[0]->getRenderItem(RenderOrder::TextOverlay, 0))->renderDPI.y / windowSize.Height; //no floor division happens because we're using floats
	m_fontSize *= m_size.y; //change font back into a percentage of the over window height

	//Now that we know that maximum width, we can safely start creating the rest of the box. First, we need
	//to create the scroll box buttons and insert them into the child array before the TextOverlay objects.
	float button_height = 0.1 * m_size.y;
	float button_width = button_height * windowSize.Height / windowSize.Width; //to make button square we need to factor in difference in window height and width

	float buttonX = (m_location.x + m_size.x / (float)2.0 + button_width / (float)2.0); //the butt

	UIButton    top_button({ buttonX, m_location.y - m_size.y / (float)2.0 + button_height / (float)2.0 }, { button_width, button_height }, windowSize);
	UIButton bottom_button({ buttonX, m_location.y + m_size.y / (float)2.0 - button_height / (float)2.0 }, { button_width, button_height }, windowSize);

	top_button.setParent(this);
	bottom_button.setParent(this);

	p_children.insert(p_children.begin(), std::make_shared<UIButton>(bottom_button));
	p_children.insert(p_children.begin(), std::make_shared<UIButton>(top_button));

	topOption = 2; //Since the buttons are now the first two child elements, the first option has moved to element 2
	
	//Set the actual pixel sizes for everything based on the current window size
	resize(windowSize);

	//Once the text options have been creted we need to make sure that only the first few are visible.
	for (int i = topOption + optionsDisplayed; i < p_children.size(); i++) p_children[i]->setState(UIElementState::Invisible);
}

void OptionsBox::setOptionText(winrt::Windows::Foundation::Size windowSize)
{
	//Based on the current size of the text box and the current top option,
	//set the render locations for all visible options.

	float fontHeight = ((UIText*)p_children[topOption]->getRenderItem(RenderOrder::TextOverlay, 0))->renderDPI.y;
	for (int i = 2; i < p_children.size(); i++)
	{
		//Then update the window screen dependent variables
		((HighlightableTextOverlay*)p_children[i].get())->updateLocation({ m_location.x - m_size.x / (float)2.0, m_location.y - m_size.y / (float)2.0 + (i - topOption) * fontHeight / windowSize.Height }); //The location for a text overlay is the top left pixel
		((HighlightableTextOverlay*)p_children[i].get())->updateSize({ m_size.x - (float)0.01, fontHeight / windowSize.Height }); //add a little padding so words aren't right up against the border
		((HighlightableTextOverlay*)p_children[i].get())->updateFontSize(m_fontSize);

		p_children[i]->resize(windowSize);
	}
}

UIElementState OptionsBox::update(InputState* inputState)
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

	//Check the visible options to see if any of the text is being hovered over
	int stop = (topOption + optionsDisplayed - 2) < m_options.size() ? topOption + optionsDisplayed : p_children.size(); //TODO: in the onScroll methods, make it so that topOption + optionsDisplayed can't exceed the existing options
	for (int i = topOption; i < stop; i++) p_children[i]->update(inputState);

	return UIElementState::Idle; //if nothing happened this update then return the idle state
}

void OptionsBox::onScrollUp()
{
	//When Scrolling up, we first check to see if there are any option in the child array that are less than the current top option.
	//If there aren't then it means we're already at the top of the scroll box so we do nothing. Since the first two children of 
	//the options box ui element are buttons, the highest up option will always appear at index 2 of the child array.
	if (topOption > 2)
	{
		//simply loop through all of the options currently displayed and have each display option "steal"
		//the coordinates of the one in front of it. This will effectively scroll every option up by 1. We
		//should never get an index out of bounds here because if all options are displayed in the box then 
		//the current topOption will have a value of 2 and this if block won't execute.

		p_children[topOption + optionsDisplayed - 1]->setState(UIElementState::Invisible); //The current bottom option will no longer be visible
		topOption--; //decrement the top option by one
		p_children[topOption]->setState(UIElementState::Idle); //The new top option is now visible

		//To actually move the options we'll need to know the current size of the screen. This can be calculated
		//by comparing the absolute size of the scroll box to its current size in pixels.
		winrt::Windows::Foundation::Size currentWindowSize;
		currentWindowSize.Height = (m_backgroundShapes[1].m_rectangle.bottom - m_backgroundShapes[1].m_rectangle.top) / m_size.y;
		currentWindowSize.Width = (m_backgroundShapes[1].m_rectangle.right - m_backgroundShapes[1].m_rectangle.left) / m_size.x;

		for (int i = topOption; i < topOption + optionsDisplayed; i++)
		{
			((HighlightableTextOverlay*)p_children[i].get())->updateLocation(p_children[i + 1]->getLocation()); //update the absolute coordinates of the option

			//After moving the absolute position of the option, we need to move its pixles to match the new location.
			p_children[i]->resize(currentWindowSize);
		}

		//After shifting all of the options move the scroll bar accordingly
		//calculateScrollBarLocation();
	}
}

void OptionsBox::onScrollDown()
{
	//When Scrolling down, we first compare the current option at the top of the box vs. the number of options below it. If all of the options
	//below the current top option fit in the box then there's no reason to move the text upwards and we do nothing.
	if (topOption + optionsDisplayed < p_children.size())
	{
		//simply loop through all of the options currently displayed and have each display option "steal"
		//the coordinates of the one behind it. This will effectively scroll every option down by 1.

		p_children[topOption]->setState(UIElementState::Invisible); //The current top option will no longer be visible
		topOption++; //increment the top option by one
		p_children[topOption + optionsDisplayed - 1]->setState(UIElementState::Idle); //The current bottom option will now be visible

		//To actually move the options we'll need to know the current size of the screen. This can be calculated
		//by comparing the absolute size of the scroll box to its current size in pixels.
		winrt::Windows::Foundation::Size currentWindowSize;
		currentWindowSize.Height = (m_backgroundShapes[1].m_rectangle.bottom - m_backgroundShapes[1].m_rectangle.top) / m_size.y;
		currentWindowSize.Width = (m_backgroundShapes[1].m_rectangle.right - m_backgroundShapes[1].m_rectangle.left) / m_size.x;
		
		for (int i = topOption + optionsDisplayed - 1; i >= topOption; i--)
		{
			((HighlightableTextOverlay*)p_children[i].get())->updateLocation(p_children[i - 1]->getLocation());  //update the absolute coordinates of the option

			//After moving the absolute position of the option, we need to move its pixles to match the new location.
			p_children[i]->resize(currentWindowSize);
		}

		//After shifting all of the options move the scroll bar accordingly
		//calculateScrollBarLocation();
	}
}

uint32_t OptionsBox::addText(std::wstring text)
{
	//For now don't use this method, this is just here to override the IScrollableUI addText() method.
	return 0;
}