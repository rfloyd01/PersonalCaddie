#include "pch.h"
#include "PartialScrollingTextBox.h"
#include "Graphics/Objects/2D/Buttons/ArrowButton.h"

PartialScrollingTextBox::PartialScrollingTextBox(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, UIColor backgroundColor, std::wstring message,
	float fontSize, std::vector<UIColor> textColor, std::vector<unsigned long long> textColorLocations, UITextJustification justification, UIColor textFillColor, bool isSquare, UIColor outlineColor, UIColor shadowColor)
{

	//Before creating the text box, check to see if the text color array was default initialized. If it was then 
	//we add the locations for the start of the text (index 0) and the end of the text (message.length())
	if (textColorLocations.size() == 0)
	{
		textColorLocations.push_back(0);
		textColorLocations.push_back(message.length());
	}

	TextBoxBasic textBox(windowSize, location, size, message, fontSize, textColor, textColorLocations, justification, textFillColor, isSquare, outlineColor, shadowColor);

	//The text covering box is the same width as the text box, and extends from the top of the text box
	//all the way to the top of the window.
	Box textCoveringBox(windowSize, { location.x, location.y / (float)2.0 - size.y / (float)4.0 }, { location.x, location.y - size.y / (float)2.0 }, backgroundColor, UIShapeFillType::Fill, isSquare);

	//A scrolling text box also features two arrow buttons, one up and one down.
	//Both of these buttons are squares
	m_buttonSize = 0.1 * size.y; //for now make both buttons 1/10th the height of the box
	ArrowButton upButton(windowSize, { location.x + (size.x + m_buttonSize) / (float)2.0, location.y - (size.y - m_buttonSize) / (float)2.0 }, { m_buttonSize, m_buttonSize }, false, true);
	ArrowButton downButton(windowSize, { location.x + (size.x + m_buttonSize) / (float)2.0, location.y + (size.y - m_buttonSize) / (float)2.0 }, { m_buttonSize, m_buttonSize }, true, true);

	//The order of the child elements is important here. The text background must be first, then the text,
	//and then finally the hiding box to go on top of it.
	p_children.push_back(std::make_shared<TextBoxBasic>(textBox));
	p_children.push_back(std::make_shared<Box>(textCoveringBox));
	p_children.push_back(std::make_shared<ArrowButton>(upButton));
	p_children.push_back(std::make_shared<ArrowButton>(downButton));

	m_isScrollable = true; //this enables scrolling detection in the main rendering loop
	m_needTextRenderDimensions = true; //alerts the current mode that this element will need text pixels from the renderer at some point
	m_scrollIntensity = 0.005; //set the scroll intensity

	m_state = UIElementStateBasic::NeedTextPixels; //Let's the render know that we currently need the pixel size of text

	//set screen size dependent variables
	m_location = location;
	m_size = size;
}

void PartialScrollingTextBox::onScrollUp()
{
	//When the mouse wheel is scrolled up, it has the effect of moving the text downwards. If
	//the text is already at the top of the scroll box though then scrolling up doesn't do anything.
	//Scrolling is also disabled if all the text already fits inside the text box.
	auto currentWindowSize = getCurrentWindowSize();

	auto a = getCurrentTextStartingHeight();
	if (getCurrentTextStartingHeight() < currentWindowSize.Height * (p_children[0]->getAbsoluteLocation().y - (p_children[0]->getAbsoluteSize().y / (float)2.0)))
	{
		//Move the text downwards by the appropriate scroll intensity. Make sure that the text doesn't go lower than the
		//top of the text box.
		auto currentTextAbsoluteLocation = p_children[0]->getChildren()[1]->getAbsoluteLocation();
		auto currentTextAbsoluteSize = p_children[0]->getChildren()[1]->getAbsoluteSize();
		p_children[0]->getChildren()[1]->setAbsoluteLocation({ currentTextAbsoluteLocation.x, currentTextAbsoluteLocation.y + m_scrollIntensity / (float)2.0 });
		p_children[0]->getChildren()[1]->setAbsoluteSize({ currentTextAbsoluteSize.x, currentTextAbsoluteSize.y - m_scrollIntensity });
		p_children[0]->getChildren()[1]->resize(currentWindowSize); //use the new absolute coordinates to update the pixels for the text
	}
}

void PartialScrollingTextBox::onScrollDown()
{
	//When the mouse wheel is scrolled down, it has the effect of moving the text upwards. If
	//the bottom of the text is already inside the scroll box though then scrolling down doesn't do anything.
	//Scrolling is also disabled if all the text already fits inside the text box.
	auto currentWindowSize = getCurrentWindowSize();

	float totalTextHeight = p_children[0]->getChildren()[1]->getText()->renderDPI.y; //the total height of the text layout, including text that's clipped by the bottom of the box
	auto b = currentWindowSize.Height * (p_children[0]->getAbsoluteLocation().y + (p_children[0]->getAbsoluteSize().y / (float)2.0));
	if (getCurrentTextStartingHeight() + totalTextHeight > currentWindowSize.Height * (p_children[0]->getAbsoluteLocation().y + (p_children[0]->getAbsoluteSize().y / (float)2.0)))
	{
		//Move the text downwards by the appropriate scroll intensity. Make sure that the text doesn't go lower than the
		//top of the text box. We also need to change the absolute size of the text element to make sure the bottom of it stays
		//glued to the bottom of the text box
		auto currentTextAbsoluteLocation = p_children[0]->getChildren()[1]->getAbsoluteLocation();
		auto currentTextAbsoluteSize = p_children[0]->getChildren()[1]->getAbsoluteSize();
		p_children[0]->getChildren()[1]->setAbsoluteLocation({ currentTextAbsoluteLocation.x, currentTextAbsoluteLocation.y - m_scrollIntensity / (float)2.0 });
		p_children[0]->getChildren()[1]->setAbsoluteSize({ currentTextAbsoluteSize.x, currentTextAbsoluteSize.y + m_scrollIntensity });
		p_children[0]->getChildren()[1]->resize(currentWindowSize); //use the new absolute coordinates to update the pixels for the text
	}
}

void PartialScrollingTextBox::repositionText()
{
	//It's possible that at one point the screen was small enough that not all text could fit in the text box,
	//but we've since made the screen larger and now all the text can fit, but it's no longer all in the box.
	//Perform a quick check to make sure that the text is all in the correct place.
	auto currentWindowSize = getCurrentWindowSize();
	float totalTextHeight = p_children[0]->getChildren()[1]->getText()->renderDPI.y; //the total height of the text layout, including text that's clipped by the bottom of the box

	//Check to see if the bottom of the text is higher than the bottom of the text box
	float textBoxBottomPixelLocation = currentWindowSize.Height * (m_location.y + m_size.y / (float)2.0);
	if (getCurrentTextStartingHeight() + totalTextHeight < textBoxBottomPixelLocation)
	{
		//Check to see if the top of the text is higher than the top of the text box
		float textBoxTopPixelLocation = currentWindowSize.Height * (m_location.y - m_size.y / (float)2.0);
		if (getCurrentTextStartingHeight() < textBoxTopPixelLocation)
		{
			//We need to scootch the text downwards until the bottom of the text is as low as it can go.
			//Whichever distance is less, either the bottom of the render rectangle to the bottom of the text
			//box, or the top of the render rectangle to the top of the text box, move the render rectangle
			//downwards by that amount.
			float lesserDistance = (textBoxTopPixelLocation - getCurrentTextStartingHeight()) < (textBoxBottomPixelLocation - (getCurrentTextStartingHeight() + totalTextHeight)) ?
				textBoxTopPixelLocation - getCurrentTextStartingHeight() : textBoxBottomPixelLocation - (getCurrentTextStartingHeight() + totalTextHeight);

			//Update the absolute location for the text
			auto currentTextAbsoluteLocation = p_children[0]->getChildren()[1]->getAbsoluteLocation();
			auto currentTextAbsoluteSize = p_children[0]->getChildren()[1]->getAbsoluteSize();
			p_children[0]->getChildren()[1]->setAbsoluteLocation({ currentTextAbsoluteLocation.x, currentTextAbsoluteLocation.y + lesserDistance / ((float) 2.0 * currentWindowSize.Height) });
			p_children[0]->getChildren()[1]->setAbsoluteSize({ currentTextAbsoluteSize.x, currentTextAbsoluteSize.y - lesserDistance / currentWindowSize.Height });
		}
	}

	//Although it isn't related to the text in the scroll box at all, this method is a good place to reposition 
	//the arrow buttons on the side of the scroll box. Since the buttons are square then their x-location will
	//drift slightly as the screen gets resized.
	auto upButtonLocation = p_children[2]->getAbsoluteLocation();
	auto downButtonLocation = p_children[3]->getAbsoluteLocation();

	float buttonWidthDifferential = ((ShadowedBox*)p_children[2]->getChildren()[0].get())->fixSquareBoxDrift(currentWindowSize);
	float shadowPixels = ((ShadowedBox*)p_children[2]->getChildren()[0].get())->getShadowWidth() / currentWindowSize.Width; //get the relative width of the shadow box shadow

	float buttonRelativeXLocation = m_location.x + (m_size.x + m_buttonSize) / 2.0f + shadowPixels - buttonWidthDifferential;

	p_children[2]->setAbsoluteLocation({ buttonRelativeXLocation, upButtonLocation.y });
	p_children[3]->setAbsoluteLocation({ buttonRelativeXLocation, downButtonLocation.y });
}

UIText* PartialScrollingTextBox::setTextDimension()
{
	//Return a reference to the text element inside of the text box
	return p_children[0]->getChildren()[1]->getText();
}

float PartialScrollingTextBox::getCurrentTextStartingHeight()
{
	//gives us the y coordinate for the starting point of the text in pixels
	return p_children[0]->getChildren()[1]->getText()->startLocation.y;
}