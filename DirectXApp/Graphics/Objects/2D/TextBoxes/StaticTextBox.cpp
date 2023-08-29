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
	UIShape background({ 0, 0, 0, 0 }, UIShapeColor::White, UIShapeFillType::Fill, UIShapeType::RECTANGLE);
	m_backgroundShapes.push_back(background);
	addText(text);
	resize(windowSize); //sets the appropriate sizes for both the rectangle and text
}

void StaticTextBox::addText(std::wstring text)
{
	//The text passed in get's converted into a Text class object and is added 
	//to the end of the elementText vector. Added text will just be simple, black text.
	//The start location of the text rendering box and hte font size get filled out in
	//the resize method.
	UIText newText(text, 0, { 0, 0 }, { UITextColor::Black }, { 0, text.length() }, UITextType::ELEMENT_TEXT);
	m_elementText.push_back(newText);
}

void StaticTextBox::resize(winrt::Windows::Foundation::Size windowSize)
{
	//The Static text box doesn't have any children to worry about, we just need to resize the main rectangle
	//which is located in the background objects array and the text that goes on top of it.
	DirectX::XMFLOAT2 center_point = { windowSize.Width * m_location.x, windowSize.Height * m_location.y };
	const D2D1_RECT_F rect = D2D1::RectF(
		center_point.x - windowSize.Width * m_size.x / (float)2.0,
		center_point.y - windowSize.Height * m_size.x / (float)2.0,
		center_point.x + windowSize.Width * m_size.x / (float)2.0,
		center_point.y + windowSize.Height * m_size.x / (float)2.0
	);
	m_backgroundShapes[0].m_rectangle = rect;

	m_elementText[0].startLocation = { rect.left, rect.top }; //set the start location of the rendering box to be at the top left of the text box
	m_elementText[0].fontSize = windowSize.Height * m_fontSize;
}