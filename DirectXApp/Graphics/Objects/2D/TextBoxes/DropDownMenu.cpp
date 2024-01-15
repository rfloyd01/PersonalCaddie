#include "pch.h"
#include "DropDownMenu.h"

/*DropDownMenu Children Order
   0: Text Box that displays the currently selected option
   1: Arrow Button that brings up the selectable options
   2: Scrolling Text box that holds the selectable options
*/

DropDownMenu::DropDownMenu(std::shared_ptr<winrt::Windows::Foundation::Size> windowSize, DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring message,
	float fontSize, int optionsDisplayed, bool isInverted, std::vector<UIColor> textColor, std::vector<unsigned long long> textColorLocations, UITextJustification justification, UIColor textFillColor, bool isSquare, UIColor outlineColor, UIColor shadowColor)
{
	m_screenSize = windowSize;
	updateLocationAndSize(location, size);

	//The Drop down menu uses a full scrolling text box whose width is determined by the length of
	//the longest option contained in it. The width of the text box for the drop down menu will
	//match this width, which means, we can't know the appropriate width of the text box until 
	//after making a call to the renderer to get the pixel sizes for the text layouts. For now
	//we just create the child elements. They will all get resized and positioned later on.
	FullScrollingTextBox scrollBox(windowSize, location, size, message, fontSize);
	TextBox textBox(windowSize, location, size, scrollBox.getChildren()[5]->getText()->message, fontSize); //use the first option in the scroll box to populate the text box
	ArrowButton button(windowSize, location, size, isInverted);

	scrollBox.setState(scrollBox.getState() | UIElementState::Invisible); //the scroll box is invisible to start off. Clicking the button makes it appear

	p_children.push_back(std::make_shared<TextBox>(textBox));
	p_children.push_back(std::make_shared<ArrowButton>(button));
	p_children.push_back(std::make_shared<FullScrollingTextBox>(scrollBox));

	//Set the screen size dependent information for the TextBox
	m_needTextRenderDimensions = true;
	m_optionsDisplayed = optionsDisplayed;
	m_inverted = isInverted;
	m_currentlySelectedOption = scrollBox.getChildren()[5]->getText()->message; //default selection is the first option

	m_state = UIElementState::NeedTextPixels; //Let's the renderer know that we currently need the pixel size of text
	m_state &= ~UIElementState::Dummy; //The default constructor wasn't used so this isn't a dummy element
}

std::vector<UIText*> DropDownMenu::setTextDimension()
{
	//We really only care about the text dimensions within the full scrolling box child element.
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

void DropDownMenu::setNewOptions(std::wstring options, bool highlightable)
{
	//Overwrite the options currently in the drop down menu with the given options.
	//The options string here should be a string delimited with \n characters between
	//each option to be displayed.
	clearAllOptions();
	((FullScrollingTextBox*)getChildren()[2].get())->addText(options, highlightable);

	//Setting new options will force the Drop down menu to resize to fit the longest
	//of the new options
	m_state |= UIElementState::NeedTextPixels;
}

void DropDownMenu::repositionText()
{
	//When this method gets called we know the dimensions of all the text inside of the full scrolling
	//text box child element. Resize the scroll box so that the appropriate number of options designated
	//by the m_optionsDisplayed field will perfectly fit in the box. Then resize the drop down box as a
	//whole to reflect the full scrolling textbox width and text height.
	float absoluteTextHeight = p_children[2]->getChildren()[5]->getText()->renderDPI.y / m_screenSize->Height;

	//Size the height of full scrolling text box to fit m_optionsDisplayed number of items with the given text height.
	//The width will get auto-set so just put it to 0 for now
	//p_children[2]->getChildren()[0]->setAbsoluteSize({0.0f, m_optionsDisplayed * absoluteTextHeight});
	p_children[2]->repositionText(); //this method will size the scroll box and position its text accordingly

	//The m_location variable of the drop down menu is slightly different than other UIElement classes.
	//Since the full scrolling text box part of the drop down menu will be invisible for most of the 
	//time it makes more sense to have the location be equal to the center of the option selecting box
	//as opposed to being the exact center of the element. For UIElements to work properly though, their
	//m_location variable needs to be at their center. To get around this, change the m_location variable
	//so that when the selection box is placed, its center will be equal the m_location passed into this
	//element's constructor. Use the updateLocationandSize() method to update the location to prevent 
	//updating the locations for child elements as well.
	auto originalLocation = getAbsoluteLocation();
	setAbsoluteLocation({ originalLocation.x, originalLocation.y - (m_optionsDisplayed * absoluteTextHeight) / 2.0f });

	//We're now ready to set the size of the DropDownMenu. The width is simply the width of the full scrolling
	//box calculated above, and the height will be the height of the full scrolling text box plus the height of 
	//a line of text.
	setAbsoluteSize({ p_children[2]->getAbsoluteSize().x, absoluteTextHeight + p_children[2]->getAbsoluteSize().y }, true); //bringing in new text will force a resize
	m_state &= ~UIElementState::NeedTextPixels; //remove the NeedTextPixels state when repositioning is complete
}

void DropDownMenu::setChildrenAbsoluteSize(DirectX::XMFLOAT2 size)
{
	auto optionBoxHeight = getAbsoluteSize().y / ((float)m_optionsDisplayed + 1.0f);
	if (!(m_state & UIElementState::NeedTextPixels))
	{
		//The full scrolling text box needs to be sized in this scenario
		p_children[2]->setAbsoluteSize({ getAbsoluteSize().x, optionBoxHeight * (float)m_optionsDisplayed });
	}

	//Set the size for the current option box and the arrow button
	float buttonWidth = MAX_SCREEN_HEIGHT / MAX_SCREEN_WIDTH * optionBoxHeight;
	p_children[0]->setAbsoluteSize({ getAbsoluteSize().x, optionBoxHeight });
	p_children[1]->setAbsoluteSize({ buttonWidth, optionBoxHeight });

	//The full scrolling text box appears above the drop down menu by default,
	//but it can be inverted.
	int invert = 1;
	if (m_inverted) invert *= -1;

	//Then set the location for each child element
	auto currentLocation = getAbsoluteLocation();
	p_children[0]->setAbsoluteLocation({ currentLocation.x, currentLocation.y + optionBoxHeight * (float)m_optionsDisplayed / 2.0f});
	p_children[1]->setAbsoluteLocation({ currentLocation.x + (getAbsoluteSize().x - buttonWidth) / 2.0f, currentLocation.y + optionBoxHeight * (float)m_optionsDisplayed / 2.0f});
	p_children[2]->setAbsoluteLocation({ currentLocation.x, currentLocation.y - (getAbsoluteSize().y / (2.0f * ((float)m_optionsDisplayed + 1.0f))) });

	//The last thing to do here is reposition the selected option text
	//inside of the option box. We need to do this since the option box
	//is a text box UIElement which isn't responsible for positioning
	//its own text.
	float absoluteFontSize = p_children[2]->getFontSize() * p_children[2]->getAbsoluteSize().y;
	p_children[0]->getChildren()[1]->setAbsoluteLocation(p_children[0]->getAbsoluteLocation());
	p_children[0]->getChildren()[1]->setAbsoluteSize(p_children[0]->getAbsoluteSize());
	p_children[0]->getChildren()[1]->setFontSize(absoluteFontSize / p_children[0]->getAbsoluteSize().y);
}

uint32_t DropDownMenu::update(InputState* inputState)
{
	//At the end of the standard update, we check to see if the arrow button has been
	//clicked. If so we toggle the visibility of the scroll box. If the scroll box
	//is visible and one of its options is selected, we update the text in the text
	//box and make the scroll box invisible.
	uint32_t currentState = UIElement::update(inputState);

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