#include "pch.h"
#include "PartialScrollingTextBox.h"
#include "Graphics/Objects/2D/Buttons/ArrowButton.h"

//TODO:
/*
There are a few bugs in this class that will need further addressing.
- The render height of the text in pixels is only calculated once currently.
  Changing the size of the element, either by changing the screen size, or manually
  and we can easily calculate the correct text height in pixels, however, the total
  height of the rendered text doesn't change. This makes it so that shrinking the screen
  size will let us scroll well beyond the bottom of the text. Not a huge deal but something
  to be aware of. May want to consider updating the full render height of the text on each
  resize like I originally used to.
- Can't scroll the box by dragging the scroll bar, like can be done in the Full Scrolling
  text box class. Should implement this feature after the above bug is addressed.
*/

PartialScrollingTextBox::PartialScrollingTextBox(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring message, float fontSize,
	UIColor coverBoxColor, std::vector<UIColor> textColor, std::vector<unsigned long long> textColorLocations, UITextJustification justification, UIColor textFillColor, bool isSquare, UIColor outlineColor, UIColor shadowColor)
{
	m_screenSize = windowSize;
	updateLocationAndSize(location, size);

	//A scrolling text box features two arrow buttons, one up and one down.
	//These are created first as other variables sizes depend on them.
	//Both of these buttons are squares
	m_buttonRatio = 0.125f; //The buttons will be 1/8th the height of the box
	m_buttonHeight = m_buttonRatio * size.y; //for now make both buttons 1/8th the height of the box
	float screen_ratio = MAX_SCREEN_HEIGHT / MAX_SCREEN_WIDTH;
	m_buttonWidth = screen_ratio * m_buttonHeight;
	ArrowButton upButton(windowSize, { location.x + (size.x - m_buttonWidth) / 2.0f, location.y - (size.y - m_buttonHeight) / 2.0f }, { m_buttonWidth, m_buttonHeight }, false);
	ArrowButton downButton(windowSize, { location.x + (size.x - m_buttonWidth) / 2.0f, location.y + (size.y - m_buttonHeight) / 2.0f }, { m_buttonWidth, m_buttonHeight }, true);

	//Before creating the text box, check to see if the text color array was default initialized. If it was then 
	//we add the locations for the start of the text (index 0) and the end of the text (message.length())
	if (textColorLocations.size() == 0)
	{
		textColorLocations.push_back(0);
		textColorLocations.push_back(message.length());
	}

	//textBox(windowSize, { location.x - buttonWidth / 2.0f, location.y }, { size.x - buttonWidth, size.y });
	TextBox textBox(windowSize, { location.x - m_buttonWidth / 2.0f, location.y }, { size.x - m_buttonWidth, size.y }, message, fontSize, textColor, textColorLocations, justification, textFillColor, outlineColor, shadowColor);

	//The text covering box is the same width as the text box, and extends from the top of the text box
	//all the way to the top of the window. The cover box should start 1 pixel above the text box so it
	//doesn't cover the upper outline.
	Box textCoveringBox(windowSize, { location.x, location.y / 2.0f - size.y / 4.0f - 1.0f / MAX_SCREEN_HEIGHT }, { size.x, location.y - size.y / 2.0f }, coverBoxColor, UIShapeFillType::Fill);
	
	//Finally, there's a progress bar between the buttons that shows how much of the text has
	//been scrolled through. This progress bar is composed of two elements: an outlined box that
	//acts as a background, and a shadowed box which shows the actual progress.
	OutlinedBox progressBarBackground(windowSize, { location.x + (size.x - m_buttonWidth) / 2.0f, location.y }, { m_buttonWidth, size.y }, UIColor::Gray);
	Button progressBarForeground(windowSize, { location.x + (size.x - m_buttonWidth) / 2.0f, location.y }, { m_buttonWidth, (size.y - 2.0f * m_buttonHeight) / 2.0f }, UIColor::PaleGray); //y size and location will change after text is added
	progressBarForeground.updateState(UIElementState::Disabled); //this is a button that can't be pressed

	//The order of the child elements is important here. The text background must be first, then the text,
	//and then finally the hiding box to go on top of it.
	p_children.push_back(std::make_shared<TextBox>(textBox));
	p_children.push_back(std::make_shared<Box>(textCoveringBox));
	p_children.push_back(std::make_shared<OutlinedBox>(progressBarBackground));
	p_children.push_back(std::make_shared<ArrowButton>(upButton));
	p_children.push_back(std::make_shared<ArrowButton>(downButton));
	p_children.push_back(std::make_shared<Button>(progressBarForeground));

	m_isScrollable = true; //this enables scrolling detection in the main rendering loop
	m_needTextRenderDimensions = true; //alerts the current mode that this element will need text pixels from the renderer at some point
	m_scrollIntensity = 0.01f; //set the scroll intensity

	m_state = UIElementState::NeedTextPixels; //Let's the render know that we currently need the pixel size of text
}

void PartialScrollingTextBox::onScrollUp()
{
	//When the mouse wheel is scrolled up, it has the effect of moving the text downwards. If
	//the text is already at the top of the scroll box though then scrolling up doesn't do anything.
	//Scrolling is also disabled if all the text already fits inside the text box.
	auto textOverlay = p_children[0]->getChildren()[1];
	auto currentTextAbsoluteLocation = textOverlay->getAbsoluteLocation();
	auto currentTextAbsoluteSize = textOverlay->getAbsoluteSize();

	if (absoluteCompare(currentTextAbsoluteLocation.y - currentTextAbsoluteSize.y / 2.0f, getAbsoluteLocation().y - getAbsoluteSize().y / 2.0f) < 0)
	{
		//Move the text downwards by the appropriate scroll intensity. Make sure that the text doesn't go lower than the
		//top of the text box.
		textOverlay->setAbsoluteLocation({ currentTextAbsoluteLocation.x, currentTextAbsoluteLocation.y + m_scrollIntensity / 2.0f });
		textOverlay->setAbsoluteSize({ currentTextAbsoluteSize.x, currentTextAbsoluteSize.y - m_scrollIntensity });

		//The font is normally sized according to the height of the text overlay, but in this case we want
		//the text to be sized based on the height of the entire text box. Since the absolute size of the
		//text overlay just changed, the font size will need to change accordingly.
		textOverlay->setFontSize(textOverlay->getFontSize() * currentTextAbsoluteSize.y / textOverlay->getAbsoluteSize().y);

		textOverlay->resize(); //use the new absolute coordinates to update the pixels for the text

		//After moving the text box, we move the scroll progress bar
		calcualteScrollBarLocation();
	}
}

void PartialScrollingTextBox::onScrollDown()
{
	//When the mouse wheel is scrolled down, it has the effect of moving the text upwards. If
	//the bottom of the text is already inside the scroll box though then scrolling down doesn't do anything.
	//Scrolling is also disabled if all the text already fits inside the text box.
	auto textOverlay = p_children[0]->getChildren()[1];
	auto currentTextAbsoluteLocation = textOverlay->getAbsoluteLocation();
	auto currentTextAbsoluteSize = textOverlay->getAbsoluteSize();
	float absoluteTextBoxBottom = (textOverlay->getText()->startLocation.y + textOverlay->getText()->renderDPI.y) / m_screenSize->Height;
	
	if (absoluteCompare(absoluteTextBoxBottom, getAbsoluteLocation().y + getAbsoluteSize().y / 2.0f) > 0)
	{
		//Move the text downwards by the appropriate scroll intensity. Make sure that the text doesn't go lower than the
		//top of the text box. We also need to change the absolute size of the text element to make sure the bottom of it stays
		//glued to the bottom of the text box
		textOverlay->setAbsoluteLocation({ currentTextAbsoluteLocation.x, currentTextAbsoluteLocation.y - m_scrollIntensity / (float)2.0 });
		textOverlay->setAbsoluteSize({ currentTextAbsoluteSize.x, currentTextAbsoluteSize.y + m_scrollIntensity });

		//The font is normally sized according to the height of the text overlay, but in this case we want
		//the text to be sized based on the height of the entire text box. Since the absolute size of the
		//text overlay just changed, the font size will need to change accordingly.
		textOverlay->setFontSize(textOverlay->getFontSize() * currentTextAbsoluteSize.y / textOverlay->getAbsoluteSize().y);

		textOverlay->resize(); //use the new absolute coordinates to update the pixels for the text

		//After moving the text box, we move the scroll progress bar
		calcualteScrollBarLocation();
	}
}

void PartialScrollingTextBox::calcualteScrollBarLocation()
{
	//We calculate the height of the scroll bar by using a simple ratio. Assuming that the center of the
	//text box correlates to the center of the progress scroll bar, and the bottom of the unclipped
	//text correlates to the bottom of the scroll bar background then location of the scroll progress bar
	//center will simply be:
	float totalTextHeight = p_children[0]->getChildren()[1]->getText()->renderDPI.y;
	float bottomTextToCenterBox = totalTextHeight + getCurrentTextStartingHeight() - m_location.y * m_screenSize->Height;
	auto scrollProgressBackgroundBarAbsoluteSize = p_children[4]->getAbsoluteSize();
	auto scrollProgressBarAbsoluteLocation = p_children[5]->getAbsoluteLocation();
	float shadowHeight = ((ShadowedBox*)p_children[2]->getChildren()[0].get())->getShadowWidth();

	float scrollBarAbsoluteHeightFromBottom = (scrollProgressBackgroundBarAbsoluteSize.y * m_screenSize->Height - shadowHeight) * (totalTextHeight + getCurrentTextStartingHeight() - m_location.y * m_screenSize->Height) / totalTextHeight;
	scrollBarAbsoluteHeightFromBottom /= m_screenSize->Height;
	scrollBarAbsoluteHeightFromBottom -= -shadowHeight / m_screenSize->Height;

	p_children[5]->setAbsoluteLocation({ scrollProgressBarAbsoluteLocation.x, (m_location.y + m_size.y / 2.0f) - m_buttonHeight - scrollBarAbsoluteHeightFromBottom });
	p_children[5]->resize();
}

void PartialScrollingTextBox::setChildrenAbsoluteSize(DirectX::XMFLOAT2 size)
{
	//Calculate the appropriate height and width of the buttons and
	//set these new sizes
	float screen_ratio = MAX_SCREEN_HEIGHT / MAX_SCREEN_WIDTH;
	m_buttonHeight = m_buttonRatio * size.y; //for now make both buttons 1/8th the height of the box
	float buttonWidth = screen_ratio * m_buttonHeight;

	p_children[3]->setAbsoluteSize({ buttonWidth, m_buttonHeight });
	p_children[4]->setAbsoluteSize({ buttonWidth, m_buttonHeight });

	//The width of the text box is set to the width of the whole element minus
	//the width of the buttons, the height is the same as the whole element.
	p_children[0]->setAbsoluteSize({ size.x - buttonWidth, size.y });

	//Unlike most UI Elements, the TextBox UI Element isn't responsible for setting
	//the location of its text (because the text is allowed to be positioned outside
	//of the box, as is the case here). Because of this we manually resize and reposition
	//the text accordingly.
	p_children[0]->getChildren()[1]->getText()->renderDPI.y = m_textTotalHeightRatio * size.y * m_screenSize->Height;
	p_children[0]->getChildren()[1]->setAbsoluteLocation({ p_children[0]->getChildren()[1]->getAbsoluteLocation().x, getAbsoluteLocation().y + size.y / 2.0f - p_children[0]->getChildren()[1]->getAbsoluteSize().y / 2.0f});

	//The cover box isn't complicated so it can be sized and placed
	//at the same time.
	p_children[1]->setAbsoluteSize({ size.x, getAbsoluteLocation().y - size.y / 2.0f});
	p_children[1]->setAbsoluteLocation({ getAbsoluteLocation().x, getAbsoluteLocation().y / 2.0f - size.y / 4.0f - 1.0f / MAX_SCREEN_HEIGHT });

	//The scroll bar background is simply the width of the buttons and the
	//height of the whole element. 
	p_children[2]->setAbsoluteSize({ buttonWidth, size.y });

	//The physical scroll bar is a little more complicated to calculate as
	//it's a function of the current text being displayed in the box. Calcualte
	//the appropriate height of the element, and set the width to be the same
	//as the button width.
	p_children[5]->setAbsoluteSize({ buttonWidth, size.y / 4.0f }); //get's fully updated later on

	//With everything sized appropriately, shift all children to their 
	//correct locations.
	auto absoluteLocation = getAbsoluteLocation();
	auto buttonXLocation = absoluteLocation.x + (size.x - buttonWidth) / 2.0f;

	p_children[0]->setAbsoluteLocation({ absoluteLocation.x - buttonWidth / 2.0f, absoluteLocation.y });
	p_children[2]->setAbsoluteLocation({ buttonXLocation, absoluteLocation.y });
	p_children[3]->setAbsoluteLocation({ buttonXLocation, absoluteLocation.y - (size.y - m_buttonHeight) / 2.0f });
	p_children[4]->setAbsoluteLocation({ buttonXLocation, absoluteLocation.y + (size.y - m_buttonHeight) / 2.0f });
	p_children[5]->setAbsoluteLocation({ buttonXLocation, absoluteLocation.y }); //get's fully updated later on

}

void PartialScrollingTextBox::repositionText()
{
	//A simple method for this class, all we do is take the height in pixels of the entire
	//text overlay of the text box, convert that height to absolute coordinates and then 
	//record the ratio of this value to the absolute height of the text box. The text overlay
	//height is obtained from the renderer class.
	m_textTotalHeightRatio = p_children[0]->getChildren()[1]->getText()->renderDPI.y / (m_screenSize->Height * getAbsoluteSize().y);
	m_state &= ~UIElementState::NeedTextPixels;
}

uint32_t PartialScrollingTextBox::update(InputState* inputState)
{
	//At the end of the standard update, we check to see if either of the buttons are currently being pressed.
	//If so, it has the effect of scrolling the text twice.
	uint32_t currentState = UIElement::update(inputState);

	if (p_children[2]->getState() & UIElementState::Released)
	{
		onScrollUp();
		onScrollUp();
	}
	else if (p_children[3]->getState() & UIElementState::Released)
	{
		onScrollDown();
		onScrollDown();
	}

	return currentState;
}

std::vector<UIText*> PartialScrollingTextBox::setTextDimension()
{
	//Return a reference to the text element inside of the text box
	return { p_children[0]->getChildren()[1]->getText() };
}

float PartialScrollingTextBox::getCurrentTextStartingHeight()
{
	//gives us the y coordinate for the starting point of the text in pixels
	return p_children[0]->getChildren()[1]->getText()->startLocation.y;
}