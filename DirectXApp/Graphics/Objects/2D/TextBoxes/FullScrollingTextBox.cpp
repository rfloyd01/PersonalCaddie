#include "pch.h"
#include "FullScrollingTextBox.h"
#include "HighlightableTextOverlay.h"
#include "Graphics/Objects/2D/Buttons/ArrowButton.h"
#include <cmath>

/*
Order of Child Elements:
0: Main Text Box
1: Scroll Progress Bar Background
2: Up Scrolling Button
3: Down Scrolling Button
4: Scroll Progress Bar Foreground
5+: Text Overlays with content
*/

FullScrollingTextBox::FullScrollingTextBox(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring message, float fontSize,
	bool highlightableText, bool dynamicSize, UITextJustification justification, UIColor textFillColor, bool isSquare, UIColor outlineColor, UIColor shadowColor)
{
	m_screenSize = windowSize;

	//First set the highlightable text and dynamic size variables
	m_highlightableText = highlightableText;
	m_dynamicSize = dynamicSize;

	//Then set the screen size dependent variables. The size variable
	//encompasses the text box, as well as the buttons and scroll bar
	//which means that the center of the element will be offset a little
	//bit from the actual center of the text box.
	updateLocationAndSize(location, size);
	m_fontSize = fontSize;

	//A scrolling text box features two arrow buttons, one up and one down.
	//Both of these buttons are squares. Create these elements first to 
	//help eith placement and sizing of the other elements.
	m_buttonHeight = m_buttonRatio * size.y;
	float screen_ratio = MAX_SCREEN_HEIGHT / MAX_SCREEN_WIDTH;
	float buttonWidth = screen_ratio * m_buttonHeight;

	ArrowButton upButton(windowSize, { location.x + (size.x - buttonWidth) / 2.0f, location.y - (size.y - m_buttonHeight) / 2.0f }, { buttonWidth, m_buttonHeight }, false, 5.0f);
	ArrowButton downButton(windowSize, { location.x + (size.x - buttonWidth) / 2.0f, location.y + (size.y - m_buttonHeight) / 2.0f }, { buttonWidth, m_buttonHeight }, true, 5.0f);

	//If the dynamic size variable is chosen then we won't actually know the width
	//of the text box until after we get the text pixel dimensions from the renderer.
	//Either way though, the textBox is the first child element.
	OutlinedBox textBox(windowSize, { location.x - buttonWidth / 2.0f, location.y }, { size.x - buttonWidth, size.y });

	//Finally, there's a progress bar between the buttons that shows how much of the text has
	//been scrolled through. This progress bar is composed of two elements: an outlined box that
	//acts as a background, and a shadowed box which shows the actual progress.
	OutlinedBox progressBarBackground(windowSize, { location.x + (size.x - buttonWidth) / 2.0f, location.y}, { buttonWidth, size.y}, UIColor::Gray);
	Button progressBarForeground(windowSize, { location.x + (size.x - buttonWidth) / 2.0f, location.y }, { buttonWidth, (size.y - 2.0f * m_buttonHeight) / 2.0f }, UIColor::PaleGray); //y size and location will change after text is added
	progressBarForeground.updateState(UIElementState::Disabled); //this is a button that can't be pressed

	p_children.push_back(std::make_shared<OutlinedBox>(textBox));
	p_children.push_back(std::make_shared<OutlinedBox>(progressBarBackground));
	p_children.push_back(std::make_shared<ArrowButton>(upButton));
	p_children.push_back(std::make_shared<ArrowButton>(downButton));
	p_children.push_back(std::make_shared<Button>(progressBarForeground));

	//Set values for class variables
	m_isScrollable = true; //this enables scrolling detection in the main rendering loop
	m_needTextRenderDimensions = true; //alerts the current mode that this element will need text pixels from the renderer at some point

	m_isScrollable = true; //this enables scrolling detection in the main rendering loop
	m_needTextRenderDimensions = true; //alerts the current mode that this element will need text pixels from the renderer at some point
	m_topText = 5; //Set the first line of text as the current top
	m_lastSelectedText = L"";
	m_state &= ~UIElementState::Dummy; //The default constructor wasn't used so this isn't a dummy element

	//Once everything is set we add text to the text box
	addText(message, m_highlightableText, false);
}

void FullScrollingTextBox::addText(std::wstring message, bool highlightable, bool existingText)
{
	//Unlike the partial scrolling text box, the full scrolling text box features multiple
	//different text elements. this constructor expects a single input string that's delimited with '\n' characters.
	//Split the string by the newline charcter and create a new textOverlay object from each one
	DirectX::XMFLOAT2 textLocation = p_children[0]->getAbsoluteLocation(), textSize = p_children[0]->getAbsoluteSize();

	if (m_dynamicSize && !existingText)
	{
		//Make the text overlay take up most of the screen to ensure we get the exact
		//height for the line of text. If we use the whole screen then the UI Element
		//class will automatically shrink the element (because it goes beyond the edge
		//threshold defined in the UI Element class) and cause issues in font size.
		textLocation = { 0.5f, 0.5f };
		textSize = { 0.75f, 0.75f }; //Make very large so the odds of rendering only single lines are large
	}

	//Before adding text, see if there's any text yet. If there is, and it's just an empty string
	//then we remove this element.
	if (p_children.size() == 6)
	{
		if (p_children[5]->getText()->message == L"")
		{
			//this is a placeholder element for resizing the rest of the box. We can safely delete it here.
			p_children[5] = nullptr;
			p_children.erase(p_children.begin() + 5);
			existingText = false;
		}
	}

	float existingTextHeight = 0;
	if (existingText)
	{
		//since there's already existing text in the box then we know exactly where the new text should go.
		//Calcualte the absolute height of the current text and use this as an adder when putting the new
		//text in the text box.
		textSize = p_children.back()->getAbsoluteSize(); //the size of the last line of text
		existingTextHeight = textSize.y / p_children.back()->getText()->renderLines;

		textLocation = { p_children.back()->getAbsoluteLocation().x, p_children.back()->getAbsoluteLocation().y + (textSize.y + existingTextHeight) / 2.0f }; //the location of the last line of text
	}

	int i = 0, j = 0, currentLine = 0;

	float adjustedFontHeight = m_fontSize * getAbsoluteSize().y / textSize.y; //font height is based on textbox height, not text overlay height

	while (j != std::string::npos)
	{
		j = message.find(L'\n', i);

		std::wstring textLine = message.substr(i, j - i);
		i = j + 1; //increment the i variable after finding the next line of text

		//Text defaults to a single black color that's left justified. When first creating the text, set the location to the center
		//of the screen and the size to be the same as the screen size. This will get overriden but it helps to determine the
		//correct text height.

		if (highlightable)
		{
			HighlightableTextOverlay newText(m_screenSize, { textLocation.x, textLocation.y + currentLine * existingTextHeight}, textSize, textLine, adjustedFontHeight,
				{ UIColor::Black }, { 0, (unsigned int)textLine.length() }, UITextJustification::CenterLeft, false);
			p_children.push_back(std::make_shared<HighlightableTextOverlay>(newText));
		}
		else
		{
			TextOverlay newText(m_screenSize, { textLocation.x, textLocation.y + currentLine * existingTextHeight }, textSize, textLine, adjustedFontHeight,
				{ UIColor::Black }, { 0, (unsigned int)textLine.length() }, UITextJustification::CenterLeft, false);
			p_children.push_back(std::make_shared<TextOverlay>(newText));
		}

		i = j + 1;
		currentLine++;
	}

	//With the difference between relative coordinates, absolute coordinates, and physical pixel sizes
	//it can get a little bit tricky to keep track of what the correct font size should be for the text
	//inside of the text box. To make things easier on us, simply save the pixel height of the font when
	//the text overlays are first created. This will allow us to dynamically update the absolute font size
	//to achieve this same pixel size later on
	m_initialFontPixelSize = adjustedFontHeight * p_children[5]->getPixelSize().y;

	m_state |= UIElementState::NeedTextPixels; //Let's the renderer know that we currently need the pixel size of text
}

void FullScrollingTextBox::clearText()
{
	//Removes all text child elements from the full scrolling text box except for the first
	//one (as it's needed in resizing methods). Set the text of that element to blank and
	//remove the other elements.
	p_children[5]->getText()->message = L"";
	for (int i = 6; i < p_children.size(); i++) p_children[i] = nullptr;
	
	if (p_children.size() >= 7) p_children.erase(p_children.begin() + 6, p_children.end());
}

void FullScrollingTextBox::repositionText()
{
	//This method gets called whenever the setAbsoluteSize() method for the FullScrollingTextBox
	//is called. When a manual resize happens all the different lines of text will need to be shifted
	//back into place. If it's the first time this method is being called then the text box will
	//actually be resized vertically so that an even number of lines of text will fit. In the case
	//that the dynamic width flag has been set, this method will execute on window resize
	auto currentTextBoxAbsoluteSize = p_children[0]->getAbsoluteSize();

	if (m_dynamicSize)
	{
		//The width of the text box is set to the width of the widest line of text
		//See which line of text has the widest width in pixels. Also, it's possible
		//that resizing the box has caused the current longest text to wrap down to a
		//second line, we need to make sure that all render heights are the same 
		currentTextBoxAbsoluteSize.x = 0;

		//first scan and make sure all text has the same height
		for (int i = 6; i < p_children.size(); i++)
		{
			int difference = absoluteCompare(p_children[i - 1]->getText()->renderDPI.y / m_screenSize->Height, p_children[i]->getText()->renderDPI.y / m_screenSize->Height);
			if (difference != 0)
			{
				auto a = p_children[i - 1]->getText();
				auto b = p_children[i]->getText();
				//overflow has occured. Increase the length by a factor of 1.5
				int widerWord = i;
				if (difference > 0) widerWord--;

				p_children[widerWord]->getText()->renderDPI.x *= 1.5;
				break; //only one line of text should cause overflow so break after finding it
			}
		}

		for (int i = 5; i < p_children.size(); i++)
		{
			float currentWidth = p_children[i]->getText()->renderDPI.x;
			if (currentWidth > currentTextBoxAbsoluteSize.x) currentTextBoxAbsoluteSize.x = currentWidth;
		}

		currentTextBoxAbsoluteSize.x /= m_screenSize->Width; //convert pixels back to absolute size
		currentTextBoxAbsoluteSize.x += 0.02f; //give a little breathing room so text isn't right up against the edge of the box
	}

	if (m_state & UIElementState::NeedTextPixels)
	{
		//We want to make sure that the height of the text overlays is a perfect
		//multiple of the height of the text box to make sure that things look
		//good. When the box is first created and we get the pixel height of the
		//text for the first time, the height of the box will be slightly altered
		//so that things match up well.
		m_displayedText = round(currentTextBoxAbsoluteSize.y * m_screenSize->Height / (p_children[m_topText]->getText()->renderDPI.y / p_children[m_topText]->getText()->renderLines)); //round to the nearest integer
		if (m_displayedText < 2) m_displayedText = 2;

		//We now have info on the text height from the renderer class so set the 
		//absoluteTextHeight variable, which holds the ratio of the text height of
		//a single line to the height of the text box. It doesn't matter if we resize the window
		//or change only the size of the FullScrollingTextBox element. The size
		//of the text will change proportionally from this point onwards.
		m_relativeTextHeight = p_children[m_topText]->getText()->renderDPI.y / (p_children[m_topText]->getText()->renderLines * m_screenSize->Height);
		currentTextBoxAbsoluteSize.y = (float)m_displayedText * m_relativeTextHeight;

		//Update the absolute size for the parent element based on the new size
		//of the text box child element. Add the difference in width of the text
		//box child to the current width of the parent, and use the new height of
		//the child text box.
		setAbsoluteSize({ getAbsoluteSize().x + currentTextBoxAbsoluteSize.x - p_children[0]->getAbsoluteSize().x, currentTextBoxAbsoluteSize.y });

		//The text height is in relation to the height of the text box, not the screen.
		//Now that the height of the text box is finalized, divide the relative text
		//height to the screen by the absolute ratio of the text box height to the 
		//screen.
		m_relativeTextHeight /= getAbsoluteSize().y;
		setTextLocationsAndDimensions();
	
		//Remove the NeedTextPixels flag from the state
		m_state &= ~UIElementState::NeedTextPixels;
	}	
}

void FullScrollingTextBox::setTextLocationsAndDimensions()
{
	//We now need to update the locations for each text element. Whatever element is currently at the top
	//should have it's location changed to match the top of the text box. All other text elements are then
	//raised or lowered accordingly. Base the text location and size on the new size and location of the
	//text box child element that was calculated in the setAbsoluteSize() method right above.
	DirectX::XMFLOAT2 textBoxAbsoluteLocation     = p_children[0]->getAbsoluteLocation();
	DirectX::XMFLOAT2 textBoxAbsoluteSize         = p_children[0]->getAbsoluteSize();
	DirectX::XMFLOAT2 textOverlayAbsoluteSize     = { textBoxAbsoluteSize.x, m_relativeTextHeight * p_children[m_topText]->getText()->renderLines * textBoxAbsoluteSize.y };
	DirectX::XMFLOAT2 textOverlayAbsoluteLocation = { textBoxAbsoluteLocation.x + 0.001f, textBoxAbsoluteLocation.y - (textBoxAbsoluteSize.y - textOverlayAbsoluteSize.y) / 2.0f};
	
	p_children[m_topText]->setAbsoluteSize(textOverlayAbsoluteSize);
	p_children[m_topText]->setAbsoluteLocation(textOverlayAbsoluteLocation);

	//When the element is first created we update the absolute font height for each text overlay
	//to achieve the same pixel font height that was used during the original dimension calculations.
	//In all other calls to this text resizing method after initial set up we don't need to alter the
	//font in any way.
	float correctAbsoluteFontHeight = m_initialFontPixelSize / p_children[m_topText]->getPixelSize().y;
	if (m_state & UIElementState::NeedTextPixels) p_children[m_topText]->setFontSize(correctAbsoluteFontHeight);

	int linesRendered = p_children[m_topText]->getText()->renderLines; //we can only render the number of lines dictated by m_displayedText, keep track with this variable

	//Save the vertical components of text overlay location and size as these are the
	//only components that will be changing
	float previousTextOverlayAbsoluteHeight, previousTextOverlayAbsoluteYLocation;

	//First start at the top line of text and scroll backwards. Non-resizing text boxes can have items that are multiple lines
	//long so each text item can theoretically be a different height.
	for (int i = m_topText - 1; i >= 5; i--)
	{
		//get the height and location of the text in front of this one
		previousTextOverlayAbsoluteYLocation = textOverlayAbsoluteLocation.y;
		previousTextOverlayAbsoluteHeight = textOverlayAbsoluteSize.y;

		//set the location and height of the current text overlay
		textOverlayAbsoluteSize.y = m_relativeTextHeight * p_children[i]->getText()->renderLines * textBoxAbsoluteSize.y;
		textOverlayAbsoluteLocation.y = previousTextOverlayAbsoluteYLocation - (previousTextOverlayAbsoluteHeight + textOverlayAbsoluteSize.y) / 2.0f;
		p_children[i]->setAbsoluteSize(textOverlayAbsoluteSize);
		p_children[i]->setAbsoluteLocation(textOverlayAbsoluteLocation);
		
		//set the appropriate font height during initial set up
		if (m_state & UIElementState::NeedTextPixels)
		{
			correctAbsoluteFontHeight = m_initialFontPixelSize / p_children[i]->getPixelSize().y;
			p_children[i]->setFontSize(correctAbsoluteFontHeight);
		}

		//Since all text in this loop comes before the top option, they should all be made invisible
		if (p_children[i]->getState() & UIElementState::Hovered) p_children[i]->removeState(UIElementState::Hovered); //this makes sure anything that was hovered at creation gets its colors reset
		p_children[i]->setState(UIElementState::Invisible);
	}

	//Reset the textOverlay variables to reflect the textOverlay at the top of
	//the text box
	textOverlayAbsoluteLocation.y = p_children[m_topText]->getAbsoluteLocation().y;
	textOverlayAbsoluteSize.y = p_children[m_topText]->getAbsoluteSize().y;

	//Now do the same thing for all text after the top text
	for (int i = m_topText + 1; i < p_children.size(); i++)
	{
		//get the height and location of the text in front of this one
		previousTextOverlayAbsoluteYLocation = textOverlayAbsoluteLocation.y;
		previousTextOverlayAbsoluteHeight = textOverlayAbsoluteSize.y;

		//set the location and height of the current text overlay
		textOverlayAbsoluteSize.y = m_relativeTextHeight * p_children[i]->getText()->renderLines * textBoxAbsoluteSize.y;
		textOverlayAbsoluteLocation.y = previousTextOverlayAbsoluteYLocation + (previousTextOverlayAbsoluteHeight + textOverlayAbsoluteSize.y) / 2.0f;
		p_children[i]->setAbsoluteSize(textOverlayAbsoluteSize);
		p_children[i]->setAbsoluteLocation(textOverlayAbsoluteLocation);

		//set the appropriate font height during initial setup
		if (m_state & UIElementState::NeedTextPixels)
		{
			correctAbsoluteFontHeight = m_initialFontPixelSize / p_children[i]->getPixelSize().y;
			p_children[i]->setFontSize(correctAbsoluteFontHeight);
		}

		//increment the number of lines being rendered accordingly
		linesRendered += p_children[i]->getText()->renderLines;

		//if the current child is outside of the rendering area, set its state to invisible
		if (linesRendered > m_displayedText)
		{
			if (p_children[i]->getState() & UIElementState::Hovered) p_children[i]->removeState(UIElementState::Hovered); //this makes sure anything that was hovered at creation gets its colors reset
			p_children[i]->setState(UIElementState::Invisible);
		}
	}

	//With all text resized and repositioned, calculate the new location
	//for the scroll bar
	calcualteScrollBarLocation();
}

void FullScrollingTextBox::onScrollDown()
{
	//When the mouse wheel is scrolled up, it has the effect of moving the text downwards. If
	//the text is already at the top of the scroll box though then scrolling up doesn't do anything.
	//Scrolling is also disabled if all the text already fits inside the text box.

	//If the final option is at the bottom of the text box then we can't make
	//the text go any lower
	auto a = m_topText;
	int b = p_children.size() - m_displayedText;
	if (m_topText < (int)(p_children.size() - m_displayedText))
	{
		if (p_children[m_topText]->getState() & UIElementState::Hovered) p_children[m_topText]->removeState(UIElementState::Hovered); //anything that's being hovered while disappearing needs its state reset
		p_children[m_topText++]->setState(UIElementState::Invisible); //the current top line will get scroll upwards so it becomes invisible
		p_children[m_topText + m_displayedText - 1]->removeState(UIElementState::Invisible); //the new bottom is no longer invisible

		//Now shift the absolute locations for each text element upwards
		float scrollDistance = m_relativeTextHeight * getAbsoluteSize().y;
		for (int i = 5; i < p_children.size(); i++)
		{
			auto currentLocation = p_children[i]->getAbsoluteLocation();
			p_children[i]->setAbsoluteLocation({ currentLocation.x, currentLocation.y - scrollDistance });
			p_children[i]->resize(); //call the resize method on each text overlay to actually shift it as necessary
		}

		//finally, move the scroll bar
		calcualteScrollBarLocation();
	}
}

void FullScrollingTextBox::onScrollUp()
{
	//When the mouse wheel is scrolled down, it has the effect of moving the text upwards. If
	//the bottom of the text is already inside the scroll box though then scrolling down doesn't do anything.
	//Scrolling is also disabled if all the text already fits inside the text box.

	//If the first option is at the top of the text box then we can't make
	//the text go any higher
	if (m_topText > 5)
	{
		if (p_children[m_topText + m_displayedText - 1]->getState() & UIElementState::Hovered) p_children[m_topText + m_displayedText - 1]->removeState(UIElementState::Hovered); //anything that's being hovered while disappearing needs its state reset
		p_children[m_topText + m_displayedText - 1]->setState(UIElementState::Invisible);  //the curent bottom is no longer invisible
		p_children[--m_topText]->removeState(UIElementState::Invisible); //the new top line is now visible

		//Now shift the absolute locations for each text element downwards
		float scrollDistance = m_relativeTextHeight * getAbsoluteSize().y;
		for (int i = 5; i < p_children.size(); i++)
		{
			auto currentLocation = p_children[i]->getAbsoluteLocation();
			p_children[i]->setAbsoluteLocation({ currentLocation.x, currentLocation.y + scrollDistance });
			p_children[i]->resize(); //call the resize method on each text overlay to actually shift it as necessary
		}
		
		//finally, move the scroll bar
		calcualteScrollBarLocation();
	}
}

void FullScrollingTextBox::calcualteScrollBarLocation()
{
	//We calculate the height of the scroll bar by using a simple ratio. Assuming that the center of the
	//text box correlates to the center of the progress scroll bar, and the bottom of the unclipped
	//text correlates to the bottom of the scroll bar background then location of the scroll progress bar
	//center will simply be:

	//First, calculate the height of all text overlay children in pixels. Not all of them
	//are guaranteed to be the same height so this must be done in a loop.
	float totalAbsoluteTextHeight = 0.0f;
	for (int i = 5; i < p_children.size(); i++) totalAbsoluteTextHeight += p_children[i]->getAbsoluteSize().y;

	//If the height of the text is less than the height of the text box then the scroll bar
	//will simply be the height between the buttons and in the middle of the element so there's
	//no reason to update anything.
	if (totalAbsoluteTextHeight <= getAbsoluteSize().y) return;

	float ratio = getAbsoluteSize().y * (getAbsoluteSize().y - 2 * m_buttonHeight) / totalAbsoluteTextHeight;
	float bottomTextToCenterBox = (p_children.back()->getAbsoluteLocation().y + p_children.back()->getAbsoluteSize().y / 2.0f) - getAbsoluteLocation().y;
	float centerScrollBarToButtonTop = ratio / getAbsoluteSize().y * bottomTextToCenterBox;
	
	p_children[4]->setAbsoluteLocation({ p_children[3]->getAbsoluteLocation().x, getAbsoluteLocation().y + getAbsoluteSize().y / 2.0f - m_buttonHeight - centerScrollBarToButtonTop });
	p_children[4]->setAbsoluteSize({ p_children[4]->getAbsoluteSize().x, ratio }, true); //force a resize here to actually move the scroll bar

	//TODO: Since resizes no longer happen every single time the screen size changes (a certain threshold
	//now needs to be reached), the text overlays and the scroll bar will be resized when scrolling occurs but
	//other elements won't. May want to consider just resizing the entire element here.
}

uint32_t FullScrollingTextBox::update(InputState* inputState)
{
	//At the end of the standard update, we check to see if either of the buttons are currently being pressed,
	//or the scroll bar is being dragged. These actions will cause the box to scroll. We also check to see if
	//any options inside the scroll box have been selected (if the scroll box supports choosing items)
	uint32_t currentState = UIElement::update(inputState);

	if (p_children[2]->getState() & UIElementState::Released) onScrollUp();
	else if (p_children[3]->getState() & UIElementState::Released) onScrollDown();
	else if (p_children[4]->getState() & UIElementState::Clicked)
	{
		if (inputState->mouseClickState == MouseClickState::MouseClicked)
		{
			//Set the scroll bar click height as well as the current mouse height
			m_scrollBarClickHeight = inputState->mousePosition.y;
			m_currentMouseHeight = inputState->mousePosition.y;
		}
		else if (inputState->mouseClickState == MouseClickState::MouseHeld)
		{
			if (inputState->mousePosition.y != m_currentMouseHeight)
			{
				//When dragging the scroll bar upwards, if the absolute change in mouse position
				//is greater than the absolute height of the bottom text of the scroll box, it initiates
				//an upward scroll. Likewise when the bar is dragged downwards. We can figure out if 
				//the scroll threshold has been met by comparing two absolute ratios (i.e absolute scroll distance /
				//absolute scroll bar height = absolute text overlay height = absolute text box height)
				int bottom_text_index = m_topText + m_displayedText - 1;
				if (bottom_text_index >= p_children.size()) bottom_text_index = p_children.size() - 1;

				auto r2a = p_children[4]->getAbsoluteSize().y; //absolute height of scroll bar
				auto r1b = p_children[bottom_text_index]->getAbsoluteSize().y; //absolute height of text overlay at bottom of text box
				auto r2b = getAbsoluteSize().y; //absolute height of text box

				if (inputState->mousePosition.y < m_scrollBarClickHeight)
				{
					if (((m_scrollBarClickHeight - inputState->mousePosition.y) / m_screenSize->Height) >= (r2a * r1b / r2b))
					{
						onScrollUp();
						m_scrollBarClickHeight = inputState->mousePosition.y;
					}
				}
				else
				{
					if (((inputState->mousePosition.y - m_scrollBarClickHeight) / m_screenSize->Height) >= (r2a * r1b / r2b))
					{
						onScrollDown();
						m_scrollBarClickHeight = inputState->mousePosition.y;
					}
				}
				
				m_currentMouseHeight = inputState->mousePosition.y; //reset the click variable to prevent infinite scrolling
			}
		}
	}
	else if ((inputState->mouseClickState == MouseClickState::MouseClicked) && isMouseHovered(inputState->mousePosition))
	{
		//if a mouse click occurs while over the text box (the m_size and m_location variables
		//are tied to the text box), set the clicked flag of the state and also call the select()
		//method on the option that was clicked. Remove the selected flag from any other previously 
		//selected option
		for (int i = m_topText; i < m_topText + m_displayedText; i++)
		{
			if (i >= p_children.size()) break; //it's possible that we can display more text than is currently in the box
			if (p_children[i]->getState() & UIElementState::Hovered)
			{
				//Set the m_clickedTextIndex variable which is used to confirm
				//that the mouse is released on the same option that gets clicked.
				m_clickedTextIndex = i;
				break;
			}
		}

		//add the clicked flag to the current state
		currentState |= UIElementState::Clicked;
	}
	else if ((inputState->mouseClickState == MouseClickState::MouseReleased) && isMouseHovered(inputState->mousePosition))
	{
		//If a mouse click is released while over the text box (the m_size and m_location variables
		//are tied to the text box) then we select the option being hovered over, however, only
		//if that option was actually clicked first.
		for (int i = m_topText; i < m_topText + m_displayedText; i++)
		{
			if (i >= p_children.size()) break; //it's possible that we can display more text than is currently in the box
			if (p_children[i]->getState() & UIElementState::Hovered)
			{
				if (i == m_clickedTextIndex)
				{
					//The mouse was released on the same option that was clicked so we change
					//the color of the text and update the last selected text for the scroll box
					for (int j = m_topText; j < m_topText + m_displayedText; j++)
					{
						if (j >= p_children.size()) break; //again, make sure we don't go outside of the vector
						if (p_children[j]->getState() & UIElementState::Selected)
						{
							p_children[j]->removeState(UIElementState::Selected);
						}
					}

					((HighlightableTextOverlay*)p_children[i].get())->select(); //select the current option
					m_lastSelectedText = p_children[i]->getText()->message;

					//add the released flag to the current state
					currentState |= UIElementState::Released;
				}
				
				break;
			}
		}

		//regardless of whether we release on the clicked option or not, remove the
		//clicked state from the text box
		currentState &= ~UIElementState::Clicked;
	}

	return currentState;
}

std::vector<UIText*> FullScrollingTextBox::setTextDimension()
{
	//Return a reference to all text elements inside the text box. Text
	//starts at the 5th element of the child array
	std::vector<UIText*> text;
	for (int i = 5; i < p_children.size(); i++) text.push_back(p_children[i]->getText());
	return text;
}

void FullScrollingTextBox::setChildrenAbsoluteSize(DirectX::XMFLOAT2 size)
{
	//Calculate the appropriate height and width of the buttons and
	//set these new sizes
	float screen_ratio = MAX_SCREEN_HEIGHT / MAX_SCREEN_WIDTH;
	m_buttonHeight = m_buttonRatio * size.y; //for now make both buttons 1/8th the height of the box
	float buttonWidth = screen_ratio * m_buttonHeight;
	
	p_children[2]->setAbsoluteSize({ buttonWidth, m_buttonHeight });
	p_children[3]->setAbsoluteSize({ buttonWidth, m_buttonHeight });

	//The width of the text box is set to the width of the whole element minus
	//the width of the buttons, the height is the same as the whole element.
	p_children[0]->setAbsoluteSize({ size.x - buttonWidth, size.y });

	//The scroll bar background is simply the width of the buttons and the
	//height of the whole element. 
	p_children[1]->setAbsoluteSize({ buttonWidth, size.y });

	//The physical scroll bar is a little more complicated to calculate as
	//it's a function of the current text being displayed in the box. For now
	//just set the scroll bar to its maximal height. If there's more text than
	//fits in the text box then its size and location will be updated automatically
	//later on.
	p_children[4]->setAbsoluteSize({ buttonWidth, size.y - 2 * m_buttonHeight });

	//With everything sized appropriately, shift all children to their 
	//correct locations.
	auto absoluteLocation = getAbsoluteLocation();
	auto buttonXLocation = absoluteLocation.x + (size.x - buttonWidth) / 2.0f;

	p_children[0]->setAbsoluteLocation({ absoluteLocation.x - buttonWidth / 2.0f, absoluteLocation.y });
	p_children[1]->setAbsoluteLocation({ buttonXLocation, absoluteLocation.y });
	p_children[2]->setAbsoluteLocation({ buttonXLocation, absoluteLocation.y - (size.y - m_buttonHeight) / 2.0f });
	p_children[3]->setAbsoluteLocation({ buttonXLocation, absoluteLocation.y + (size.y - m_buttonHeight) / 2.0f });
	p_children[4]->setAbsoluteLocation({ buttonXLocation, absoluteLocation.y }); //get's fully updated later on if necessary

	//Next, all of the text overlays inside the text box need to be resized, but only
	//if the text has already been appropriately resized by the renderer class.
	if (!(m_state & UIElementState::NeedTextPixels)) setTextLocationsAndDimensions();
}