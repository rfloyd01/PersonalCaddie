#include "pch.h"
#include "StaticTextBox.h"

StaticTextBox::StaticTextBox(DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring text, winrt::Windows::Foundation::Size windowSize)
{
	m_location = location;
	m_size = size;
	m_fontSize = 0.1 * size.y; //set the font height to be 1/10 the height of the text box

	//The StaticTextBox is the most basic of UI Elements. It's just a white rectangle that has
	//black text overlayed on top of it. If there's too much text then it gets clipped at the
	//bottom of the text box. The m_location variable points to the center of the rectangle.
	UIShape background({ 0, 0, 0, 0 }, UIColor::White, UIShapeFillType::Fill, UIShapeType::RECTANGLE);
	m_backgroundShapes.push_back(background);
	addText(text);
	resize(windowSize); //sets the appropriate sizes for both the rectangle and text

	m_state = UIElementState::Idle; //the static text box will always have an idle state
}

uint32_t StaticTextBox::addText(std::wstring text)
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
		//This is the first time we're adding text, add in all elements that aren't dependent on the size of the screen
		//and add the text to the text vector
		UIText newText(text, 0, { 0, 0 }, { 0, 0 }, { UIColor::Black }, { 0, text.length() }, UITextType::ELEMENT_TEXT);
		m_elementText.push_back(newText);
	}
	
	return 0; //the text add worked, no need to update anything
}

void StaticTextBox::resize(winrt::Windows::Foundation::Size windowSize)
{
	//The Static text box doesn't have any children to worry about, we just need to resize the main rectangle
	//which is located in the background objects array and the text that goes on top of it.
	DirectX::XMFLOAT2 center_point = { windowSize.Width * m_location.x, windowSize.Height * m_location.y };
	const D2D1_RECT_F rect = D2D1::RectF(
		center_point.x - windowSize.Width * m_size.x / (float)2.0,
		center_point.y - windowSize.Height * m_size.y / (float)2.0,
		center_point.x + windowSize.Width * m_size.x / (float)2.0,
		center_point.y + windowSize.Height * m_size.y / (float)2.0
	);
	m_backgroundShapes[0].m_rectangle = rect;

	m_elementText[0].startLocation = { rect.left, rect.top }; //set the start location of the rendering box to be at the top left of the text box
	m_elementText[0].renderArea = { rect.right - rect.left, rect.bottom - rect.top }; //the rendering area is the same as the rectangle area
	m_elementText[0].fontSize = windowSize.Height * m_fontSize;
}

//the StaticTextBox class has nothing to update but this pure virtual method must be implemented
UIElementState StaticTextBox::update(InputState* inputState)
{
	return m_state;
}