#include "pch.h"
#include "ScrollingTextBox.h"
#include "../UIButton.h"

ScrollingTextBox::ScrollingTextBox(DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring text, winrt::Windows::Foundation::Size windowSize)
{
	//This is a compound UIElement, featuring two child buttons. There's also a number of 
	//UI shapes, including a white rectangle (and a border for it) where text is rendered,
	//some element text (which has the capability to move), and a foreground UI shape which
	//is used to cover up any text that appears outside of the rendering area.

	//The location and size variables passed in are for the actual text box, the size and location
	//of all other parts of the scroll box are derived from that.
	m_location = location;
	m_size = size;
	m_fontSize = 0.1 * size.y; //set the font height to be 1/10 the height of the text box
	m_textStart = { location.x - size.x / (float)2.0, location.y - size.y / (float)2.0, }; //the text always starts in the top left of the text box, only scrolling will change this

	UIShape background({ 0, 0, 0, 0 }, UIColor::White, UIShapeFillType::Fill, UIShapeType::RECTANGLE); //The white background for the text
	UIShape outline({ 0, 0, 0, 0 }, UIColor::Black, UIShapeFillType::NoFill, UIShapeType::RECTANGLE); //Outline for the text box

	m_backgroundShapes.push_back(background);
	m_backgroundShapes.push_back(outline);

	//Add the up and down buttons. The top of the top button and the bottom of the bottom button align
	//with the top and bottom of the text box respectively. Both buttons are flush with the right
	//side of the text box.
	float button_width = 0.1 * size.y; //for now, make the button width equal to 1/10 of the height of the text box. The button is a square.
	float buttonX = location.x + size.x / (float)2.0 + button_width / (float)2.0; //the butt
	UIButton    top_button({ buttonX, location.y - size.y / (float)2.0 + button_width / (float)2.0 }, { button_width, button_width }, windowSize);
	UIButton bottom_button({ buttonX, location.y + size.y / (float)2.0 - button_width / (float)2.0 }, { button_width, button_width }, windowSize);

	top_button.setParent(this);
	bottom_button.setParent(this);

	p_children.push_back(std::make_shared<UIButton>(top_button));
	p_children.push_back(std::make_shared<UIButton>(bottom_button));

	//The last background shapes to make have to do with the scrolling rectangle.
	//TODO: add these shapes

	addText(text);
	resize(windowSize); //sets the appropriate sizes for both the rectangle and text

	m_state = UIElementState::Idle; //the static text box will always have an idle state
}

void ScrollingTextBox::addText(std::wstring text)
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
	m_backgroundShapes[0].m_rectangle = rect;
	m_backgroundShapes[1].m_rectangle = rect;

	//To acheive the illusion of text scrolling, the starting location of the text drifts upwards and
	//out of the text box (where it get's covered by foreground UI shapes. The bottom of the rendering
	//area for text remains glued to the bottom of the text box to make sure text is clipped properly.
	m_elementText[0].startLocation = { m_textStart.x * windowSize.Width, m_textStart.y * windowSize.Height }; //start location for text moves with the window, not the text box
	m_elementText[0].renderArea = { rect.right - m_elementText[0].startLocation.x, rect.bottom - m_elementText[0].startLocation.y }; //bottom of text rendering area is glued to the bottom of the text box
	m_elementText[0].fontSize = windowSize.Height * m_fontSize;

	//After resizing the shapes owned directly by the scroll box, we need to resize the button UI Elements as well.
	for (int i = 0; i < p_children.size(); i++) p_children[i]->resize(windowSize);
}

//the StaticTextBox class has nothing to update but this pure virtual method must be implemented
UIElementState ScrollingTextBox::update(DirectX::XMFLOAT2 mousePosition, bool mouseClick)
{
	return m_state;
}

bool ScrollingTextBox::checkHover(DirectX::XMFLOAT2 mousePosition)
{
	//For the scroll box element we allow scrolling as long as the mouse is anywhere inside of the box. That includes the text box,
	//the buttons, or the progress bar.
	if (m_backgroundShapes.size() == 0) return false;

	//For the mouse to be considered "in" the button it needs to be within the outline of it, not the shadow
	if ((mousePosition.x >= m_backgroundShapes[0].m_rectangle.left) &&
		(mousePosition.x <= p_children[0]->getPixels(RenderOrder::Background, 0).right) &&
		(mousePosition.y >= m_backgroundShapes[0].m_rectangle.top) &&
		(mousePosition.y <= m_backgroundShapes[0].m_rectangle.bottom))
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

}

void ScrollingTextBox::onScrollDown()
{

}