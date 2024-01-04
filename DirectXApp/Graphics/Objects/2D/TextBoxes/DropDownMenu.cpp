#include "pch.h"
#include "DropDownMenu.h"

/*DropDownMenu Children Order
   0: Text Box that displays the currently selected option
   1: Arrow Button that brings up the selectable options
   2: Scrolling Text box that holds the selectable options
*/

DropDownMenu::DropDownMenu(winrt::Windows::Foundation::Size windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring message,
	float fontSize, int optionsDisplayed, bool isInverted, std::vector<UIColor> textColor, std::vector<unsigned long long> textColorLocations, UITextJustification justification, UIColor textFillColor, bool isSquare, UIColor outlineColor, UIColor shadowColor)
{
	//The Drop down menu uses a full scrolling text box whose width is determined by the length of
	//the longest option contained in it. The width of the text box for the drop down menu will
	//match this width, which means, we can't know the appropriate width of the text box until 
	//after making a call to the renderer to get the pixel sizes for the text layouts. For now
	//we just create the child elements. They will all get resized and positioned later on.
	FullScrollingTextBox scrollBox(windowSize, location, size, message, fontSize);
	TextBox textBox(windowSize, location, size, scrollBox.getChildren()[5]->getText()->message, fontSize); //use the first option in the scroll box to populate the text box
	ArrowButton button(windowSize, location, size, isInverted, true);

	scrollBox.setState(scrollBox.getState() | UIElementState::Invisible); //the scroll box is invisible to start off. Clicking the button makes it appear

	p_children.push_back(std::make_shared<TextBox>(textBox));
	p_children.push_back(std::make_shared<ArrowButton>(button));
	p_children.push_back(std::make_shared<FullScrollingTextBox>(scrollBox));

	//Set the screen size dependent information for the TextBox
	m_size = size;
	m_location = location;
	m_needTextRenderDimensions = true;
	m_optionsDisplayed = optionsDisplayed;
	m_inverted = isInverted;
	m_currentlySelectedOption = scrollBox.getChildren()[5]->getText()->message; //default selection is the first option

	m_state = UIElementState::NeedTextPixels; //Let's the renderer know that we currently need the pixel size of text
	m_state &= ~UIElementState::Dummy; //The default constructor wasn't used so this isn't a dummy element
}

std::vector<UIText*> DropDownMenu::setTextDimension()
{
	//We really only care about the text dimensions withing the full scrolling box child element.
	//Since this child also implements the setTextDimension() method we just call that method. The width
	//of the drop down text box will be sized off of the widest option in the scroll box.
	return ((FullScrollingTextBox*)p_children[2].get())->setTextDimension();
}

void DropDownMenu::setSelectedOption(std::wstring option)
{
	//This method gives a way to set the selected option of the drop down box without
	//requiriug a click.
	m_currentlySelectedOption = option;
	getChildren()[0]->getChildren()[1]->getText()->message = option;
	getChildren()[0]->getChildren()[1]->getText()->colorLocations.back() = getChildren()[0]->getChildren()[1]->getText()->message.length();
	((FullScrollingTextBox*)getChildren()[2].get())->setLastSelectedText(option);
}

void DropDownMenu::clearAllOptions()
{
	//Removes all of the options currently stored in the drop down menu
	((FullScrollingTextBox*)getChildren()[2].get())->clearText();
	((FullScrollingTextBox*)getChildren()[2].get())->setLastSelectedText(L""); //since there are no options set an empty string as the currently selected option
}

void DropDownMenu::setNewOptions(std::wstring options, winrt::Windows::Foundation::Size windowSize, bool highlightable)
{
	//Overwrite the options currently in the drop down menu with the given options.
	//The options string here should be a string delimited with \n characters between
	//each option to be displayed.
	clearAllOptions();
	((FullScrollingTextBox*)getChildren()[2].get())->addText(options, windowSize, highlightable);

	//Setting new options will force the Drop down menu to resize to fit the longest
	//of the new options
	m_state |= UIElementState::NeedTextPixels;
}

void DropDownMenu::repositionText()
{
	//When this method gets called we know the dimensions of all the text inside of the full scrolling
	//text box child element. Resize the scroll box so that the appropriate number of options designated
	//by the m_optionsDisplayed field will perfectly fit in the box. Then resize the drop down text box
	//accordingly and place the scroll box directly on top of or below the text box (depending on the value
	//of the m_inverted field).
	auto currentWindowSize = getCurrentWindowSize();
	float absoluteTextHeight = p_children[2]->getChildren()[5]->getText()->renderDPI.y / currentWindowSize.Height;
	//p_children[2]->setAbsoluteSize({ p_children[2]->getAbsoluteSize().x, absoluteTextHeight * m_optionsDisplayed }); //the x-dimension will get resized by the scroll box class
	//p_children[2]->resize(currentWindowSize);
	p_children[2]->repositionText(); //resize the scroll box

	int invert = 1;
	if (m_inverted) invert *= -1;

	m_size = { p_children[2]->getAbsoluteSize().x, absoluteTextHeight }; //don't use setAbsoluteSize() as it will resize the scroll box
	p_children[0]->setAbsoluteSize(m_size); //change the text box size to reflect m_size
	p_children[2]->setAbsoluteLocation({ m_location.x, m_location.y - invert * (m_size.y + p_children[2]->getAbsoluteSize().y) / 2.0f });

	//resize the button to match the height of the drop down text box and move
	//it to the right side of the text box (making sure to compensate for square
	//element drift)
	p_children[1]->setAbsoluteSize({ absoluteTextHeight, absoluteTextHeight });
	float driftCorrectedButtonLocation = m_location.x + (m_size.x + absoluteTextHeight) / 2.0f - ((ShadowedBox*)p_children[1]->getChildren()[0].get())->fixSquareBoxDrift(currentWindowSize);
	p_children[1]->setAbsoluteLocation({driftCorrectedButtonLocation, m_location.y});
}

uint32_t DropDownMenu::update(InputState* inputState)
{
	//At the end of the standard update, we check to see if the arrow button has been
	//clicked. If so we toggle the visibility of the scroll box. If the scroll box
	//is visible and one of its options is selected, we update the text in the text
	//box and make the scroll box invisible.
	uint32_t currentState = UIElement::update(inputState);

	/*if ((p_children[1]->getState() & UIElementState::Clicked) && inputState->mouseClick)
	{
		p_children[2]->setState(p_children[2]->getState() ^ UIElementState::Invisible);
	}*/
	if (p_children[1]->getState() & UIElementState::Released)
	{
		//Clicking the arrow button toggles the visibility of scrolling text box containing the selectable options
		p_children[2]->setState(p_children[2]->getState() ^ UIElementState::Invisible);
	}
	else if ((inputState->mouseClickState == MouseClickState::MouseReleased) && !(p_children[2]->getState() & UIElementState::Invisible) &&
		(p_children[2]->getState() & UIElementState::Hovered))
	{
		p_children[0]->getChildren()[1]->getText()->message = ((FullScrollingTextBox*)p_children[2].get())->getLastSelectedText();
		p_children[0]->getChildren()[1]->getText()->colorLocations.back() = p_children[0]->getChildren()[1]->getText()->message.length();
		p_children[2]->setState(p_children[2]->getState() ^ UIElementState::Invisible);

		m_currentlySelectedOption = p_children[0]->getChildren()[1]->getText()->message; //update the currentlySelectedOption variable
	}

	return currentState;
}