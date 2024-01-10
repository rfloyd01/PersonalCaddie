#include "pch.h"
#include "FullScrollingTextBox.h"
#include "HighlightableTextOverlay.h"
#include "Graphics/Objects/2D/Buttons/ArrowButton.h"
#include <cmath>

/*
Order of Child Elements:
0: Main Text Box
1: Up Scrolling Button
2: Down Scrolling Button
3: Scroll Progress Bar Background
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
	m_heightSet = false;

	//Then set the screen size dependent variables. The size variable
	//encompasses the text box, as well as the buttons and scroll bar
	//which means that the center of the element will be offset a little
	//bit from the actual center of the text box.
	updateLocationAndSize(location, size);
	m_fontSize = fontSize;

	//A scrolling text box features two arrow buttons, one up and one down.
	//Both of these buttons are squares. Create these elements first to 
	//help eith placement and sizing of the other elements.
	float screen_ratio = MAX_SCREEN_HEIGHT / MAX_SCREEN_WIDTH;
	m_buttonHeight = m_buttonRatio * size.y; //for now make both buttons 1/8th the height of the box
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
	ShadowedBox progressBarForeground(windowSize, { location.x + (size.x - buttonWidth) / 2.0f, location.y }, { buttonWidth, (size.y - 2.0f * m_buttonHeight) / 2.0f }, UIColor::PaleGray); //y size and location will change after text is added

	p_children.push_back(std::make_shared<OutlinedBox>(textBox));
	p_children.push_back(std::make_shared<OutlinedBox>(progressBarBackground));
	p_children.push_back(std::make_shared<ArrowButton>(upButton));
	p_children.push_back(std::make_shared<ArrowButton>(downButton));
	p_children.push_back(std::make_shared<ShadowedBox>(progressBarForeground));

	//Set values for class variables
	m_isScrollable = true; //this enables scrolling detection in the main rendering loop
	m_needTextRenderDimensions = true; //alerts the current mode that this element will need text pixels from the renderer at some point

	m_isScrollable = true; //this enables scrolling detection in the main rendering loop
	m_needTextRenderDimensions = true; //alerts the current mode that this element will need text pixels from the renderer at some point
	m_topText = 5; //Set the first line of text as the current top
	m_lastSelectedText = L"";

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
		//Make the text overlay take up the whole screen to ensure we get the exact
		//height for the line of text.
		textLocation = { 0.5, 0.5 };
		textSize = { 1.0, 1.0 };
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
			HighlightableTextOverlay newText(m_screenSize, { textLocation.x, textLocation.y + currentLine * existingTextHeight}, textSize, textLine, m_fontSize,
				{ UIColor::Black }, { 0, (unsigned int)textLine.length() }, UITextJustification::CenterLeft, false);
			p_children.push_back(std::make_shared<HighlightableTextOverlay>(newText));
		}
		else
		{
			TextOverlay newText(m_screenSize, { textLocation.x, textLocation.y + currentLine * existingTextHeight }, textSize, textLine, m_fontSize,
				{ UIColor::Black }, { 0, (unsigned int)textLine.length() }, UITextJustification::CenterLeft, false);
			p_children.push_back(std::make_shared<TextOverlay>(newText));
		}

		i = j + 1;
		currentLine++;
	}

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
			
			int difference = pixelCompare(p_children[i - 1]->getText()->renderDPI.y, p_children[i]->getText()->renderDPI.y);
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
		currentTextBoxAbsoluteSize.x += 0.005; //give a little breathing room so text isn't right up against the edge of the box
	}

	if (!m_heightSet)
	{
		//In case the size of the whole element changes, record the original height of the
	    //text box as this was used to calculate the font size.
		float textOriginalHeight = p_children[m_topText]->getAbsoluteSize().y; //TODO: Remove this

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
		setAbsoluteSize({ getAbsoluteSize().x + p_children[0]->getAbsoluteSize().x - currentTextBoxAbsoluteSize.x, currentTextBoxAbsoluteSize.y });

		//The text height is in relation to the height of the text box, not the screen.
		//Now that the height of the text box is finalized, divide the relative text
		//height to the screen by the absolute ratio of the text box height to the 
		//screen.
		m_relativeTextHeight /= getAbsoluteSize().y;

		setTextLocationsAndDimensions(textOriginalHeight);
	
		m_heightSet = true;
	}	
}

void FullScrollingTextBox::setTextLocationsAndDimensions(float textSize)
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

	//The font size of the text overlay needs to be updated to reflect that it's a ratio of the height of the 
	//entire text box, and not the individual text overlay.
	p_children[m_topText]->setFontSize(m_fontSize * textBoxAbsoluteSize.y / textOverlayAbsoluteSize.y);
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

		//set the appropriate font height
		p_children[i]->setFontSize(m_fontSize * textBoxAbsoluteSize.y / textOverlayAbsoluteSize.y);

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

		//set the appropriate font height
		p_children[i]->setFontSize(m_fontSize * textBoxAbsoluteSize.y / textOverlayAbsoluteSize.y);

		//increment the number of lines being rendered accordingly
		linesRendered += p_children[i]->getText()->renderLines;

		//if the current child is outside of the rendering area, set its state to invisible
		if (linesRendered > m_displayedText)
		{
			if (p_children[i]->getState() & UIElementState::Hovered) p_children[i]->removeState(UIElementState::Hovered); //this makes sure anything that was hovered at creation gets its colors reset
			p_children[i]->setState(UIElementState::Invisible);
		}
	}

	//DEBUG: Once All text has been resized and set, create red boxes around each text overlay
	//int end = p_children.size();
	//for (int i = 5; i < end; i++)
	//{
	//	//DEBUG:
	//	Box box(m_screenSize, p_children[i]->getAbsoluteLocation(), p_children[i]->getAbsoluteSize(), UIColor::Red, UIShapeFillType::NoFill);
	//	p_children.push_back(std::make_shared<Box>(box));
	//}


	//With all text resized and repositioned, calculate the new location
	//for the scroll bar
	calcualteScrollBarLocation();
}

void FullScrollingTextBox::onScrollUp()
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
		//float absoluteTextHeight = p_children[5]->getText()->renderDPI.y / m_screenSize->Height;
		float scrollDistance = m_relativeTextHeight * getAbsoluteSize().y;
		for (int i = 5; i < p_children.size(); i++)
		{
			auto currentLocation = p_children[i]->getAbsoluteLocation();
			//p_children[i]->setAbsoluteLocation({currentLocation.x, currentLocation.y - absoluteTextHeight});
			p_children[i]->setAbsoluteLocation({ currentLocation.x, currentLocation.y - scrollDistance });
			p_children[i]->resize();
		}

		//finally, move the scroll bar
		calcualteScrollBarLocation();
	}
}

void FullScrollingTextBox::onScrollDown()
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
		//float absoluteTextHeight = p_children[5]->getText()->renderDPI.y / m_screenSize->Height;
		float scrollDistance = m_relativeTextHeight * getAbsoluteSize().y;
		for (int i = 5; i < p_children.size(); i++)
		{
			auto currentLocation = p_children[i]->getAbsoluteLocation();
			//p_children[i]->setAbsoluteLocation({ currentLocation.x, currentLocation.y + absoluteTextHeight });
			p_children[i]->setAbsoluteLocation({ currentLocation.x, currentLocation.y + scrollDistance });
			p_children[i]->resize();
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
	for (int i = 5; i < p_children.size(); i++)
	{
		totalAbsoluteTextHeight += p_children[i]->getAbsoluteSize().y;
	}

	float ratio = getAbsoluteSize().y * (getAbsoluteSize().y - 2 * m_buttonHeight) / totalAbsoluteTextHeight;
	p_children[4]->setAbsoluteSize({ p_children[4]->getAbsoluteSize().x, ratio });

	float bottomTextToCenterBox = (p_children.back()->getAbsoluteLocation().y + p_children.back()->getAbsoluteSize().y / 2.0f) - getAbsoluteLocation().y;
	/*auto scrollProgressBackgroundBarAbsoluteSize = p_children[3]->getAbsoluteSize();
	auto scrollProgressBarAbsoluteLocation = p_children[4]->getAbsoluteLocation();
	float shadowHeight = ((ShadowedBox*)p_children[2]->getChildren()[0].get())->getShadowWidth();

	float scrollBarAbsoluteHeightFromBottom = (scrollProgressBackgroundBarAbsoluteSize.y * m_screenSize->Height - shadowHeight) * bottomTextToCenterBox / totalTextHeight;
	scrollBarAbsoluteHeightFromBottom /= m_screenSize->Height;
	scrollBarAbsoluteHeightFromBottom -= -shadowHeight / m_screenSize->Height;*/
	float centerScrollBarToButtonTop = ratio / getAbsoluteSize().y * bottomTextToCenterBox;
	p_children[4]->setAbsoluteLocation({ p_children[4]->getAbsoluteLocation().x, getAbsoluteLocation().y + getAbsoluteSize().y / 2.0f - m_buttonHeight - centerScrollBarToButtonTop });
	
	//p_children[4]->setAbsoluteLocation({ scrollProgressBarAbsoluteLocation.x, (m_location.y + m_size.y / 2.0f) - m_buttonHeight - scrollBarAbsoluteHeightFromBottom });
	p_children[4]->resize();
}

uint32_t FullScrollingTextBox::update(InputState* inputState)
{
	//At the end of the standard update, we check to see if either of the buttons are currently being pressed.
	//If so, it has the effect of scrolling the text twice.
	uint32_t currentState = UIElement::update(inputState);

	if (p_children[2]->getState() & UIElementState::Released)
	{
		onScrollUp();
		onScrollUp();
	}
	else if (p_children[1]->getState() & UIElementState::Released)
	{
		onScrollDown();
		onScrollDown();
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

void FullScrollingTextBox::setAbsoluteSize(DirectX::XMFLOAT2 size)
{
	//The Full Scrolling Text box has 4 direct child elements that
	//need to be positioned about the center of the element when a
	//manual resize occurs.
	UIElement::setAbsoluteSize(size); //first resize the element as a whole

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
	//it's a function of the current text being displayed in the box. Calcualte
	//the appropriate height of the element, and set the width to be the same
	//as the button width.
	p_children[4]->setAbsoluteSize({ buttonWidth, size.y / 4.0f }); //TODO: Update this when ready

	//With everything sized appropriately, shift all children to their 
	//correct locations.
	auto absoluteLocation = getAbsoluteLocation();
	p_children[0]->setAbsoluteLocation({ absoluteLocation.x - buttonWidth / 2.0f, absoluteLocation.y });
	p_children[1]->setAbsoluteLocation({ absoluteLocation.x + (size.x - buttonWidth) / 2.0f, absoluteLocation.y });
	p_children[2]->setAbsoluteLocation({ absoluteLocation.x + (size.x - buttonWidth) / 2.0f, absoluteLocation.y - (size.y - m_buttonHeight) / 2.0f });
	p_children[3]->setAbsoluteLocation({ absoluteLocation.x + (size.x - buttonWidth) / 2.0f, absoluteLocation.y + (size.y - m_buttonHeight) / 2.0f });
	p_children[4]->setAbsoluteLocation({ absoluteLocation.x + (size.x - buttonWidth) / 2.0f, absoluteLocation.y }); //TODO: Update this when ready

	//Next all of the text overlays inside the text box need to be resized
	if (m_heightSet)
	{
		setTextLocationsAndDimensions(p_children[5]->getAbsoluteSize().y);
	}

	//A resize means we need to get text sizes from the renderer again.
	m_state |= UIElementState::NeedTextPixels;
}

float FullScrollingTextBox::getCurrentTextStartingHeight()
{
	//gives us the y coordinate for the starting point of the text in pixels
	return p_children[0]->getChildren()[1]->getText()->startLocation.y;
}