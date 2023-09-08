#include "pch.h"
#include "DropDownMenu.h"

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

	scrollBox.setState(UIElementState::Invisible); //the scroll box is invisible to start off. Clicking the button makes it appear

	p_children.push_back(std::make_shared<TextBox>(textBox));
	p_children.push_back(std::make_shared<ArrowButton>(button));
	p_children.push_back(std::make_shared<FullScrollingTextBox>(scrollBox));

	//Set the screen size dependent information for the TextBox
	m_size = size;
	m_location = location;
	m_needTextRenderDimensions = true;
	m_optionsDisplayed = optionsDisplayed;
	m_inverted = isInverted;

	m_state = UIElementState::NeedTextPixels; //Let's the renderer know that we currently need the pixel size of text
}

std::vector<UIText*> DropDownMenu::setTextDimension()
{
	//We really only care about the text dimensions withing the full scrolling box child element.
	//Since this child also implements the setTextDimension() method we just call that method. The width
	//of the drop down text box will be sized off of the widest option in the scroll box.
	return ((FullScrollingTextBox*)p_children[2].get())->setTextDimension();
}

void DropDownMenu::repositionText()
{
	//When this method gets called we know the dimensions of all the text inside of the full scrolling
	//text box child element. Resize the scroll box so that the appropriate number of options designated
	//by the m_optionsDisplayed field will perfectly fit in the box. Then resize the drop down text box
	//accordingly and place the scroll box directly on top of or below the text box (depending on the value
	//of the m_inverted field).
	float textHeight = p_children[2]->getChildren()[5]->getText()->renderDPI.y;
	p_children[2]->setAbsoluteSize({ 0, textHeight * m_optionsDisplayed }); //the x-dimension will get resized by the scroll box class
	p_children[2]->repositionText(); //resize the scroll box

	//TODO: resize/move the button and move the scroll box accordingly
	m_size = { p_children[2]->getAbsoluteSize().x, textHeight };
	p_children[2]->setAbsoluteLocation({ m_location.x, m_location.y - (m_size.y + p_children[2]->getAbsoluteSize().y) / 2.0f }); //TODO: need to take inversion into account
}