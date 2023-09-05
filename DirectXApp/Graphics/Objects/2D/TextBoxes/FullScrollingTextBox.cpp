#include "pch.h"
#include "FullScrollingTextBox.h"
#include "HighlightableTextOverlayBasic.h"
#include "Graphics/Objects/2D/Buttons/ArrowButton.h"

FullScrollingTextBox::FullScrollingTextBox(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring message, float fontSize,
	bool highlightableText, bool dynamicSize, UITextJustification justification, UIColor textFillColor, bool isSquare, UIColor outlineColor, UIColor shadowColor)
{
	//First set the highlightable text and dynamic size variables
	m_highlightableText = highlightableText;
	m_dynamicSize = dynamicSize;

	//If the dynamic size variable is chosen then we won't actually know the width
	//of the text box until after we get the text pixel dimensions from the renderer.
	//Either way though, the textBox is the first child element.
	ShadowedBox textBox(windowSize, location, size, false);

	//A scrolling text box also features two arrow buttons, one up and one down.
	//Both of these buttons are squares
	m_buttonSize = 0.1 * size.y; //for now make both buttons 1/10th the height of the box
	ArrowButton upButton(windowSize, { location.x + (size.x + m_buttonSize) / (float)2.0, location.y - (size.y - m_buttonSize) / (float)2.0 }, { m_buttonSize, m_buttonSize }, false, true);
	ArrowButton downButton(windowSize, { location.x + (size.x + m_buttonSize) / (float)2.0, location.y + (size.y - m_buttonSize) / (float)2.0 }, { m_buttonSize, m_buttonSize }, true, true);

	//Finally, there's a progress bar between the buttons that shows how much of the text has
	//been scrolled through. This progress bar is composed of a background shadow box and a
	//foreground shadow box. The Foreground shadow box will need to potentially change in size
	//as the window get's bigger and smaller.
	float shadowPixels = (((ShadowedBox*)upButton.getChildren()[0].get())->getShadowWidth() + 1.0f) / windowSize.Width; //get the relative width of the button shadows
	OutlinedBox progressBarBackground(windowSize, { location.x + (size.x + m_buttonSize) / (float)2.0 + shadowPixels / 2.0f, location.y + shadowPixels / 2.0f }, { m_buttonSize + shadowPixels, size.y - 2.0f * m_buttonSize - shadowPixels }, true, UIColor::Gray);
	ShadowedBox progressBarForeground(windowSize, { location.x + (size.x + m_buttonSize) / (float)2.0 - shadowPixels, location.y }, { m_buttonSize - 2.0f / windowSize.Width, (size.y - 2.0f * m_buttonSize) / 2.0f }, true, UIColor::PaleGray); //y size and location will change

	//The order of the child elements is important here. The text background must be first, then the text,
	//and then finally the hiding box to go on top of it.
	p_children.push_back(std::make_shared<ShadowedBox>(textBox));
	p_children.push_back(std::make_shared<ArrowButton>(upButton));
	p_children.push_back(std::make_shared<ArrowButton>(downButton));
	p_children.push_back(std::make_shared<OutlinedBox>(progressBarBackground));
	p_children.push_back(std::make_shared<ShadowedBox>(progressBarForeground));

	m_isScrollable = true; //this enables scrolling detection in the main rendering loop
	m_needTextRenderDimensions = true; //alerts the current mode that this element will need text pixels from the renderer at some point

	//Unlike the partial scrolling text box, the full scrolling text box features multiple
	//different text elements. this constructor expects a single input string that's delimited with '\n' characters.
	//Split the string by the newline charcter and create a new textOverlay object from each one
	int i = 0, j = 0;
	while (j != std::string::npos)
	{
		j = message.find(L'\n', i);

		std::wstring textLine = message.substr(i, j - i);

		//Text defaults to a single black color that's left justified. When first creating the text, set the location to the center
		//of the screen and the size to be the same as the screen size. This will get overriden but it helps to determine the
		//correct text height.
		if (m_highlightableText)
		{
			HighlightableTextOverlayBasic newText(windowSize, { 0.5, 0.5 }, { 1, 1 }, textLine, fontSize, { UIColor::Black }, { 0, (unsigned int)textLine.length() }, UITextJustification::CenterLeft);
			p_children.push_back(std::make_shared<HighlightableTextOverlayBasic>(newText));
		}
		else 
		{
			TextOverlayBasic newText(windowSize, { 0.5, 0.5 }, { 1, 1 }, textLine, fontSize, { UIColor::Black }, { 0, (unsigned int)textLine.length() }, UITextJustification::CenterLeft);
			p_children.push_back(std::make_shared<TextOverlayBasic>(newText));
		}

		i = j + 1;
	}

	m_isScrollable = true; //this enables scrolling detection in the main rendering loop
	m_needTextRenderDimensions = true; //alerts the current mode that this element will need text pixels from the renderer at some point
	m_state = UIElementStateBasic::NeedTextPixels; //Let's the render know that we currently need the pixel size of text
	m_topText = 5; //Set the first line of text as the current top

	//set screen size dependent variables
	m_location = location;
	m_size = size; //if m_dynamicSize is set to true then this will be overridden
	m_fontSize = fontSize;
}

void FullScrollingTextBox::repositionText()
{
	//This method gets called whenever the state of the FullScrollingTextBox is set to NeedTextPixels. This
	//can happen when the object is first created, the window is resized, etc.

	auto currentWindowSize = getCurrentWindowSize();
	auto currentTextBoxAbsoluteSize = p_children[0]->getAbsoluteSize();

	if (m_dynamicSize)
	{
		//The width of the text box is set to the width of the widest line of text
		//See which line of text has the widest width in pixels.
		currentTextBoxAbsoluteSize.x = 0;
		for (int i = 5; i < p_children.size(); i++)
		{
			float currentWidth = p_children[i]->getText()->renderDPI.x;
			if (currentWidth > currentTextBoxAbsoluteSize.x) currentTextBoxAbsoluteSize.x = currentWidth;
		}

		currentTextBoxAbsoluteSize.x /= currentWindowSize.Width; //convert pixels back to absolute size
		currentTextBoxAbsoluteSize.x += 0.005; //give a little breathing room so text isn't reight up against the edge of the box

		//After calculating the widest bit of text, change the absolute size of each text object to match,
		//when calculating text dimensions these are set to match the window so need to be reduced
		for (int i = 5; i < p_children.size(); i++) p_children[i]->setAbsoluteSize({ currentTextBoxAbsoluteSize.x, p_children[i]->getText()->renderDPI.y / currentWindowSize.Height });
	}

	//In order to make everything look clean, set the absolute height of the scroll box to 
	//be a multiple of the text height of the options. Round down when doing this. We also
	//change the absolute font size
	m_fontSize /= currentTextBoxAbsoluteSize.y; //get the font size as a percentage of the text box height
	m_displayedText = currentTextBoxAbsoluteSize.y * currentWindowSize.Height / p_children[5]->getText()->renderDPI.y; //the division gets floored automatically here by converting to an int
	currentTextBoxAbsoluteSize.y = (float)m_displayedText * p_children[5]->getText()->renderDPI.y / currentWindowSize.Height; //no floor division happens because we're using floats
	m_fontSize *= currentTextBoxAbsoluteSize.y; //change font back into a percentage of the over window height

	//Update the absolute size for the TextBox, this should also update the size of
	//all children elements accordingly.
	p_children[0]->setAbsoluteSize(currentTextBoxAbsoluteSize);
	m_size = currentTextBoxAbsoluteSize; //update the size of the FullScrollingTextBox to reflect that of the text box child element.
	
	//We now need to update the locations for each text element. Whatever element is currently at the top
	//should have it's location changed to match the top of the text box. All other text elements are then
	//raised or lowered accordingly.
	DirectX::XMFLOAT2 topTextAbsoluteLocation = {m_location.x, m_location.y - m_size.y / 2.0f + (p_children[5]->getText()->renderDPI.y / currentWindowSize.Height) / 2.0f};
	for (int i = 5; i < p_children.size(); i++)
	{
		p_children[i]->setAbsoluteLocation({ topTextAbsoluteLocation.x + 0.001f, topTextAbsoluteLocation.y + (i - m_topText) * p_children[5]->getText()->renderDPI.y / currentWindowSize.Height });

		//if the current child is outside of the rendering area, set its state to invisible
		if (i < m_topText || i >= (m_topText + m_displayedText)) p_children[i]->setState(UIElementStateBasic::Invisible);
	}

	//Since the height of the text box can change depending on the size of the text we need to
	//also move the scroll bars and buttons for the scroll box.
	
	auto progressBarBackgroundSize = p_children[3]->getAbsoluteSize();
	auto upButtonLocation = p_children[1]->getAbsoluteLocation();
	auto downButtonLocation = p_children[2]->getAbsoluteLocation();

	//Need to account for the "drift" of square elements in the scroll bar and button
	float driftCorrectedButtonLocation = m_location.x + (m_size.x + progressBarBackgroundSize.x) / 2.0f - ((ShadowedBox*)p_children[2]->getChildren()[0].get())->fixSquareBoxDrift(currentWindowSize);

	p_children[1]->setAbsoluteLocation({ driftCorrectedButtonLocation, m_location.y - (m_size.y - m_buttonSize) / 2.0f });
	p_children[2]->setAbsoluteLocation({ driftCorrectedButtonLocation, m_location.y + (m_size.y - m_buttonSize) / 2.0f });
	p_children[3]->setAbsoluteSize({ progressBarBackgroundSize.x, m_size.y - 2.0f * m_buttonSize});
	p_children[3]->setAbsoluteLocation({ driftCorrectedButtonLocation, m_location.y });

	float shadowPixels = (((ShadowedBox*)p_children[2]->getChildren()[0].get())->getShadowWidth() + 1.0) / currentWindowSize.Width; //get the relative width of the shadow box shadow for the buttons

	//Now we need to recalculate the size and location of the scroll progress bar
	float textProgress = (float)m_displayedText / (float)(p_children.size() - 5);
	p_children[4]->setAbsoluteSize({ progressBarBackgroundSize.x, textProgress * (m_size.y - 2.0f * m_buttonSize)});
	p_children[4]->setAbsoluteLocation({ driftCorrectedButtonLocation, m_location.y }); //the y-component will get updated in the calculateScrollBarLocation() method
	calcualteScrollBarLocation(currentWindowSize);
}

void FullScrollingTextBox::onScrollUp()
{
	//When the mouse wheel is scrolled up, it has the effect of moving the text downwards. If
	//the text is already at the top of the scroll box though then scrolling up doesn't do anything.
	//Scrolling is also disabled if all the text already fits inside the text box.
	auto currentWindowSize = getCurrentWindowSize();

	//If the final option is at the bottom of the text box then we can't make
	//the text go any lower
	if (m_topText < p_children.size() - m_displayedText)
	{
		p_children[m_topText++]->setState(UIElementStateBasic::Invisible); //the current top line will get scroll upwards so it becomes invisible
		p_children[m_topText + m_displayedText - 1]->removeState(UIElementStateBasic::Invisible); //the new bottom is no longer invisible

		//Now shift the absolute locations for each text element upwards
		float absoluteTextHeight = p_children[5]->getText()->renderDPI.y / currentWindowSize.Height;
		for (int i = 5; i < p_children.size(); i++)
		{
			auto currentLocation = p_children[i]->getAbsoluteLocation();
			p_children[i]->setAbsoluteLocation({currentLocation.x, currentLocation.y - absoluteTextHeight});
			p_children[i]->resize(currentWindowSize);
		}

		//finally, move the scroll bar
		calcualteScrollBarLocation(currentWindowSize);
	}
}

void FullScrollingTextBox::onScrollDown()
{
	//When the mouse wheel is scrolled down, it has the effect of moving the text upwards. If
	//the bottom of the text is already inside the scroll box though then scrolling down doesn't do anything.
	//Scrolling is also disabled if all the text already fits inside the text box.
	auto currentWindowSize = getCurrentWindowSize();

	//If the first option is at the top of the text box then we can't make
	//the text go any higher
	if (m_topText > 5)
	{
		p_children[m_topText + m_displayedText - 1]->setState(UIElementStateBasic::Invisible);  //the curent bottom is no longer invisible
		p_children[--m_topText]->removeState(UIElementStateBasic::Invisible); //the new top line is now visible

		//Now shift the absolute locations for each text element downwards
		float absoluteTextHeight = p_children[5]->getText()->renderDPI.y / currentWindowSize.Height;
		for (int i = 5; i < p_children.size(); i++)
		{
			auto currentLocation = p_children[i]->getAbsoluteLocation();
			p_children[i]->setAbsoluteLocation({ currentLocation.x, currentLocation.y + absoluteTextHeight });
			p_children[i]->resize(currentWindowSize);
		}
		
		//finally, move the scroll bar
		calcualteScrollBarLocation(currentWindowSize);
	}
}

void FullScrollingTextBox::calcualteScrollBarLocation(winrt::Windows::Foundation::Size windowSize)
{
	//We calculate the height of the scroll bar by using a simple ratio. Assuming that the center of the
	//text box correlates to the center of the progress scroll bar, and the bottom of the unclipped
	//text correlates to the bottom of the scroll bar background then location of the scroll progress bar
	//center will simply be:
	float totalTextHeight = p_children[5]->getText()->renderDPI.y * (p_children.size() - 5); //the first 5 elements of the child array aren't text
	float bottomTextToCenterBox = p_children[5]->getText()->renderDPI.y * (p_children.size() - (m_topText + m_displayedText)) + windowSize.Height * (m_size.y / 2.0f);
	auto scrollProgressBackgroundBarAbsoluteSize = p_children[3]->getAbsoluteSize();
	auto scrollProgressBarAbsoluteLocation = p_children[4]->getAbsoluteLocation();
	float shadowHeight = ((ShadowedBox*)p_children[2]->getChildren()[0].get())->getShadowWidth();

	float scrollBarAbsoluteHeightFromBottom = (scrollProgressBackgroundBarAbsoluteSize.y * windowSize.Height - shadowHeight) * bottomTextToCenterBox / totalTextHeight;
	scrollBarAbsoluteHeightFromBottom /= windowSize.Height;
	scrollBarAbsoluteHeightFromBottom -= -shadowHeight / windowSize.Height;

	p_children[4]->setAbsoluteLocation({ scrollProgressBarAbsoluteLocation.x, (m_location.y + m_size.y / 2.0f) - m_buttonSize - scrollBarAbsoluteHeightFromBottom });
	p_children[4]->resize(windowSize);
}

uint32_t FullScrollingTextBox::update(InputState* inputState)
{
	//At the end of the standard update, we check to see if either of the buttons are currently being pressed.
	//If so, it has the effect of scrolling the text twice.
	uint32_t currentState = UIElementBasic::update(inputState);

	if ((p_children[2]->getState() & UIElementStateBasic::Clicked) && inputState->mouseClick)
	{
		onScrollUp();
		onScrollUp();
	}
	else if ((p_children[1]->getState() & UIElementStateBasic::Clicked) && inputState->mouseClick)
	{
		onScrollDown();
		onScrollDown();
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

float FullScrollingTextBox::getCurrentTextStartingHeight()
{
	//gives us the y coordinate for the starting point of the text in pixels
	return p_children[0]->getChildren()[1]->getText()->startLocation.y;
}